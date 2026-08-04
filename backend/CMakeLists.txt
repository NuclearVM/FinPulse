cmake_minimum_required(VERSION 3.23)

project(FinPulse)

add_executable(finpulse_backend)

target_sources(finpulse_backend
    PRIVATE
        src/main.cpp
)