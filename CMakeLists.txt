cmake_minimum_required(VERSION 3.15)
project(sfml_chess)

set(CMAKE_CXX_STANDARD 20)

# Find SFML
set(SFML_ROOT "/usr/local/Cellar/sfml/2.5.1_2")  # Adjust this path to where SFML is installed
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS system window graphics network audio REQUIRED)

add_executable(sfml_chess main.cpp CBoard.cpp CBoard.h)

target_link_libraries(sfml_chess sfml-system sfml-window sfml-graphics sfml-network sfml-audio)
