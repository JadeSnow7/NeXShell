#pragma once

#include <string>
#include <vector>
#include <optional>

namespace NeXShell {

/**
 * @brief 表示一个解析后的命令
 */
struct Command {
    std::string program;                    // 程序名
    std::vector<std::string> arguments;     // 参数列表
    std::optional<std::string> input_file;  // 输入重定向文件
    std::optional<std::string> output_file; // 输出重定向文件
    bool append_output = false;             // 是否追加输出
    bool run_in_background = false;         // 是否后台运行
};

/**
 * @brief 表示一个管道命令序列
 */
struct Pipeline {
    std::vector<Command> commands;          // 管道中的命令列表
    bool run_in_background = false;         // 是否后台运行
};

/**
 * @brief 命令解析器类
 */
class CommandParser {
public:
    CommandParser() = default;
    ~CommandParser() = default;

    /**
     * @brief 解析命令行
     * @param input 用户输入的命令字符串
     * @return 解析后的管道对象
     */
    Pipeline parse(const std::string& input);

    /**
     * @brief 检查输入是否为空或只包含空白字符
     * @param input 输入字符串
     * @return 如果为空返回 true
     */
    static bool is_empty(const std::string& input);

    /**
     * @brief 去除字符串首尾空白字符
     * @param str 要处理的字符串
     * @return 处理后的字符串
     */
    static std::string trim(const std::string& str);

private:
    /**
     * @brief 将输入字符串分割为 token
     * @param input 输入字符串
     * @return token 列表
     */
    std::vector<std::string> tokenize(const std::string& input);

    /**
     * @brief 解析单个命令
     * @param tokens token 列表
     * @param start 开始位置
     * @param end 结束位置
     * @return 解析后的命令对象
     */
    Command parse_command(const std::vector<std::string>& tokens, 
                         size_t start, size_t end);

    /**
     * @brief 展开环境变量
     * @param token 包含环境变量的 token
     * @return 展开后的字符串
     */
    std::string expand_variables(const std::string& token);

    /**
     * @brief 处理引号
     * @param token 包含引号的 token
     * @return 处理后的字符串
     */
    std::string handle_quotes(const std::string& token);

    /**
     * @brief 检查字符是否为特殊字符
     * @param c 要检查的字符
     * @return 如果是特殊字符返回 true
     */
    bool is_special_char(char c) const;
};

} // namespace NeXShell
