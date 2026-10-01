add_executable(test_dpi_conf_yaml
    test_yaml.cpp
)

target_link_libraries(test_dpi_conf_yaml
    PRIVATE
        GTest::gtest_main
        conflib
)

# Прокидываем путь к данным через define, чтобы тесты не зависели от CWD
target_compile_definitions(test_dpi_conf_yaml
    PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

gtest_discover_tests(test_dpi_conf_yaml)


# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/deep_nesting.yaml
#      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(TARGET test_dpi_conf_yaml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/deep_nesting.yaml"
        "$<TARGET_FILE_DIR:test_dpi_conf_yaml>/deep_nesting.yaml"
)