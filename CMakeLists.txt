cmake_minimum_required(VERSION 3.16)

project(Calang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(calang
    src/main.cpp
    src/api.cpp
    src/core_routes.cpp
    src/dashboard.cpp
    src/login.cpp
    src/registration.cpp
    src/functions.cpp
    src/cookies.cpp
    src/event.cpp
    src/terminal.cpp
)

# target_include_directories(calang PRIVATE src)
target_include_directories(calang PRIVATE
    src
    external/crow
)

target_link_libraries(calang pthread curl)
