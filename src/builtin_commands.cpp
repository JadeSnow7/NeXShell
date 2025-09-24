#include "builtin_commands.h"
#include "shell.h"
#include "ai_assistant.h"
#include "utils.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <string>

namespace NeXShell {

BuiltinCommands::BuiltinCommands(Shell* shell) : shell_(shell) {
    initialize_commands();
}

void BuiltinCommands::initialize_commands() {
    commands_["cd"] = [this](const std::vector<std::string>& args) { return cmd_cd(args); };
    commands_["pwd"] = [this](const std::vector<std::string>& args) { return cmd_pwd(args); };
    commands_["exit"] = [this](const std::vector<std::string>& args) { return cmd_exit(args); };
    commands_["help"] = [this](const std::vector<std::string>& args) { return cmd_help(args); };
    commands_["history"] = [this](const std::vector<std::string>& args) { return cmd_history(args); };
    commands_["echo"] = [this](const std::vector<std::string>& args) { return cmd_echo(args); };
    commands_["export"] = [this](const std::vector<std::string>& args) { return cmd_export(args); };
    commands_["unset"] = [this](const std::vector<std::string>& args) { return cmd_unset(args); };
    commands_["jobs"] = [this](const std::vector<std::string>& args) { return cmd_jobs(args); };
    commands_["fg"] = [this](const std::vector<std::string>& args) { return cmd_fg(args); };
    commands_["bg"] = [this](const std::vector<std::string>& args) { return cmd_bg(args); };
    commands_["ai"] = [this](const std::vector<std::string>& args) { return cmd_ai(args); };
}

bool BuiltinCommands::is_builtin(const std::string& command_name) const {
    return commands_.find(command_name) != commands_.end();
}

int BuiltinCommands::execute(const Command& command) {
    auto it = commands_.find(command.program);
    if (it != commands_.end()) {
        return it->second(command.arguments);
    }
    return 1; // 命令不存在
}

std::vector<std::string> BuiltinCommands::get_builtin_commands() const {
    std::vector<std::string> command_names;
    for (const auto& pair : commands_) {
        command_names.push_back(pair.first);
    }
    return command_names;
}

int BuiltinCommands::cmd_cd(const std::vector<std::string>& args) {
    std::string path = shell_->get_environment_variable("HOME");
    
    if (!args.empty()) {
        path = args[0];
    }
    
    if (shell_->change_directory(path)) {
        return 0;
    } else {
        return 1;
    }
}

int BuiltinCommands::cmd_pwd(const std::vector<std::string>& args) {
    (void)args; // 未使用的参数
    std::cout << shell_->get_current_directory() << std::endl;
    return 0;
}

int BuiltinCommands::cmd_exit(const std::vector<std::string>& args) {
    int exit_code = 0;
    if (!args.empty()) {
        exit_code = Utils::safe_stoi(args[0], 0);
    }
    
    shell_->request_exit();
    return exit_code;
}

int BuiltinCommands::cmd_help(const std::vector<std::string>& args) {
    (void)args; // 未使用的参数
    
    std::cout << "CppShell - Built-in Commands:\n\n";
    std::cout << "  cd [directory]    - Change current directory\n";
    std::cout << "  pwd              - Print current directory\n";
    std::cout << "  exit [code]      - Exit the shell\n";
    std::cout << "  help             - Show this help message\n";
    std::cout << "  history          - Show command history\n";
    std::cout << "  echo [text]      - Display text\n";
    std::cout << "  export VAR=value - Set environment variable\n";
    std::cout << "  unset VAR        - Unset environment variable\n";
    std::cout << "  jobs             - List active jobs\n";
    std::cout << "  fg [job]         - Bring job to foreground\n";
    std::cout << "  bg [job]         - Send job to background\n";
    std::cout << "\nSupported features:\n";
    std::cout << "  - Pipes (|)\n";
    std::cout << "  - Redirection (>, <, >>)\n";
    std::cout << "  - Background execution (&)\n";
    std::cout << "  - Environment variables ($VAR)\n";
    std::cout << "  - Tab completion\n";
    std::cout << "  - Command history\n";
    
    return 0;
}

int BuiltinCommands::cmd_history(const std::vector<std::string>& args) {
    (void)args; // 未使用的参数
    
    const auto& history = shell_->get_history();
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << "  " << (i + 1) << "  " << history[i] << std::endl;
    }
    
    return 0;
}

int BuiltinCommands::cmd_echo(const std::vector<std::string>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i];
    }
    std::cout << std::endl;
    
    return 0;
}

int BuiltinCommands::cmd_export(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "export: usage: export VAR=value" << std::endl;
        return 1;
    }
    
    for (const auto& arg : args) {
        size_t pos = arg.find('=');
        if (pos != std::string::npos) {
            std::string name = arg.substr(0, pos);
            std::string value = arg.substr(pos + 1);
            shell_->set_environment_variable(name, value);
        } else {
            std::cerr << "export: invalid format: " << arg << std::endl;
            return 1;
        }
    }
    
    return 0;
}

int BuiltinCommands::cmd_unset(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "unset: usage: unset VAR" << std::endl;
        return 1;
    }
    
    for (const auto& var : args) {
        unsetenv(var.c_str());
    }
    
    return 0;
}

int BuiltinCommands::cmd_jobs(const std::vector<std::string>& args) {
    (void)args; // 未使用的参数
    
    // TODO: 实现作业控制
    std::cout << "No active jobs" << std::endl;
    
    return 0;
}

int BuiltinCommands::cmd_fg(const std::vector<std::string>& args) {
    (void)args; // 未使用的参数
    
    // TODO: 实现前台作业控制
    std::cout << "fg: no current job" << std::endl;
    
    return 1;
}

int BuiltinCommands::cmd_bg(const std::vector<std::string>& args) {
    (void)args; // 未使用的参数
    
    // TODO: 实现后台作业控制
    std::cout << "bg: no current job" << std::endl;
    
    return 1;
}

int BuiltinCommands::cmd_ai(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "AI Assistant Usage:\n";
        std::cout << "  ai \"describe what you want to do\"\n";
        std::cout << "  ai explain <command>\n";
        std::cout << "  ai suggest <task>\n";
        std::cout << "  ai status\n";
        std::cout << "\nExamples:\n";
        std::cout << "  ai \"find all .txt files in current directory\"\n";
        std::cout << "  ai explain \"ls -la\"\n";
        std::cout << "  ai suggest \"backup my files\"\n";
        return 0;
    }
    
    AIAssistant* ai = shell_->get_ai_assistant();
    if (!ai) {
        std::cout << "AI Assistant not available. Please check if Ollama is running." << std::endl;
        return 1;
    }
    
    std::string first_arg = args[0];
    
    if (first_arg == "status") {
        if (ai->is_ai_enabled()) {
            std::cout << "AI Assistant is enabled using model: " << ai->get_current_model() << std::endl;
        } else {
            std::cout << "AI Assistant is disabled. Check Ollama service." << std::endl;
        }
        return 0;
    }
    
    if (first_arg == "explain" && args.size() > 1) {
        std::string command = Utils::join(std::vector<std::string>(args.begin() + 1, args.end()), " ");
        std::string explanation = ai->explain_command(command);
        std::cout << "Explanation: " << explanation << std::endl;
        return 0;
    }
    
    if (first_arg == "suggest" && args.size() > 1) {
        std::string task = Utils::join(std::vector<std::string>(args.begin() + 1, args.end()), " ");
        auto suggestions = ai->suggest_commands(task);
        
        if (suggestions.empty()) {
            std::cout << "No suggestions available." << std::endl;
            return 1;
        }
        
        std::cout << "Suggested commands for '" << task << "':" << std::endl;
        for (size_t i = 0; i < suggestions.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << suggestions[i] << std::endl;
        }
        return 0;
    }
    
    // 默认处理：自然语言命令
    std::string natural_input = Utils::join(args, " ");
    std::string result = ai->process_natural_command(natural_input);
    
    if (result.find("Error:") == 0 || result.find("AI Response:") == 0) {
        std::cout << result << std::endl;
        return 1;
    }
    
    // 如果返回的是一个命令，询问用户是否执行
    std::cout << "AI suggests: " << result << std::endl;
    std::cout << "Execute this command? [y/N]: ";
    
    std::string response;
    std::getline(std::cin, response);
    
    if (response == "y" || response == "Y" || response == "yes") {
        // 执行建议的命令
        int exit_code = shell_->execute_command(result);
        return exit_code;
    } else {
        std::cout << "Command not executed." << std::endl;
        return 0;
    }
}

} // namespace NeXShell
