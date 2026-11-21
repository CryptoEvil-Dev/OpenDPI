add_library(eventlib STATIC 
    events.hpp
    events.cpp
)

target_include_directories(eventlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)