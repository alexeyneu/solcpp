include(${CMAKE_SOURCE_DIR}/libsodium-config.cmake)
include(${CMAKE_SOURCE_DIR}/nlohmann_json-config.cmake)
include(${CMAKE_SOURCE_DIR}/CURLConfig.cmake)

include_directories(${solcpp_SOURCE_DIR}/include)
include_directories(${libsodium_INCLUDE_DIRS})
include_directories(${nlohmann_json_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIR})

add_library(sol solana.cpp)
target_link_libraries(sol ${libsodium_LIBRARIES} ${nlohmann_json_LIBRARIES} ${CURL_LIBRARIES})
