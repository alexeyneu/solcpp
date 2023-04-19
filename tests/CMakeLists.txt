include_directories(${solcpp_SOURCE_DIR}/include)
include(${CMAKE_SOURCE_DIR}/doctest-config.cmake)
include(${CMAKE_SOURCE_DIR}/fmt-config.cmake)
include_directories(${doctest_INCLUDE_DIRS})
include_directories(${fmt_INCLUDE_DIRS})

# tests
add_executable(tests main.cpp)
target_link_libraries(tests ${doctest_LIBRARIES} ${fmt_LIBRARIES} sol)
target_compile_definitions(tests PUBLIC FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures")
add_test(NAME TestMangoGroup COMMAND tests)