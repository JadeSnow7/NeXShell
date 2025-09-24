#include "command_parser.h"
#include "utils.h"
#include <sstream>
#include <algorithm>
#include <cstdlib>

namespace NeXShell {

Pipeline CommandParser::parse(const std::string& input) {
    Pipeline pipeline;
    
    if (is_empty(input)) {
        return pipeline;
    }
    
    // 简单实现：先按 | 分割，然后解析每个命令
    std::vector<std::string> pipe_parts = Utils::split(input, '|');
    
    for (size_t i = 0; i < pipe_parts.size(); ++i) {
        std::string cmd_str = trim(pipe_parts[i]);
        if (!cmd_str.empty()) {
            std::vector<std::string> tokens = tokenize(cmd_str);
            if (!tokens.empty()) {
                Command cmd = parse_command(tokens, 0, tokens.size());
                pipeline.commands.push_back(cmd);
            }
        }
    }
    
    // 检查是否有后台执行标志
    if (!pipeline.commands.empty()) {
        const auto& last_cmd = pipeline.commands.back();
        if (last_cmd.run_in_background) {
            pipeline.run_in_background = true;
            // 移除最后一个命令的后台标志
            pipeline.commands.back().run_in_background = false;
        }
    }
    
    return pipeline;
}

bool CommandParser::is_empty(const std::string& input) {
    return trim(input).empty();
}

std::string CommandParser::trim(const std::string& str) {
    return Utils::trim(str);
}

std::vector<std::string> CommandParser::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current_token;
    bool in_quotes = false;
    char quote_char = '\0';
    
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        
        if (!in_quotes) {
            if (c == '"' || c == '\'') {
                in_quotes = true;
                quote_char = c;
            } else if (std::isspace(c)) {
                if (!current_token.empty()) {
                    tokens.push_back(expand_variables(current_token));
                    current_token.clear();
                }
            } else if (is_special_char(c)) {
                if (!current_token.empty()) {
                    tokens.push_back(expand_variables(current_token));
                    current_token.clear();
                }
                tokens.push_back(std::string(1, c));
            } else {
                current_token += c;
            }
        } else {
            if (c == quote_char) {
                in_quotes = false;
                quote_char = '\0';
            } else {
                current_token += c;
            }
        }
    }
    
    if (!current_token.empty()) {
        tokens.push_back(expand_variables(current_token));
    }
    
    return tokens;
}

Command CommandParser::parse_command(const std::vector<std::string>& tokens, 
                                    size_t start, size_t end) {
    Command cmd;
    
    if (start >= end || start >= tokens.size()) {
        return cmd;
    }
    
    cmd.program = tokens[start];
    
    for (size_t i = start + 1; i < end && i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        
        if (token == ">") {
            if (i + 1 < end && i + 1 < tokens.size()) {
                cmd.output_file = tokens[i + 1];
                cmd.append_output = false;
                ++i; // 跳过文件名
            }
        } else if (token == ">>") {
            if (i + 1 < end && i + 1 < tokens.size()) {
                cmd.output_file = tokens[i + 1];
                cmd.append_output = true;
                ++i; // 跳过文件名
            }
        } else if (token == "<") {
            if (i + 1 < end && i + 1 < tokens.size()) {
                cmd.input_file = tokens[i + 1];
                ++i; // 跳过文件名
            }
        } else if (token == "&") {
            cmd.run_in_background = true;
        } else {
            cmd.arguments.push_back(token);
        }
    }
    
    return cmd;
}

std::string CommandParser::expand_variables(const std::string& token) {
    std::string result = token;
    size_t pos = 0;
    
    while ((pos = result.find('$', pos)) != std::string::npos) {
        size_t start = pos + 1;
        size_t end = start;
        
        // 找到变量名的结束位置
        while (end < result.length() && 
               (std::isalnum(result[end]) || result[end] == '_')) {
            ++end;
        }
        
        if (end > start) {
            std::string var_name = result.substr(start, end - start);
            const char* var_value = getenv(var_name.c_str());
            std::string replacement = var_value ? var_value : "";
            
            result.replace(pos, end - pos, replacement);
            pos += replacement.length();
        } else {
            ++pos;
        }
    }
    
    return result;
}

std::string CommandParser::handle_quotes(const std::string& token) {
    if (token.length() < 2) {
        return token;
    }
    
    char first = token[0];
    char last = token[token.length() - 1];
    
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
        return token.substr(1, token.length() - 2);
    }
    
    return token;
}

bool CommandParser::is_special_char(char c) const {
    return c == '>' || c == '<' || c == '|' || c == '&';
}

} // namespace NeXShell
