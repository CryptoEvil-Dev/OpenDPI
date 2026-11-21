add_executable(test_dpi_net_types
    test_socketaddr.cpp
)

add_executable(test_dpi_net_sockets
    test_sockets.cpp
)

add_executable(test_dpi_net_nonblocking
    test_nonblocking.cpp
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

target_link_libraries(test_dpi_net_nonblocking
    PRIVATE
        GTest::gtest_main
        netlib
)

gtest_discover_tests(test_dpi_net_types)
gtest_discover_tests(test_dpi_net_sockets)