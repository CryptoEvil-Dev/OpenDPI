add_library(conflib STATIC 
    iconf.hpp
    yaml.hpp
    iconf.cpp
    yaml.cpp
)

include(FetchContent)
FetchContent_Declare(
    ryml
    GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
    GIT_TAG v0.16.0
    GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(ryml)

target_include_directories(conflib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(conflib PRIVATE ryml::ryml)