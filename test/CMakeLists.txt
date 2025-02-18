cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(test VERSION 1.0)

add_executable(test
    test.cpp
)

target_link_libraries(test PRIVATE
    iocpclient
)