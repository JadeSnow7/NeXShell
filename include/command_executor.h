#pragma once

#include "command_parser.h"
#include <sys/types.h>

namespace NeXShell {

class Shell;

/**
 * @brief 命令执行器类
 */
class CommandExecutor {
public:
    explicit CommandExecutor(Shell* shell);
    ~CommandExecutor() = default;

    /**
     * @brief 执行管道命令
     * @param pipeline 要执行的管道
     * @return 最后一个命令的退出码
     */
    int execute_pipeline(const Pipeline& pipeline);

    /**
     * @brief 执行单个命令
     * @param command 要执行的命令
     * @return 命令的退出码
     */
    int execute_command(const Command& command);

    /**
     * @brief 等待所有后台进程
     */
    void wait_for_background_processes();

    /**
     * @brief 清理已完成的后台进程
     */
    void cleanup_background_processes();

private:
    /**
     * @brief 检查是否为内建命令
     * @param command 命令对象
     * @return 如果是内建命令返回 true
     */
    bool is_builtin_command(const Command& command) const;

    /**
     * @brief 执行内建命令
     * @param command 命令对象
     * @return 命令的退出码
     */
    int execute_builtin_command(const Command& command);

    /**
     * @brief 执行外部程序
     * @param command 命令对象
     * @param input_fd 输入文件描述符
     * @param output_fd 输出文件描述符
     * @return 进程 ID
     */
    pid_t execute_external_program(const Command& command, 
                                  int input_fd = -1, 
                                  int output_fd = -1);

    /**
     * @brief 设置重定向
     * @param command 命令对象
     * @param input_fd 输入文件描述符引用
     * @param output_fd 输出文件描述符引用
     * @return 成功返回 true
     */
    bool setup_redirections(const Command& command, int& input_fd, int& output_fd);

    /**
     * @brief 创建管道
     * @param pipeline 管道对象
     * @return 最后一个命令的退出码
     */
    int create_pipeline(const Pipeline& pipeline);

    /**
     * @brief 等待进程完成
     * @param pid 进程 ID
     * @return 进程退出码
     */
    int wait_for_process(pid_t pid);

private:
    Shell* shell_;
    std::vector<pid_t> background_processes_;
};

} // namespace NeXShell
