#include "shell.h"
#include "command_parser.h"
#include "command_executor.h"
#include "ai_assistant.h"
#include "utils.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <vector>
#include <string>
extern char **environ;
// #include <readline/readline.h>
// #include <readline/history.h>

namespace NeXShell {

// 全局 Shell 指针，用于信号处理
static Shell* g_shell_instance = nullptr;

// 信号处理函数
void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:  // Ctrl+C
            std::cout << "\n";
            if (g_shell_instance) {
                std::cout << g_shell_instance->get_current_directory() << "$ ";
                std::cout.flush();
            }
            break;
        case SIGTSTP: // Ctrl+Z
            std::cout << "\n[Process suspended]\n";
            break;
        default:
            break;
    }
}

Shell::Shell() : exit_requested_(false) {
    g_shell_instance = this;
    initialize();
}

Shell::~Shell() {
    g_shell_instance = nullptr;
}

void Shell::initialize() {
    // 初始化解析器和执行器
    parser_ = std::make_unique<CommandParser>();
    executor_ = std::make_unique<CommandExecutor>(this);
    
    // 初始化 AI 助手
    ai_assistant_ = std::make_unique<AIAssistant>(this);
    ai_assistant_->auto_initialize(); // 自动检查并初始化AI服务
    
    // 获取当前工作目录
    char* cwd = getcwd(nullptr, 0);
    if (cwd) {
        current_directory_ = cwd;
        free(cwd);
    } else {
        current_directory_ = "/";
    }
    
    // 设置信号处理
    setup_signal_handlers();
    
    // 初始化环境变量
    char** env = environ;
    while (*env) {
        std::string env_str(*env);
        size_t pos = env_str.find('=');
        if (pos != std::string::npos) {
            std::string name = env_str.substr(0, pos);
            std::string value = env_str.substr(pos + 1);
            environment_variables_[name] = value;
        }
        ++env;
    }
}

void Shell::setup_signal_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
}

void Shell::run() {
    while (!should_exit()) {
        std::string input = read_input();
        
        if (!input.empty() && !CommandParser::is_empty(input)) {
            add_to_history(input);
            int exit_code = execute_command(input);
            
            // 可以根据需要处理退出码
            if (exit_code != 0) {
                // 命令执行失败，但不退出 shell
            }
        }
        
        // 清理已完成的后台进程
        executor_->cleanup_background_processes();
    }
}

std::string Shell::read_input() {
    std::string prompt = get_prompt();
    std::cout << prompt;
    std::cout.flush();
    
    std::string input;
    if (!std::getline(std::cin, input)) {
        // EOF (Ctrl+D)
        request_exit();
        return "";
    }
    
    // 如果输入不为空，添加到历史记录（简单实现，不使用 readline）
    if (!input.empty() && !CommandParser::is_empty(input)) {
        // 历史记录在 add_to_history 中处理
    }
    
    return input;
}

std::string Shell::get_prompt() const {
    std::string user = get_environment_variable("USER");
    if (user.empty()) user = "user";
    
    std::string hostname = get_environment_variable("HOSTNAME");
    if (hostname.empty()) hostname = "localhost";
    
    std::string cwd = current_directory_;
    std::string home = get_environment_variable("HOME");
    if (!home.empty() && cwd.find(home) == 0) {
        cwd = "~" + cwd.substr(home.length());
    }
    
    return user + "@" + hostname + ":" + cwd + "$ ";
}

int Shell::execute_command(const std::string& command) {
    try {
        Pipeline pipeline = parser_->parse(command);
        return executor_->execute_pipeline(pipeline);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

void Shell::set_environment_variable(const std::string& name, const std::string& value) {
    environment_variables_[name] = value;
    setenv(name.c_str(), value.c_str(), 1);
}

std::string Shell::get_environment_variable(const std::string& name) const {
    auto it = environment_variables_.find(name);
    if (it != environment_variables_.end()) {
        return it->second;
    }
    
    const char* value = getenv(name.c_str());
    return value ? value : "";
}

std::string Shell::get_current_directory() const {
    return current_directory_;
}

bool Shell::change_directory(const std::string& path) {
    std::string target_path = path;
    
    // 处理特殊情况
    if (target_path.empty() || target_path == "~") {
        target_path = get_environment_variable("HOME");
    } else if (target_path == "-") {
        target_path = get_environment_variable("OLDPWD");
        if (target_path.empty()) {
            std::cerr << "cd: OLDPWD not set" << std::endl;
            return false;
        }
    } else if (target_path[0] == '~' && target_path[1] == '/') {
        target_path = get_environment_variable("HOME") + target_path.substr(1);
    }
    
    if (chdir(target_path.c_str()) == 0) {
        // 更新 OLDPWD
        set_environment_variable("OLDPWD", current_directory_);
        
        // 更新当前目录
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            current_directory_ = cwd;
            free(cwd);
            set_environment_variable("PWD", current_directory_);
            return true;
        }
    }
    
    perror("cd");
    return false;
}

void Shell::add_to_history(const std::string& command) {
    command_history_.push_back(command);
    
    // 限制历史记录大小
    const size_t max_history_size = 1000;
    if (command_history_.size() > max_history_size) {
        command_history_.erase(command_history_.begin());
    }
}

const std::vector<std::string>& Shell::get_history() const {
    return command_history_;
}

} // namespace NeXShell
