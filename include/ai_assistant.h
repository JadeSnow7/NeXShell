#pragma once

#include "ollama_connector.h"
#include <string>
#include <vector>
#include <memory>
#include <set>

namespace NeXShell {

class Shell;

/**
 * @brief AI 助手类，负责自然语言命令解析和安全验证
 */
class AIAssistant {
public:
    explicit AIAssistant(Shell* shell);
    ~AIAssistant() = default;

    /**
     * @brief 初始化 AI 助手
     * @param model_name 使用的模型名称
     * @return 初始化是否成功
     */
    bool initialize(const std::string& model_name = "llama3.2");

    /**
     * @brief 自动检查并初始化 AI 服务
     * @param model_name 使用的模型名称
     * @return 初始化是否成功
     */
    bool auto_initialize(const std::string& model_name = "llama3.2");

    /**
     * @brief 处理自然语言命令
     * @param natural_input 用户的自然语言输入
     * @return 解析后的命令或错误信息
     */
    std::string process_natural_command(const std::string& natural_input);

    /**
     * @brief 解释命令的作用
     * @param command 要解释的命令
     * @return 命令的解释
     */
    std::string explain_command(const std::string& command);

    /**
     * @brief 建议相关命令
     * @param intent 用户意图描述
     * @return 建议的命令列表
     */
    std::vector<std::string> suggest_commands(const std::string& intent);

    /**
     * @brief 检查是否启用了 AI 功能
     * @return 如果 AI 可用返回 true
     */
    bool is_ai_enabled() const { return ai_enabled_; }

    /**
     * @brief 获取当前使用的模型名称
     * @return 模型名称
     */
    const std::string& get_current_model() const { return current_model_; }

private:
    /**
     * @brief 构建系统提示词
     * @return 系统提示词
     */
    std::string build_system_prompt();

    /**
     * @brief 构建用户上下文
     * @param user_input 用户输入
     * @return 包含上下文的完整提示
     */
    std::string build_context_prompt(const std::string& user_input);

    /**
     * @brief 验证命令安全性
     * @param command 要验证的命令
     * @return 如果命令安全返回 true
     */
    bool is_command_safe(const std::string& command);

    /**
     * @brief 解析 AI 响应，提取命令
     * @param ai_response AI 的响应
     * @return 提取的命令
     */
    std::string extract_command_from_response(const std::string& ai_response);

    /**
     * @brief 获取当前系统上下文信息
     * @return 系统上下文字符串
     */
    std::string get_system_context();

    /**
     * @brief 检查Ollama服务状态并处理用户选择
     * @return 是否成功启动或连接服务
     */
    bool check_and_handle_ollama_service();

    /**
     * @brief 尝试启动本地Ollama服务
     * @return 启动是否成功
     */
    bool start_ollama_service();

    /**
     * @brief 设置使用API模式
     * @param api_endpoint API端点地址
     * @return 设置是否成功
     */
    bool setup_api_mode(const std::string& api_endpoint);

private:
    Shell* shell_;
    std::unique_ptr<OllamaConnector> ollama_;
    std::string current_model_;
    bool ai_enabled_;
    
    // 危险命令黑名单
    std::set<std::string> dangerous_commands_;
    
    // 命令历史记录（用于上下文）
    std::vector<std::pair<std::string, std::string>> command_history_;
    static const size_t MAX_HISTORY_SIZE = 10;
};

/**
 * @brief 命令安全验证器
 */
class CommandValidator {
public:
    CommandValidator();
    
    /**
     * @brief 检查命令是否安全
     * @param command 要检查的命令
     * @return 如果安全返回 true
     */
    bool is_safe(const std::string& command);
    
    /**
     * @brief 获取危险原因
     * @param command 命令
     * @return 危险原因描述
     */
    std::string get_danger_reason(const std::string& command);
    
    /**
     * @brief 建议更安全的替代命令
     * @param dangerous_command 危险命令
     * @return 安全的替代建议
     */
    std::string suggest_safer_alternative(const std::string& dangerous_command);

private:
    void initialize_dangerous_patterns();
    
private:
    std::set<std::string> dangerous_commands_;
    std::set<std::string> dangerous_patterns_;
    std::map<std::string, std::string> safe_alternatives_;
};

} // namespace NeXShell
