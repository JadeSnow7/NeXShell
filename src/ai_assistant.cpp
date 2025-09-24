#include "ai_assistant.h"
#include "shell.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <unistd.h>
#include <cstdlib>

namespace NeXShell {

AIAssistant::AIAssistant(Shell* shell) 
    : shell_(shell), current_model_("llama3.2"), ai_enabled_(false) {
    
    // 初始化危险命令列表
    dangerous_commands_ = {
        "rm -rf /",
        "rm -rf /*",
        "dd if=/dev/zero",
        "mkfs",
        "fdisk",
        "format",
        "del /f /s /q C:\\",
        "shutdown -h now",
        "reboot",
        "halt",
        "init 0",
        "kill -9 -1",
        ":(){ :|:& };:",  // fork bomb
        "chmod -R 777 /",
        "chown -R root:root /",
        "sudo rm -rf"
    };
}

bool AIAssistant::initialize(const std::string& model_name) {
    current_model_ = model_name;
    ollama_ = std::make_unique<OllamaConnector>();
    
    // 检查 Ollama 服务是否可用
    if (!ollama_->is_service_available()) {
        std::cerr << "Warning: Ollama service not available. AI features disabled." << std::endl;
        return false;
    }
    
    // 检查模型是否可用
    auto models = ollama_->get_available_models();
    bool model_found = std::find(models.begin(), models.end(), model_name) != models.end();
    
    if (!model_found && !models.empty()) {
        std::cerr << "Warning: Model '" << model_name << "' not found. Using: " << models[0] << std::endl;
        current_model_ = models[0];
    } else if (models.empty()) {
        std::cerr << "Warning: No models available in Ollama." << std::endl;
        return false;
    }
    
    ai_enabled_ = true;
    std::cout << "AI Assistant initialized with model: " << current_model_ << std::endl;
    return true;
}

bool AIAssistant::auto_initialize(const std::string& model_name) {
    current_model_ = model_name;
    
    // 首先检查Ollama服务状态并处理用户选择
    if (!check_and_handle_ollama_service()) {
        std::cout << "AI features disabled." << std::endl;
        return false;
    }
    
    // 服务可用后，继续正常初始化流程
    auto models = ollama_->get_available_models();
    bool model_found = std::find(models.begin(), models.end(), model_name) != models.end();
    
    if (!model_found && !models.empty()) {
        std::cout << "Warning: Model '" << model_name << "' not found. Using: " << models[0] << std::endl;
        current_model_ = models[0];
    } else if (models.empty()) {
        std::cout << "Warning: No models available in Ollama." << std::endl;
        return false;
    }
    
    ai_enabled_ = true;
    std::cout << "AI Assistant initialized with model: " << current_model_ << std::endl;
    return true;
}

std::string AIAssistant::process_natural_command(const std::string& natural_input) {
    if (!ai_enabled_) {
        return "AI features are not available. Please check if Ollama is running.";
    }
    
    try {
        std::string context_prompt = build_context_prompt(natural_input);
        std::string ai_response = ollama_->query_model(context_prompt, current_model_);
        
        // 从 AI 响应中提取命令
        std::string command = extract_command_from_response(ai_response);
        
        if (command.empty()) {
            return "AI Response: " + ai_response;
        }
        
        // 验证命令安全性
        if (!is_command_safe(command)) {
            return "Unsafe command detected: " + command + "\nFor safety, this command was not executed.";
        }
        
        // 记录到历史
        command_history_.push_back({natural_input, command});
        if (command_history_.size() > MAX_HISTORY_SIZE) {
            command_history_.erase(command_history_.begin());
        }
        
        return command;
        
    } catch (const std::exception& e) {
        return "Error processing command: " + std::string(e.what());
    }
}

std::string AIAssistant::explain_command(const std::string& command) {
    if (!ai_enabled_) {
        return "AI features are not available.";
    }
    
    std::string prompt = "Explain what this Linux command does in simple terms:\n" + command;
    return ollama_->query_model(prompt, current_model_);
}

std::vector<std::string> AIAssistant::suggest_commands(const std::string& intent) {
    std::vector<std::string> suggestions;
    
    if (!ai_enabled_) {
        return suggestions;
    }
    
    std::string prompt = "Suggest 3 Linux commands for this task: " + intent + 
                        "\nReturn only the commands, one per line, no explanations.";
    
    std::string response = ollama_->query_model(prompt, current_model_);
    
    // 简单分割响应为命令列表
    std::istringstream iss(response);
    std::string line;
    while (std::getline(iss, line) && suggestions.size() < 3) {
        line = Utils::trim(line);
        if (!line.empty() && line[0] != '#') {
            suggestions.push_back(line);
        }
    }
    
    return suggestions;
}

std::string AIAssistant::build_system_prompt() {
    return R"(You are a Linux shell command assistant. Your job is to convert natural language requests into appropriate Linux shell commands.

Rules:
1. Return ONLY the command, no explanations unless specifically asked
2. Use safe, commonly available Linux commands
3. Be precise and avoid dangerous operations
4. If the request is unclear, ask for clarification
5. For file operations, use relative paths unless absolute paths are specified

Current directory: )" + shell_->get_current_directory() + R"(
Available files: )" + get_system_context();
}

std::string AIAssistant::build_context_prompt(const std::string& user_input) {
    std::ostringstream prompt;
    
    prompt << build_system_prompt() << "\n\n";
    
    // 添加最近的命令历史作为上下文
    if (!command_history_.empty()) {
        prompt << "Recent commands:\n";
        for (const auto& hist : command_history_) {
            prompt << "User: " << hist.first << " -> Command: " << hist.second << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "User request: " << user_input << "\n";
    prompt << "Command:";
    
    return prompt.str();
}

bool AIAssistant::is_command_safe(const std::string& command) {
    // 检查命令是否在危险命令列表中
    for (const auto& dangerous : dangerous_commands_) {
        if (command.find(dangerous) != std::string::npos) {
            return false;
        }
    }
    
    // 检查其他危险模式
    if (command.find("rm -rf") != std::string::npos && 
        (command.find("/*") != std::string::npos || command.find("/ ") != std::string::npos)) {
        return false;
    }
    
    // 检查是否尝试修改系统关键目录
    std::vector<std::string> critical_dirs = {"/bin", "/sbin", "/usr", "/lib", "/etc", "/boot"};
    for (const auto& dir : critical_dirs) {
        if (command.find("rm") != std::string::npos && command.find(dir) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

std::string AIAssistant::extract_command_from_response(const std::string& ai_response) {
    std::string response = Utils::trim(ai_response);
    
    // 如果响应看起来像一个命令（不包含解释性文字）
    if (response.length() < 200 && 
        response.find('\n') == std::string::npos &&
        (response.find("ls") != std::string::npos ||
         response.find("cd") != std::string::npos ||
         response.find("mkdir") != std::string::npos ||
         response.find("cp") != std::string::npos ||
         response.find("mv") != std::string::npos ||
         response.find("find") != std::string::npos ||
         response.find("grep") != std::string::npos ||
         response.find("cat") != std::string::npos ||
         response.find("echo") != std::string::npos)) {
        return response;
    }
    
    // 尝试从多行响应中提取第一个命令
    std::istringstream iss(response);
    std::string line;
    while (std::getline(iss, line)) {
        line = Utils::trim(line);
        if (!line.empty() && line[0] != '#' && line.find("command:") == std::string::npos) {
            // 简单检查是否看起来像命令
            if (line.length() < 100 && line.find(' ') != std::string::npos) {
                return line;
            }
        }
    }
    
    return ""; // 无法提取命令，返回空字符串
}

std::string AIAssistant::get_system_context() {
    // 获取当前目录的文件列表作为上下文
    try {
        // 使用 shell 执行 ls 命令
        // 这里简化实现，实际应该通过 shell 的接口获取
        return "Use 'ls' to see current directory contents";
    } catch (...) {
        return "Unable to get directory context";
    }
}

// CommandValidator 实现
CommandValidator::CommandValidator() {
    initialize_dangerous_patterns();
}

void CommandValidator::initialize_dangerous_patterns() {
    dangerous_commands_ = {
        "rm -rf /",
        "rm -rf /*",
        "dd if=/dev/zero",
        "mkfs",
        "fdisk",
        "format",
        "shutdown",
        "reboot",
        "halt",
        "init 0",
        "kill -9 -1"
    };
    
    dangerous_patterns_ = {
        "rm -rf",
        "dd if=",
        "mkfs.",
        ":(){ :|:& };:",
        "chmod -R 777 /",
        "chown -R"
    };
    
    safe_alternatives_ = {
        {"rm -rf /", "Use 'rm -rf directory_name' with specific directory"},
        {"dd if=/dev/zero", "Use 'dd' with specific of= parameter"},
        {"shutdown", "Use 'shutdown -h +5' to schedule shutdown"},
        {"reboot", "Use 'sudo reboot' if system restart is needed"}
    };
}

bool CommandValidator::is_safe(const std::string& command) {
    // 检查完全匹配的危险命令
    for (const auto& dangerous : dangerous_commands_) {
        if (command.find(dangerous) != std::string::npos) {
            return false;
        }
    }
    
    // 检查危险模式
    for (const auto& pattern : dangerous_patterns_) {
        if (command.find(pattern) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

std::string CommandValidator::get_danger_reason(const std::string& command) {
    for (const auto& dangerous : dangerous_commands_) {
        if (command.find(dangerous) != std::string::npos) {
            return "Contains dangerous command: " + dangerous;
        }
    }
    
    for (const auto& pattern : dangerous_patterns_) {
        if (command.find(pattern) != std::string::npos) {
            return "Contains dangerous pattern: " + pattern;
        }
    }
    
    return "Command appears safe";
}

std::string CommandValidator::suggest_safer_alternative(const std::string& dangerous_command) {
    for (const auto& alt : safe_alternatives_) {
        if (dangerous_command.find(alt.first) != std::string::npos) {
            return alt.second;
        }
    }
    
    return "No specific alternative available. Please review the command carefully.";
}

bool AIAssistant::check_and_handle_ollama_service() {
    // 先尝试连接默认的Ollama服务
    ollama_ = std::make_unique<OllamaConnector>();
    
    if (ollama_->is_service_available()) {
        std::cout << "✓ Ollama service is running." << std::endl;
        return true;
    }
    
    std::cout << "⚠ Ollama service is not running." << std::endl;
    std::cout << "Choose an option:" << std::endl;
    std::cout << "  1) Start local Ollama service" << std::endl;
    std::cout << "  2) Use remote API endpoint" << std::endl;
    std::cout << "  3) Disable AI features" << std::endl;
    std::cout << "Enter your choice (1-3): ";
    
    std::string choice;
    std::getline(std::cin, choice);
    
    if (choice == "1") {
        return start_ollama_service();
    } else if (choice == "2") {
        std::cout << "Enter API endpoint (e.g., http://remote-server:11434): ";
        std::string api_endpoint;
        std::getline(std::cin, api_endpoint);
        return setup_api_mode(api_endpoint);
    } else {
        std::cout << "AI features will be disabled." << std::endl;
        return false;
    }
}

bool AIAssistant::start_ollama_service() {
    std::cout << "Attempting to start Ollama service..." << std::endl;
    
    // 尝试启动ollama服务
    int result = system("ollama serve > /dev/null 2>&1 &");
    
    if (result != 0) {
        std::cout << "Failed to start Ollama service automatically." << std::endl;
        std::cout << "Please start it manually by running: ollama serve" << std::endl;
        return false;
    }
    
    // 等待服务启动
    std::cout << "Waiting for Ollama service to start";
    for (int i = 0; i < 10; ++i) {
        std::cout << ".";
        std::cout.flush();
        sleep(1);
        
        if (ollama_->is_service_available()) {
            std::cout << " ✓" << std::endl;
            std::cout << "Ollama service started successfully!" << std::endl;
            return true;
        }
    }
    
    std::cout << " ✗" << std::endl;
    std::cout << "Timeout waiting for Ollama service to start." << std::endl;
    std::cout << "Please check if Ollama is installed and try starting it manually." << std::endl;
    return false;
}

bool AIAssistant::setup_api_mode(const std::string& api_endpoint) {
    std::cout << "Testing connection to " << api_endpoint << "..." << std::endl;
    
    // 创建新的connector指向指定端点
    ollama_ = std::make_unique<OllamaConnector>(api_endpoint);
    
    if (ollama_->is_service_available()) {
        std::cout << "✓ Successfully connected to remote API." << std::endl;
        return true;
    } else {
        std::cout << "✗ Failed to connect to " << api_endpoint << std::endl;
        std::cout << "Please check the endpoint URL and network connectivity." << std::endl;
        return false;
    }
}

} // namespace NeXShell
