#include "ollama_connector.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstddef>
#include <unistd.h>

namespace NeXShell {

OllamaConnector::OllamaConnector(const std::string& api_endpoint) 
    : api_endpoint_(api_endpoint), timeout_seconds_(30) {}

std::string OllamaConnector::query_model(const std::string& prompt, const std::string& model) {
    if (!is_service_available()) {
        return "Error: Ollama service is not available. Please start Ollama first.";
    }

    // 构建请求JSON
    std::string json_data = R"({
        "model": ")" + model + R"(",
        "prompt": ")" + prompt + R"(",
        "stream": false
    })";

    // 创建临时文件保存JSON数据
    std::string temp_file = "/tmp/ollama_request_" + std::to_string(getpid()) + ".json";
    std::string cmd = "echo '" + json_data + "' > " + temp_file;
    system(cmd.c_str());

    // 使用curl发送请求
    std::string curl_cmd = "curl -s -X POST " + api_endpoint_ + "/api/generate -H 'Content-Type: application/json' -d @" + temp_file;
    std::string response = execute_command(curl_cmd);

    // 清理临时文件
    std::string cleanup_cmd = "rm -f " + temp_file;
    system(cleanup_cmd.c_str());

    if (response.empty()) {
        return "Error: No response from Ollama service";
    }

    return parse_ollama_response(response);
}

bool OllamaConnector::is_service_available() {
    try {
        std::string check_cmd = "curl -s " + api_endpoint_ + "/api/tags";
        std::string response = execute_command(check_cmd);
        return !response.empty() && response.find("models") != std::string::npos;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> OllamaConnector::get_available_models() {
    std::vector<std::string> models;
    
    try {
        std::string cmd = "curl -s " + api_endpoint_ + "/api/tags";
        std::string response = execute_command(cmd);
        
        // 使用简单的字符串查找提取模型名称
        size_t pos = 0;
        while ((pos = response.find("\"name\":\"", pos)) != std::string::npos) {
            pos += 8; // 跳过 "name":"
            size_t end_pos = response.find("\"", pos);
            if (end_pos != std::string::npos) {
                std::string model_name = response.substr(pos, end_pos - pos);
                models.push_back(model_name);
                pos = end_pos;
            }
        }
    } catch (...) {
        // 发生错误时返回默认模型列表
        models.push_back("qwen3:4b");
    }
    
    return models;
}

void OllamaConnector::set_timeout(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
}

std::string OllamaConnector::send_http_request(const std::string& endpoint, const std::string& json_data) {
    // 创建临时文件保存JSON数据
    std::string temp_file = "/tmp/ollama_json_" + std::to_string(getpid()) + ".json";
    std::string cmd = "echo '" + json_data + "' > " + temp_file;
    system(cmd.c_str());

    // 使用curl发送请求
    std::string curl_cmd = "curl -s -X POST " + api_endpoint_ + endpoint + 
                          " -H 'Content-Type: application/json' -d @" + temp_file +
                          " --max-time " + std::to_string(timeout_seconds_);
    std::string response = execute_command(curl_cmd);

    // 清理临时文件
    std::string cleanup_cmd = "rm -f " + temp_file;
    system(cleanup_cmd.c_str());

    return response;
}

std::string OllamaConnector::parse_ollama_response(const std::string& response) {
    // 首先尝试使用jq解析
    std::string temp_file = "/tmp/ollama_response_" + std::to_string(getpid()) + ".json";
    std::string save_cmd = "echo '" + response + "' > " + temp_file;
    system(save_cmd.c_str());
    
    std::string jq_cmd = "jq -r '.response // .error // \"parse_error\"' " + temp_file + " 2>/dev/null";
    std::string jq_result = execute_command(jq_cmd);
    
    // 清理临时文件
    std::string cleanup_cmd = "rm -f " + temp_file;
    system(cleanup_cmd.c_str());
    
    // 如果jq成功解析且不是错误
    if (!jq_result.empty() && jq_result != "parse_error" && 
        jq_result != "null" && jq_result != "\"null\"" && 
        jq_result.find("parse error") == std::string::npos) {
        // 移除可能的引号和换行符
        if (jq_result.length() >= 2 && jq_result.front() == '"' && jq_result.back() == '"') {
            jq_result = jq_result.substr(1, jq_result.length() - 2);
        }
        // 移除尾部换行符
        if (!jq_result.empty() && jq_result.back() == '\n') {
            jq_result.pop_back();
        }
        return jq_result;
    }
    
    // 如果jq失败，尝试简单的字符串解析
    size_t response_pos = response.find("\"response\":\"");
    if (response_pos != std::string::npos) {
        response_pos += 12; // 跳过 "response":"
        size_t end_pos = response_pos;
        
        // 找到响应内容的结束位置，注意处理转义字符
        while (end_pos < response.length()) {
            if (response[end_pos] == '"' && (end_pos == 0 || response[end_pos - 1] != '\\')) {
                break;
            }
            end_pos++;
        }
        
        if (end_pos < response.length()) {
            std::string result = response.substr(response_pos, end_pos - response_pos);
            
            // 处理基本的转义字符
            size_t pos = 0;
            while ((pos = result.find("\\n", pos)) != std::string::npos) {
                result.replace(pos, 2, "\n");
                pos += 1;
            }
            
            pos = 0;
            while ((pos = result.find("\\t", pos)) != std::string::npos) {
                result.replace(pos, 2, "\t");
                pos += 1;
            }
            
            pos = 0;
            while ((pos = result.find("\\\"", pos)) != std::string::npos) {
                result.replace(pos, 2, "\"");
                pos += 1;
            }
            
            return result;
        }
    }
    
    // 检查是否有错误信息
    if (response.find("error") != std::string::npos) {
        return "Error: " + response;
    }
    
    return "Failed to parse response. Raw response: " + response.substr(0, 200) + "...";
}

std::string OllamaConnector::execute_command(const std::string& command) {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    
    return result;
}

} // namespace NeXShell
