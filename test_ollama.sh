#!/bin/bash

# 测试 Ollama 连接和解析
echo "Testing Ollama connection and response parsing..."

# 发送请求
RESPONSE=$(curl -s -X POST http://localhost:11434/api/generate \
    -H "Content-Type: application/json" \
    -d '{"model": "qwen3:4b", "prompt": "List files in current directory. Give me only the shell command:", "stream": false}')

echo "Full response:"
echo "$RESPONSE" | jq '.'

echo -e "\nExtracted response:"
echo "$RESPONSE" | jq -r '.response'

echo -e "\nCleaned response (removing think tags):"
echo "$RESPONSE" | jq -r '.response' | sed ':a;N;$!ba;s/<think>.*<\/think>//g' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//'
