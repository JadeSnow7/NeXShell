#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace NeXShell {

class CommandParser;
class CommandExecutor;
class AIAssistant;

/**
 * @brief 主 Shell 类，负责整个 Shell 的运行逻辑
 */
class Shell {
public:
    Shell();
    ~Shell();

    /**
     * @brief 启动 Shell 主循环
     */
    void run();

    /**
     * @brief 执行单条命令
     * @param command 要执行的命令字符串
     * @return 命令执行的退出码
     */
    int execute_command(const std::string& command);

    /**
     * @brief 设置环境变量
     * @param name 变量名
     * @param value 变量值
     */
    void set_environment_variable(const std::string& name, const std::string& value);

    /**
     * @brief 获取环境变量
     * @param name 变量名
     * @return 变量值，如果不存在返回空字符串
     */
    std::string get_environment_variable(const std::string& name) const;

    /**
     * @brief 获取当前工作目录
     * @return 当前工作目录路径
     */
    std::string get_current_directory() const;

    /**
     * @brief 改变当前工作目录
     * @param path 目标目录路径
     * @return 成功返回 true，失败返回 false
     */
    bool change_directory(const std::string& path);

    /**
     * @brief 添加命令到历史记录
     * @param command 命令字符串
     */
    void add_to_history(const std::string& command);

    /**
     * @brief 获取命令历史记录
     * @return 历史记录列表
     */
    const std::vector<std::string>& get_history() const;

    /**
     * @brief 获取 AI 助手实例
     * @return AI 助手指针
     */
    AIAssistant* get_ai_assistant() const { return ai_assistant_.get(); }

    /**
     * @brief 检查是否应该退出 Shell
     * @return 如果应该退出返回 true
     */
    bool should_exit() const { return exit_requested_; }

    /**
     * @brief 请求退出 Shell
     */
    void request_exit() { exit_requested_ = true; }

private:
    /**
     * @brief 显示提示符
     * @return 提示符字符串
     */
    std::string get_prompt() const;

    /**
     * @brief 读取用户输入
     * @return 用户输入的命令字符串
     */
    std::string read_input();

    /**
     * @brief 初始化 Shell 环境
     */
    void initialize();

    /**
     * @brief 设置信号处理器
     */
    void setup_signal_handlers();

private:
    std::unique_ptr<CommandParser> parser_;
    std::unique_ptr<CommandExecutor> executor_;
    std::unique_ptr<AIAssistant> ai_assistant_;
    std::vector<std::string> command_history_;
    std::unordered_map<std::string, std::string> environment_variables_;
    bool exit_requested_;
    std::string current_directory_;
};

} // namespace NeXShell
