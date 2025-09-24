#include "shell.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <exception>

/**
 * @brief 程序入口点
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出码
 */
int main(int argc, char* argv[]) {
    try {
        // 创建 Shell 实例
        NeXShell::Shell shell;
        
        // 如果有命令行参数，执行指定的命令
        if (argc > 1) {
            std::string command;
            for (int i = 1; i < argc; ++i) {
                if (i > 1) command += " ";
                command += argv[i];
            }
            return shell.execute_command(command);
        }
        
        // 否则启动交互式 Shell
        std::cout << "Welcome to NeXShell - A modern C++20 AI-Enhanced Linux Shell\n";
        std::cout << "Type 'help' for available commands, 'ai <query>' for AI assistance, or 'exit' to quit.\n\n";
        
        shell.run();
        
        std::cout << "\nGoodbye!\n";
        return EXIT_SUCCESS;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
}
