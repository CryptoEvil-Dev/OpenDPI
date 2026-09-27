add_library(conflib STATIC 
    iconf.hpp
    iconf.cpp
)

target_include_directories(conflib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)