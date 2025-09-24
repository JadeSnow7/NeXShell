#include "command_executor.h"
#include "shell.h"
#include "builtin_commands.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <memory>

namespace NeXShell {

CommandExecutor::CommandExecutor(Shell* shell) : shell_(shell) {
}

int CommandExecutor::execute_pipeline(const Pipeline& pipeline) {
    if (pipeline.commands.empty()) {
        return 0;
    }
    
    if (pipeline.commands.size() == 1) {
        // 单个命令
        return execute_command(pipeline.commands[0]);
    }
    
    // 管道命令
    return create_pipeline(pipeline);
}

int CommandExecutor::execute_command(const Command& command) {
    if (command.program.empty()) {
        return 0;
    }
    
    // 检查是否为内建命令
    BuiltinCommands builtin(shell_);
    if (builtin.is_builtin(command.program)) {
        return builtin.execute(command);
    }
    
    // 执行外部程序
    int input_fd = -1;
    int output_fd = -1;
    
    if (!setup_redirections(command, input_fd, output_fd)) {
        return 1;
    }
    
    pid_t pid = execute_external_program(command, input_fd, output_fd);
    
    // 关闭文件描述符
    if (input_fd != -1) close(input_fd);
    if (output_fd != -1) close(output_fd);
    
    if (pid < 0) {
        return 1;
    }
    
    if (command.run_in_background) {
        background_processes_.push_back(pid);
        std::cout << "[" << background_processes_.size() << "] " << pid << std::endl;
        return 0;
    } else {
        return wait_for_process(pid);
    }
}

bool CommandExecutor::setup_redirections(const Command& command, int& input_fd, int& output_fd) {
    // 设置输入重定向
    if (command.input_file.has_value()) {
        input_fd = open(command.input_file->c_str(), O_RDONLY);
        if (input_fd < 0) {
            perror(("open " + *command.input_file).c_str());
            return false;
        }
    }
    
    // 设置输出重定向
    if (command.output_file.has_value()) {
        int flags = O_WRONLY | O_CREAT;
        if (command.append_output) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        output_fd = open(command.output_file->c_str(), flags, 0644);
        if (output_fd < 0) {
            perror(("open " + *command.output_file).c_str());
            if (input_fd != -1) close(input_fd);
            return false;
        }
    }
    
    return true;
}

pid_t CommandExecutor::execute_external_program(const Command& command, 
                                               int input_fd, 
                                               int output_fd) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        // 子进程
        
        // 设置输入重定向
        if (input_fd != -1) {
            if (dup2(input_fd, STDIN_FILENO) < 0) {
                perror("dup2 input");
                _exit(1);
            }
            close(input_fd);
        }
        
        // 设置输出重定向
        if (output_fd != -1) {
            if (dup2(output_fd, STDOUT_FILENO) < 0) {
                perror("dup2 output");
                _exit(1);
            }
            close(output_fd);
        }
        
        // 准备参数
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(command.program.c_str()));
        
        for (const auto& arg : command.arguments) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        // 执行程序
        execvp(command.program.c_str(), argv.data());
        
        // 如果执行到这里，说明 execvp 失败了
        perror(("execvp " + command.program).c_str());
        _exit(127);
    }
    
    return pid;
}

int CommandExecutor::create_pipeline(const Pipeline& pipeline) {
    std::vector<pid_t> pids;
    std::vector<int> pipe_fds;
    
    // 创建管道
    for (size_t i = 0; i < pipeline.commands.size() - 1; ++i) {
        int pipefd[2];
        if (pipe(pipefd) < 0) {
            perror("pipe");
            return 1;
        }
        pipe_fds.push_back(pipefd[0]); // 读端
        pipe_fds.push_back(pipefd[1]); // 写端
    }
    
    // 启动每个命令
    for (size_t i = 0; i < pipeline.commands.size(); ++i) {
        const Command& cmd = pipeline.commands[i];
        
        int input_fd = -1;
        int output_fd = -1;
        
        // 设置管道
        if (i > 0) {
            // 不是第一个命令，从前一个管道读取
            input_fd = pipe_fds[(i - 1) * 2];
        }
        
        if (i < pipeline.commands.size() - 1) {
            // 不是最后一个命令，写入到下一个管道
            output_fd = pipe_fds[i * 2 + 1];
        }
        
        // 检查是否为内建命令
        BuiltinCommands builtin(shell_);
        if (builtin.is_builtin(cmd.program)) {
            // 内建命令不能很好地处理管道，暂时跳过
            std::cerr << "Built-in commands in pipelines not fully supported" << std::endl;
            continue;
        }
        
        // 设置重定向（只对第一个和最后一个命令有效）
        if (i == 0 && cmd.input_file.has_value()) {
            input_fd = open(cmd.input_file->c_str(), O_RDONLY);
        }
        
        if (i == pipeline.commands.size() - 1 && cmd.output_file.has_value()) {
            int flags = O_WRONLY | O_CREAT;
            if (cmd.append_output) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            output_fd = open(cmd.output_file->c_str(), flags, 0644);
        }
        
        pid_t pid = execute_external_program(cmd, input_fd, output_fd);
        if (pid > 0) {
            pids.push_back(pid);
        }
    }
    
    // 关闭所有管道文件描述符
    for (int fd : pipe_fds) {
        close(fd);
    }
    
    // 等待所有进程完成
    int last_exit_code = 0;
    for (pid_t pid : pids) {
        int exit_code = wait_for_process(pid);
        last_exit_code = exit_code; // 使用最后一个命令的退出码
    }
    
    return last_exit_code;
}

int CommandExecutor::wait_for_process(pid_t pid) {
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    
    return 1;
}

void CommandExecutor::wait_for_background_processes() {
    for (pid_t pid : background_processes_) {
        waitpid(pid, nullptr, 0);
    }
    background_processes_.clear();
}

void CommandExecutor::cleanup_background_processes() {
    auto it = background_processes_.begin();
    while (it != background_processes_.end()) {
        int status;
        pid_t result = waitpid(*it, &status, WNOHANG);
        
        if (result > 0) {
            // 进程已完成
            std::cout << "[Done] Process " << *it << " finished" << std::endl;
            it = background_processes_.erase(it);
        } else if (result < 0) {
            // 错误或进程不存在
            it = background_processes_.erase(it);
        } else {
            // 进程仍在运行
            ++it;
        }
    }
}

} // namespace NeXShell
