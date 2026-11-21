add_executable(test_dpi_event_ctl
    test_evctl.cpp
)

target_link_libraries(test_dpi_event_ctl
    PRIVATE
        GTest::gtest_main
        eventlib
        netlib
)

gtest_discover_tests(test_dpi_event_ctl)