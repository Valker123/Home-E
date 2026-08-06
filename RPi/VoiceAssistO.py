import ollama
import subprocess
import tempfile
import os
import time
import signal
import json
import wave
import pyaudio
import serial
import requests
from vosk import Model, KaldiRecognizer
import sys
import numpy as np
from collections import deque
from datetime import datetime

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

MODEL_PATH = os.path.join(BASE_DIR, "piper", "en_US-lessac-medium.onnx")
CONFIG_PATH = os.path.join(BASE_DIR, "piper", "en_US-lessac-medium.onnx.json")
OUTPUT_WAV = os.path.join(BASE_DIR, "test.wav")
THINKING_SOUND = os.path.join(BASE_DIR, "process.wav")
VOSK_MODEL_PATH = os.path.join(BASE_DIR, "vosk-model-small-en-us-0.15")
BEEP_SOUND = os.path.join(BASE_DIR, "beep.wav")
WAKE_WORDS = ["hey assistant", "hello assistant", "okay assistant", "assistant"]

# --- Mic settings ---
MIC_DEVICE_INDEX = 1          # USB PnP Sound Device
MIC_NATIVE_RATE = 44100       # what the USB mic actually supports
TARGET_RATE = 16000           # what Vosk needs
CHUNK_AT_TARGET = 4000
CHUNK_AT_NATIVE = int(CHUNK_AT_TARGET * MIC_NATIVE_RATE / TARGET_RATE)
FRAMES_PER_BUFFER = int(8000 * MIC_NATIVE_RATE / TARGET_RATE)

# --- ESP32 serial settings ---
ESP_PORT = "/dev/ttyUSB0"
ESP_BAUD = 115200

DIRECTION_TO_COMMAND = {
    "forward": b'W',
    "left": b'A',
    "backward": b'S',
    "right": b'D',
    "stop": b'V',
}

# --- Tool definitions for Ollama function calling ---
TOOLS = [
    {
        "type": "function",
        "function": {
            "name": "move_robot",
            "description": (
                "Move the robot in a direction, or stop it. Use this whenever "
                "the user asks the robot to move, drive, turn, go, or stop."
            ),
            "parameters": {
                "type": "object",
                "properties": {
                    "direction": {
                        "type": "string",
                        "enum": ["forward", "backward", "left", "right", "stop"],
                        "description": "Direction to move the robot"
                    },
                    "duration": {
                        "type": "number",
                        "description": "How long to move, in seconds. Default 1 second if not specified."
                    }
                },
                "required": ["direction"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "set_mode",
            "description": (
                "Switch the robot between manual (user-controlled) mode and "
                "autonomous (self-driving, obstacle-avoiding) mode."
            ),
            "parameters": {
                "type": "object",
                "properties": {
                    "mode": {
                        "type": "string",
                        "enum": ["manual", "auto"],
                        "description": "manual = user controlled, auto = autonomous obstacle avoidance"
                    }
                },
                "required": ["mode"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_outdoor_weather",
            "description": (
                "Get the CURRENT OUTDOOR temperature and weather conditions "
                "outside, based on your current location. Use this for phrases "
                "like 'what's the temperature outside', 'what's the weather "
                "like', 'is it cold out', 'how hot is it outside'. If the user "
                "asks about both indoor and outdoor temperature in the same "
                "request, call this tool AND get_room_temperature separately — "
                "do not just describe what you would do, actually call both."
            ),
            "parameters": {
                "type": "object",
                "properties": {}
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_room_temperature",
            "description": (
                "Get the CURRENT INDOOR/ROOM temperature from the local "
                "temperature sensor attached to the robot. Use this for "
                "phrases like 'what's the temperature in here', 'how warm "
                "is this room', 'what's the temperature right now' (when "
                "clearly referring to the immediate surroundings, not "
                "outside weather). Do NOT use this for outdoor weather "
                "questions — use get_outdoor_weather for that instead."
            ),
            "parameters": {
                "type": "object",
                "properties": {}
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_current_date",
            "description": (
                "Get the current date. Use this for phrases like 'what date "
                "is it', 'what's today's date', or 'what day is it'."
            ),
            "parameters": {
                "type": "object",
                "properties": {}
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "get_current_time",
            "description": (
                "Get the current time. Use this for phrases like 'what time "
                "is it', 'what's the time', 'do you know the time'."
            ),
            "parameters": {
                "type": "object",
                "properties": {}
            }
        }
    }
                
]

vosk_model = None
recognizer = None
is_listening = False
wake_word_detected = False
esp_serial = None


def init_esp_connection():
    global esp_serial
    try:
        esp_serial = serial.Serial(ESP_PORT, ESP_BAUD, timeout=1)
        time.sleep(2)  # give ESP32 time to reset/initialize
        print(f"ESP32 connected on {ESP_PORT}")
    except Exception as e:
        print(f"Could not connect to ESP32 ({e}). Motor commands will be skipped.")
        esp_serial = None


def execute_move(direction, duration=1.0):
    if esp_serial is None:
        print(f"[No ESP connected] Would move: {direction} for {duration}s")
        return
    cmd = DIRECTION_TO_COMMAND.get(direction)
    if not cmd:
        print(f"Unknown direction: {direction}")
        return
    esp_serial.write(cmd)
    print(f"Sent to ESP32: {cmd}")
    if direction != "stop":
        try:
            time.sleep(min(float(duration), 10.0))  # cap to 10s for safety
        except (TypeError, ValueError):
            time.sleep(1.0)
        esp_serial.write(b'V')
        print("Sent stop (V) after duration")


def execute_set_mode(mode):
    if esp_serial is None:
        print(f"[No ESP connected] Would set mode: {mode}")
        return
    if mode == "manual":
        esp_serial.write(b'M')
        print("Sent mode: Manual (M)")
    elif mode == "auto":
        esp_serial.write(b'T')
        print("Sent mode: Auto (T)")


def get_current_location():
    """Uses the Pi's public IP to estimate current location (city-level accuracy)."""
    try:
        resp = requests.get("http://ip-api.com/json/", timeout=5)
        data = resp.json()
        if data.get("status") == "success":
            return {
                "lat": data["lat"],
                "lon": data["lon"],
                "city": data.get("city", "unknown"),
                "region": data.get("regionName", ""),
            }
    except Exception as e:
        print(f"Location lookup failed: {e}")
    return None


def get_outdoor_weather():
    location = get_current_location()
    if not location:
        return "I couldn't determine your current location to check the weather."

    try:
        resp = requests.get(
            "https://api.open-meteo.com/v1/forecast",
            params={
                "latitude": location["lat"],
                "longitude": location["lon"],
                "current": "temperature_2m,weather_code",
                "temperature_unit": "fahrenheit",
            },
            timeout=5,
        )
        data = resp.json()
        current = data.get("current", {})
        temp_f = current.get("temperature_2m")

        if temp_f is None:
            return "I couldn't get the current weather data."

        place = location["city"] or "your current location"
        return f"It's currently {temp_f:.0f} degrees fahrenheit outside in {place}."

    except Exception as e:
        print(f"Weather lookup failed: {e}")
        return "I couldn't reach the weather service right now."

def get_room_temperature():
    if esp_serial is None:
        return "I can't reach the ESP32 to check the room temperature right now."

    try:
        # Clear out any stale/leftover bytes sitting in the input buffer
        esp_serial.reset_input_buffer()

        esp_serial.write(b'R')

        # Give the ESP32 a moment to take the reading and respond
        line = esp_serial.readline().decode('utf-8', errors='ignore').strip()

        if not line:
            return "I didn't get a response from the temperature sensor."

        if line == "TEMP:ERROR":
            return "The temperature sensor gave a bad reading. Try again in a moment."

        # Expected format: "TEMP:72.14,HUM:45.30"
        if line.startswith("TEMP:") and ",HUM:" in line:
            temp_part, hum_part = line.split(",HUM:")
            temp_f = float(temp_part.replace("TEMP:", ""))
            hum = float(hum_part)
            return f"The room is currently {temp_f:.0f}°F with {hum:.0f}% humidity."

        return f"Got an unexpected reading from the sensor: {line}"

    except Exception as e:
        print(f"Room temperature read failed: {e}")
        return "I had trouble reading the room temperature sensor."

def resample_audio(data, orig_rate=MIC_NATIVE_RATE, target_rate=TARGET_RATE):
    audio = np.frombuffer(data, dtype=np.int16)
    if len(audio) == 0:
        return data
    duration = len(audio) / orig_rate
    target_len = max(1, int(duration * target_rate))
    resampled = np.interp(
        np.linspace(0, len(audio), target_len, endpoint=False),
        np.arange(len(audio)),
        audio
    ).astype(np.int16)
    return resampled.tobytes()

def get_current_time():
    now = datetime.now()
    return now.strftime("It's currently %-I:%M %p.")

def get_current_date():
    now = datetime.now()
    return now.strftime("Today is %A, %B %-d, %Y.")

def create_beep_sound():
    if os.path.exists(BEEP_SOUND):
        return

    print("Creating beep sound file...")
    try:
        import math
        import struct

        SAMPLE_RATE = 44100
        DURATION = 0.3
        FREQUENCY = 880

        with wave.open(BEEP_SOUND, 'w') as wav_file:
            wav_file.setnchannels(1)
            wav_file.setsampwidth(2)
            wav_file.setframerate(SAMPLE_RATE)

            for i in range(int(SAMPLE_RATE * DURATION)):
                fade_factor = 1.0
                if i < SAMPLE_RATE * 0.1:
                    fade_factor = i / (SAMPLE_RATE * 0.1)
                elif i > SAMPLE_RATE * (DURATION - 0.1):
                    fade_factor = (SAMPLE_RATE * DURATION - i) / (SAMPLE_RATE * 0.1)

                sample = fade_factor * 0.5 * math.sin(2 * math.pi * FREQUENCY * i / SAMPLE_RATE)
                sample_int = int(sample * 32767)
                wav_file.writeframes(struct.pack('<h', sample_int))

        print(f"Beep sound created at {BEEP_SOUND}")
    except Exception as e:
        print(f"Could not create beep sound: {e}")


def initialize_vosk():
    global vosk_model, recognizer

    if not os.path.exists(VOSK_MODEL_PATH):
        print(f"Error: Vosk model not found at {VOSK_MODEL_PATH}")
        print("Please download a Vosk model from https://alphacephei.com/vosk/models")
        print("Example: vosk-model-small-en-us-0.15")
        sys.exit(1)

    vosk_model = Model(VOSK_MODEL_PATH)
    recognizer = KaldiRecognizer(vosk_model, TARGET_RATE)
    print("Vosk model loaded successfully")
    print(f"Listening for wake words: {', '.join(WAKE_WORDS)}")


def start_thinking_sound():
    proc = subprocess.Popen(
        ["aplay", "-q", THINKING_SOUND],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    return proc

# blocking/non-blocking code search this up :)

def stop_thinking_sound(proc):
    if proc and proc.poll() is None:
        proc.terminate()
        try:
            proc.wait(timeout=0.3)
        except:
            proc.kill()


def play_beep():
    try:
        if os.path.exists(BEEP_SOUND):
            subprocess.run(
                ["aplay", "-q", BEEP_SOUND],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
        else:
            subprocess.run(
                ["play", "-q", "-n", "synth", "0.3", "sine", "880"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
    except FileNotFoundError:
        try:
            subprocess.run(
                ["speaker-test", "-t", "sine", "-f", "880", "-l", "1"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=0.3
            )
        except:
            print("\a", end='', flush=True)
    except Exception as e:
        print(f"Could not play beep: {e}")
        print("\a", end='', flush=True)


def speak_with_piper(text):
    thinking_proc = None

    try:
        with tempfile.NamedTemporaryFile("w", delete=False, encoding="utf-8") as tmp:
            tmp.write(text)
            tmp_path = tmp.name

        thinking_proc = start_thinking_sound()

        piper_process = subprocess.Popen([
            "piper",
            "-m", MODEL_PATH,
            "-c", CONFIG_PATH,
            "-i", tmp_path,
            "-f", OUTPUT_WAV
        ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        while piper_process.poll() is None:
            if thinking_proc.poll() is not None:
                thinking_proc = start_thinking_sound()
            time.sleep(0.1)

        piper_process.wait()

    finally:
        try:
            os.unlink(tmp_path)
        except:
            pass

        if thinking_proc:
            stop_thinking_sound(thinking_proc)

    time.sleep(0.2)

    subprocess.run(["aplay", OUTPUT_WAV])


def check_wake_word(text):
    text_lower = text.lower().strip()
    for wake_word in WAKE_WORDS:
        if wake_word in text_lower:
            return True
    return False


def listen_for_command(timeout_seconds=10):
    global is_listening

    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=MIC_NATIVE_RATE,
                    input=True,
                    input_device_index=MIC_DEVICE_INDEX,
                    frames_per_buffer=FRAMES_PER_BUFFER)
    stream.start_stream()

    print("\nListening for command...")

    start_time = time.time()
    silence_start = None
    speech_detected = False
    final_text = ""

    recognizer.Reset()

    while is_listening:
        if time.time() - start_time > timeout_seconds:
            print("Timeout - no command detected")
            break

        raw_data = stream.read(CHUNK_AT_NATIVE, exception_on_overflow=False)
        data = resample_audio(raw_data)

        if recognizer.AcceptWaveform(data):
            result = json.loads(recognizer.Result())
            if result.get("text", "").strip():
                final_text = result["text"]
                speech_detected = True
                print(f"Command: {final_text}")
                break
        else:
            partial_result = json.loads(recognizer.PartialResult())
            partial_text = partial_result.get("partial", "")
            if partial_text:
                silence_start = None
                if not speech_detected:
                    speech_detected = True
                    print(f"Command: {partial_text}", end='\r')
            elif speech_detected and silence_start is None:
                silence_start = time.time()
            elif silence_start and time.time() - silence_start > 1.5:
                result = json.loads(recognizer.FinalResult())
                final_text = result.get("text", "")
                if final_text:
                    print(f"Command: {final_text}")
                break

    stream.stop_stream()
    stream.close()
    p.terminate()

    return final_text.strip()


def continuous_listen_for_wake_word():
    global is_listening, wake_word_detected

    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=MIC_NATIVE_RATE,
                    input=True,
                    input_device_index=MIC_DEVICE_INDEX,
                    frames_per_buffer=FRAMES_PER_BUFFER)
    stream.start_stream()

    print("\nAlways listening for wake word...")
    print(f"Say one of: {', '.join(WAKE_WORDS)}")

    recognizer.Reset()

    while is_listening:
        try:
            raw_data = stream.read(CHUNK_AT_NATIVE, exception_on_overflow=False)
            data = resample_audio(raw_data)

            if recognizer.AcceptWaveform(data):
                result = json.loads(recognizer.Result())
                text = result.get("text", "").strip()

                if text and check_wake_word(text):
                    print(f"\nWake word detected: '{text}'")
                    play_beep()
                    wake_word_detected = True
                    recognizer.Reset()
                    break

            partial_result = json.loads(recognizer.PartialResult())
            partial_text = partial_result.get("partial", "").lower()

            if partial_text and any(wake_word in partial_text for wake_word in WAKE_WORDS):
                time.sleep(0.1)
                result = json.loads(recognizer.FinalResult())
                final_text = result.get("text", "").strip()

                if final_text and check_wake_word(final_text):
                    print(f"\nWake word detected: '{final_text}'")
                    play_beep()
                    wake_word_detected = True
                    recognizer.Reset()
                    break

        except Exception as e:
            print(f"Audio error: {e}")
            time.sleep(0.1)

    stream.stop_stream()
    stream.close()
    p.terminate()


def process_with_llm(user_input):
    thinking_proc = start_thinking_sound()

    response_text = ""
    try:
        response = ollama.chat(
            model="qwen2.5:1.5b",
            keep_alive=-1,
            messages=[{
                "role": "user",
                "content": user_input
            }],
            tools=TOOLS,
        )

        message = response.get("message", {})
        tool_calls = message.get("tool_calls")

        if tool_calls:
            for call in tool_calls:
                fn_name = call["function"]["name"]
                args = call["function"].get("arguments", {})
                print(f"Tool call: {fn_name}({args})")

                if fn_name == "move_robot":
                    direction = args.get("direction", "stop")
                    duration = args.get("duration", 1.0)
                    execute_move(direction, duration)
                    if direction == "stop":
                        response_text = "Stopping."
                    else:
                        response_text = f"Moving {direction}."
                elif fn_name == "set_mode":
                    mode = args.get("mode", "manual")
                    execute_set_mode(mode)
                    response_text = f"Switched to {mode} mode."
                elif fn_name == "get_outdoor_weather":
                    response_text = get_outdoor_weather()
                elif fn_name == "get_room_temperature":
                    response_text = get_room_temperature()
                elif fn_name == "get_current_time":
                                        response_text = get_current_time()
                elif fn_name == "get_current_date":
                                        response_text = get_current_date()
                else:
                    response_text = "I tried to do something but wasn't sure what."
        else:
            response_text = (message.get("content") or "").strip()
            if not response_text:
                response_text = "I didn't have a response for that."

    except Exception as e:
        print(f"Error with LLM: {e}")
        response_text = "I encountered an error processing your request."

    finally:
        stop_thinking_sound(thinking_proc)

    return response_text.strip()


def main():
    global is_listening, wake_word_detected

    print("=" * 50)
    print("Voice Assistant with Wake Word + Robot Control")
    print("=" * 50)
    print(f"\nWake words: {', '.join(WAKE_WORDS)}")
    print("\nThe assistant is always listening...")
    print("Say a wake word followed by your command")
    print("Examples: 'hey assistant move forward'")
    print("          'hey assistant turn left for 3 seconds'")
    print("          'hey assistant switch to autonomous mode'")
    print("          'hello assistant what time is it'")
    print("\nPress Ctrl+C to exit")
    print("-" * 50)

    create_beep_sound()
    initialize_vosk()
    init_esp_connection()

    is_listening = True

    try:
        while is_listening:
            wake_word_detected = False
            continuous_listen_for_wake_word()

            if not is_listening:
                break

            if wake_word_detected:
                time.sleep(0.3)

                command = listen_for_command(timeout_seconds=10)

                if not command:
                    print("No command detected. Going back to sleep.\n")
                    continue

                if command.lower() in ["exit", "quit", "stop listening", "goodbye"]:
                    print("\nGoodbye!")
                    is_listening = False
                    break

                print(f"\nProcessing: {command}")

                response = process_with_llm(command)

                if response:
                    print(f"\nResponse: {response}")

                    time.sleep(0.1)
                    speak_with_piper(response)

                print("\n" + "-" * 50)
                print("Back to listening for wake word...\n")

            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\nInterrupted by user. Goodbye!")
    except Exception as e:
        print(f"\nError: {e}")
    finally:
        is_listening = False
        if esp_serial:
            try:
                esp_serial.write(b'V')  # safety stop on exit
                esp_serial.close()
            except:
                pass


if __name__ == "__main__":
    def signal_handler(sig, frame):
        print("\nShutting down...")
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    try:
        main()
    except KeyboardInterrupt:
        print("\nGoodbye!")
    except Exception as e:
        print(f"\nFatal error: {e}")