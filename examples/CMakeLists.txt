include(${CMAKE_SOURCE_DIR}/spdlog-config.cmake)
include(${CMAKE_SOURCE_DIR}/CURLConfig.cmake)

include_directories(${solcpp_SOURCE_DIR}/include)
include_directories(${CURL_INCLUDE_DIR})
include_directories(${spdlog_INCLUDE_DIRS})

# examples
add_executable(example-get-account-info getAccountInfo.cpp)
add_executable(example-send-transaction sendTransaction.cpp)
add_executable(example-place-order placeOrder.cpp)
add_executable(example-positions positions.cpp)

# link
target_link_libraries(example-get-account-info ${spdlog_LIBRARIES} ${CURL_LIBRARIES} sol)
target_link_libraries(example-send-transaction ${CURL_LIBRARIES} ${spdlog_LIBRARIES} sol)
target_link_libraries(example-place-order ${CURL_LIBRARIES} ${spdlog_LIBRARIES} sol)
target_link_libraries(example-positions ${spdlog_LIBRARIES} ${CURL_LIBRARIES} sol)

target_compile_definitions(example-place-order PUBLIC FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures")
