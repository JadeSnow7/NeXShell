cmake_minimum_required(VERSION 3.20)
project(NeXShell VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 设置调试模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 包含目录
include_directories(include)

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(nexsh ${SOURCES})

# 查找依赖库
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(CURL QUIET libcurl)
endif()

# 链接库
if(CURL_FOUND)
    target_link_libraries(nexsh PRIVATE pthread ${CURL_LIBRARIES})
    target_include_directories(nexsh PRIVATE ${CURL_INCLUDE_DIRS})
    target_compile_definitions(nexsh PRIVATE HAVE_CURL)
else()
    # 如果没有 curl，只链接 pthread
    target_link_libraries(nexsh PRIVATE pthread)
    message(WARNING "libcurl not found. AI features will be disabled.")
endif()

# 设置输出目录
set_target_properties(nexsh PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 启用测试
enable_testing()

# 添加测试目录
add_subdirectory(tests)
