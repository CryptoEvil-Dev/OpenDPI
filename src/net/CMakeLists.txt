add_library(netlib STATIC 
    addrs.hpp
    addrs.cpp
    tcp.hpp
    tcp.cpp
)

target_include_directories(netlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)