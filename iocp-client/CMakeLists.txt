cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(iocpclient VERSION 1.0)

include_directories(src)

add_library(iocpclient STATIC
    src/iocpworker.h src/iocpworker.cpp
    "src/iocpcontext.h" "src/iocpclient.h" "src/iocpclient.cpp"
)

target_include_directories(iocpclient PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(iocpclient PRIVATE WIN32_LEAN_AND_MEAN)

target_compile_definitions(iocpclient INTERFACE WIN32_LEAN_AND_MEAN)