cmake_minimum_required(VERSION 3.10)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2 # Используем стабильный релиз вместо main
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_dpi_net_types
    net/test_socketaddr.cpp
    net/test_sockets.cpp
)

add_executable(test_dpi_net_sockets
    net/test_sockets.cpp
)


target_link_libraries(test_dpi_net_types 
    PRIVATE 
        GTest::gtest_main 
        netlib
)

target_link_libraries(test_dpi_net_sockets 
    PRIVATE 
        GTest::gtest_main 
        netlib
)

include(GoogleTest)
gtest_discover_tests(test_dpi_net_types)
gtest_discover_tests(test_dpi_net_sockets)