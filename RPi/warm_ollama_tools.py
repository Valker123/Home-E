import time
import ollama
from Latency import TOOLS

print("Starting Qwen tool-calling warm-up...")

start = time.time()

response = ollama.chat(
    model="qwen2.5:1.5b",
    keep_alive=-1,
    messages=[
        {
            "role": "system",
            "content": (
                "If the user's request matches an available tool, "
                "call that tool directly. Only respond with plain text "
                "if no tool applies, and in that case keep your response "
                "to one short sentence."
            ),
        },
        {
            "role": "user",
            "content": "What time is it?",
        },
    ],
    tools=TOOLS,
)

elapsed = time.time() - start

print(f"Qwen tool-calling warm-up complete in {elapsed:.3f} seconds.")
print(f"Prompt evaluation: {response.prompt_eval_duration / 1e9:.3f} seconds")
print(f"Prompt tokens: {response.prompt_eval_count}")
print(f"Generation: {response.eval_duration / 1e9:.3f} seconds")
print(f"Total Ollama time: {response.total_duration / 1e9:.3f} seconds")