# 编译需要的最低版本
cmake_minimum_required(VERSION 3.0)

# 给工程指定一个名字 不是可执行文件的名字
project(connPool)

# debug模式
set(CMAKE_BUILD_TYPE Debug)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置可执行附件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)


# 加载子目录
add_subdirectory(src)