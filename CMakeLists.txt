cmake_minimum_required(VERSION 3.10)
project(dpicpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main
    main.cpp
    src/sock.cpp
    src/tcp.cpp
)

include_directories(src)