cmake_minimum_required(VERSION 3.1)

add_library(nlohmann_json INTERFACE)
add_library(nlohmann::json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")