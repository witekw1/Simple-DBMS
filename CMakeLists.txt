cmake_minimum_required(VERSION 3.13)
project(SDBMS)

set(CMAKE_CXX_STANDARD 26)

add_subdirectory(libs/json)
add_subdirectory(libs/tlx)

add_executable(SDBMS
        main.cpp
        parser.cpp
        database.cpp
        where.cpp
        where.h
)


target_link_libraries(SDBMS PRIVATE
        nlohmann_json::nlohmann_json
        tlx
        )
target_compile_definitions(SDBMS PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS SDBMS DESTINATION bin)