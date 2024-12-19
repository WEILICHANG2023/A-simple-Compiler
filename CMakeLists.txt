cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(Compiler)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加源文件
set(SOURCE_FILES
    main.cpp
    lexer.cpp
    parser.cpp
    semantic_analyzer.cpp
    shared.cpp
    pcode_interpreter.cpp
)

# 添加头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}
)

# 添加可执行文件
add_executable(Compiler ${SOURCE_FILES})

# 设置输出文件名为 Compiler
set_target_properties(Compiler PROPERTIES OUTPUT_NAME "Compiler")

# 确保不提交 CMake 构建过程的临时文件
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)