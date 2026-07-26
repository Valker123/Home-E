import ollama

response = ollama.generate(
    model='mistral:latest',
    prompt='Explain Python in one sentence.'
)

# Access the response text
print(response.response)
