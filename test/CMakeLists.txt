set(TEST_FILES "")
aux_source_directory(. TEST_FILES)

foreach(TEST_FILE_PATH ${TEST_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TEST_FILE_NAME ${TEST_FILE_PATH})
    message(STATUS "Generating test target: ${TEST_FILE_NAME}")

    add_executable(${TEST_FILE_NAME} ${TEST_FILE_PATH})
    target_link_libraries(${TEST_FILE_NAME} PUBLIC libcoroutine)
    set_property(TARGET ${TEST_FILE_NAME}  PROPERTY CXX_STANDARD 20)
endforeach(TEST_FILE_PATH)