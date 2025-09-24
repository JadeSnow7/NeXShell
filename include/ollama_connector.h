#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace NeXShell {

/**
 * @brief Ollama API 连接器
 */
class OllamaConnector {
public:
    OllamaConnector(const std::string& api_endpoint = "http://localhost:11434");
    ~OllamaConnector() = default;

    /**
     * @brief 查询 Ollama 模型
     * @param prompt 输入提示
     * @param model 模型名称
     * @return 模型响应
     */
    std::string query_model(const std::string& prompt, const std::string& model = "qwen3:4b");

    /**
     * @brief 检查 Ollama 服务是否可用
     * @return 如果服务可用返回 true
     */
    bool is_service_available();

    /**
     * @brief 获取可用的模型列表
     * @return 模型名称列表
     */
    std::vector<std::string> get_available_models();

    /**
     * @brief 设置请求超时时间
     * @param timeout_seconds 超时秒数
     */
    void set_timeout(int timeout_seconds);

private:
    /**
     * @brief 发送 HTTP POST 请求
     * @param endpoint API 端点
     * @param json_data JSON 数据
     * @return 响应内容
     */
    std::string send_http_request(const std::string& endpoint, const std::string& json_data);

    /**
     * @brief 解析 JSON 响应
     * @param response JSON 响应字符串
     * @return 解析后的文本内容
     */
    std::string parse_ollama_response(const std::string& response);

    /**
     * @brief 执行系统命令
     * @param command 要执行的命令
     * @return 命令输出
     */
    std::string execute_command(const std::string& command);

private:
    std::string api_endpoint_;
    int timeout_seconds_;
};

} // namespace NeXShell
