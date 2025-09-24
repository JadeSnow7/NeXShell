#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace NeXShell {

/**
 * @brief 工具函数集合
 */
namespace Utils {

    /**
     * @brief 分割字符串
     * @param str 要分割的字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串向量
     */
    std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief 分割字符串（支持多个分隔符）
     * @param str 要分割的字符串
     * @param delimiters 分隔符字符串
     * @return 分割后的字符串向量
     */
    std::vector<std::string> split(const std::string& str, const std::string& delimiters);

    /**
     * @brief 去除字符串首尾空白字符
     * @param str 要处理的字符串
     * @return 处理后的字符串
     */
    std::string trim(const std::string& str);

    /**
     * @brief 去除字符串左侧空白字符
     * @param str 要处理的字符串
     * @return 处理后的字符串
     */
    std::string ltrim(const std::string& str);

    /**
     * @brief 去除字符串右侧空白字符
     * @param str 要处理的字符串
     * @return 处理后的字符串
     */
    std::string rtrim(const std::string& str);

    /**
     * @brief 将字符串转换为小写
     * @param str 要转换的字符串
     * @return 转换后的字符串
     */
    std::string to_lower(const std::string& str);

    /**
     * @brief 将字符串转换为大写
     * @param str 要转换的字符串
     * @return 转换后的字符串
     */
    std::string to_upper(const std::string& str);

    /**
     * @brief 检查字符串是否以指定前缀开始
     * @param str 要检查的字符串
     * @param prefix 前缀
     * @return 如果以前缀开始返回 true
     */
    bool starts_with(const std::string& str, const std::string& prefix);

    /**
     * @brief 检查字符串是否以指定后缀结束
     * @param str 要检查的字符串
     * @param suffix 后缀
     * @return 如果以后缀结束返回 true
     */
    bool ends_with(const std::string& str, const std::string& suffix);

    /**
     * @brief 连接字符串向量
     * @param strings 字符串向量
     * @param separator 分隔符
     * @return 连接后的字符串
     */
    std::string join(const std::vector<std::string>& strings, const std::string& separator);

    /**
     * @brief 替换字符串中的所有匹配项
     * @param str 原字符串
     * @param from 要被替换的子字符串
     * @param to 替换的目标字符串
     * @return 替换后的字符串
     */
    std::string replace_all(const std::string& str, const std::string& from, const std::string& to);

    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return 如果文件存在返回 true
     */
    bool file_exists(const std::string& path);

    /**
     * @brief 检查路径是否为目录
     * @param path 路径
     * @return 如果是目录返回 true
     */
    bool is_directory(const std::string& path);

    /**
     * @brief 获取文件的绝对路径
     * @param path 相对或绝对路径
     * @return 绝对路径
     */
    std::string get_absolute_path(const std::string& path);

    /**
     * @brief 获取用户主目录
     * @return 用户主目录路径
     */
    std::string get_home_directory();

    /**
     * @brief 展开波浪号路径
     * @param path 可能包含 ~ 的路径
     * @return 展开后的路径
     */
    std::string expand_tilde(const std::string& path);

    /**
     * @brief 格式化字符串（类似 printf）
     * @tparam Args 参数类型
     * @param format 格式字符串
     * @param args 参数
     * @return 格式化后的字符串
     */
    template<typename... Args>
    std::string format(const std::string& format, Args... args);

    /**
     * @brief 获取当前时间戳字符串
     * @return 时间戳字符串
     */
    std::string get_timestamp();

    /**
     * @brief 安全地转换字符串到整数
     * @param str 字符串
     * @param default_value 默认值
     * @return 转换后的整数
     */
    int safe_stoi(const std::string& str, int default_value = 0);

} // namespace Utils

} // namespace NeXShell
