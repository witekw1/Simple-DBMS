cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
project(SDBMS)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
        tlx
        GIT_REPOSITORY git@github.com:tlx/tlx.git
        GIT_TAG master
)

FetchContent_Declare(
        json
        GIT_REPOSITORY git@github.com:nlohmann/json.git
        GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(tlx json)

add_executable(SDBMS
        main.cpp
        parser.cpp
        parser.h
        database.cpp
        database.h
        where.cpp
        where.h
)


target_link_libraries(SDBMS PRIVATE
        nlohmann_json::nlohmann_json
        tlx
)
target_compile_definitions(SDBMS PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS SDBMS DESTINATION bin)