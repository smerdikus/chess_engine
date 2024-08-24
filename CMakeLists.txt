cmake_minimum_required(VERSION 3.15)
project(sfml_chess)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find SFML with the necessary components
find_package(SFML REQUIRED COMPONENTS system window graphics network audio)

# Optionally include SFML headers (only if you need them for some reason)
include_directories(${SFML_INCLUDE_DIR})

# Add your executable
add_executable(sfml_chess main.cpp CBoard.cpp CBoard.h
        CBitboardIterator.h)

# Link SFML libraries to your executable
target_link_libraries(sfml_chess sfml-system sfml-window sfml-graphics sfml-network sfml-audio)
