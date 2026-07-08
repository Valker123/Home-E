# TODO: Run Ollama using Python
import requests

response = requests.post(
    "http://localhost:11434/api/chat",
    json={
        "model": "mistral",
        "messages": [
            {"role": "user", "content": "hello"}
        ],
        "stream": False
    }
)

print(response.json()["message"]["content"])
