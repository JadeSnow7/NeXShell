cmake_minimum_required(VERSION 3.20)

# 查找测试框架（可选）
# find_package(GTest REQUIRED)

# 收集测试源文件
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# 如果没有 GTest，创建简单的测试
if(NOT GTest_FOUND)
    # 创建简单的测试可执行文件
    foreach(test_source ${TEST_SOURCES})
        get_filename_component(test_name ${test_source} NAME_WE)
        add_executable(${test_name} ${test_source})
        
        # 链接主程序的源文件（除了 main.cpp）
        file(GLOB_RECURSE MAIN_SOURCES "../src/*.cpp")
        list(REMOVE_ITEM MAIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp")
        target_sources(${test_name} PRIVATE ${MAIN_SOURCES})
        
        target_include_directories(${test_name} PRIVATE ../include)
        target_link_libraries(${test_name} PRIVATE pthread)
        
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()
