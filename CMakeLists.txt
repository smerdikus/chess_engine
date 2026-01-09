cmake_minimum_required(VERSION 3.15)
project(sfml_chess)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find SFML with the necessary components
find_package(SFML 3 QUIET COMPONENTS Graphics Window System)
if (SFML_FOUND)
  set(SFML_TARGETS SFML::Graphics SFML::Window SFML::System)
else()
  find_package(SFML 2 REQUIRED COMPONENTS graphics window system)
  set(SFML_TARGETS sfml-graphics sfml-window sfml-system)
endif()

# Add your executable
add_executable(sfml_chess
        main.cpp
        CBoard.cpp
        Evaluation.cpp
        CBoard.h
        CBitboardIterator.h
        BitboardMovement.h
        Evaluation.h)

# Link SFML libraries to your executable
target_link_libraries(sfml_chess PRIVATE ${SFML_TARGETS})
