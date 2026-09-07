import time
import ollama
from RPi.mainPipeline import TOOLS

print("Starting Qwen diagnostic test...")

start = time.time()

response = ollama.chat(
    model="qwen2.5:1.5b",
    keep_alive=-1,
messages=[
    {
        "role": "user",
        "content": "what time is it right now"
    }
],
tools=TOOLS,
)

elapsed = time.time() - start

print("\n========== RESPONSE ==========")
print(response)

print("\n========== TIMING ==========")
print(f"Total Python time: {elapsed:.3f} seconds")

if hasattr(response, "prompt_eval_duration"):
    print(f"Prompt evaluation: {response.prompt_eval_duration / 1e9:.3f} seconds")
    print(f"Prompt tokens: {response.prompt_eval_count}")

if hasattr(response, "eval_duration"):
    print(f"Generation: {response.eval_duration / 1e9:.3f} seconds")
    print(f"Generated tokens: {response.eval_count}")

if hasattr(response, "load_duration"):
    print(f"Model loading: {response.load_duration / 1e9:.3f} seconds")

if hasattr(response, "total_duration"):
    print(f"Ollama total duration: {response.total_duration / 1e9:.3f} seconds")
