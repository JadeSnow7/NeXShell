#pragma once

#include "command_parser.h"
#include <functional>
#include <unordered_map>

namespace NeXShell {

class Shell;
class AIAssistant;

/**
 * @brief 内建命令处理器类
 */
class BuiltinCommands {
public:
    explicit BuiltinCommands(Shell* shell);
    ~BuiltinCommands() = default;

    /**
     * @brief 检查是否为内建命令
     * @param command_name 命令名
     * @return 如果是内建命令返回 true
     */
    bool is_builtin(const std::string& command_name) const;

    /**
     * @brief 执行内建命令
     * @param command 命令对象
     * @return 命令退出码
     */
    int execute(const Command& command);

    /**
     * @brief 获取所有内建命令的列表
     * @return 内建命令名称列表
     */
    std::vector<std::string> get_builtin_commands() const;

private:
    // 内建命令处理函数类型
    using CommandHandler = std::function<int(const std::vector<std::string>&)>;

    /**
     * @brief 初始化内建命令映射
     */
    void initialize_commands();

    // 各种内建命令的实现
    int cmd_cd(const std::vector<std::string>& args);
    int cmd_pwd(const std::vector<std::string>& args);
    int cmd_exit(const std::vector<std::string>& args);
    int cmd_help(const std::vector<std::string>& args);
    int cmd_history(const std::vector<std::string>& args);
    int cmd_echo(const std::vector<std::string>& args);
    int cmd_export(const std::vector<std::string>& args);
    int cmd_unset(const std::vector<std::string>& args);
    int cmd_jobs(const std::vector<std::string>& args);
    int cmd_fg(const std::vector<std::string>& args);
    int cmd_bg(const std::vector<std::string>& args);
    int cmd_ai(const std::vector<std::string>& args);

private:
    Shell* shell_;
    std::unordered_map<std::string, CommandHandler> commands_;
};

} // namespace NeXShell
