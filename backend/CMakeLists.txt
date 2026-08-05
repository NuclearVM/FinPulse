cmake_minimum_required(VERSION 3.23)

project(FinPulse)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

add_executable(finpulse_backend)

target_sources(finpulse_backend
    PRIVATE
        src/main.cpp
)

target_link_libraries(finpulse_backend
    PRIVATE
        Boost::boost
)