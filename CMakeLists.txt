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

# Engine library
add_library(chess_engine
        src/engine/Board.cpp
        src/engine/Evaluation.cpp
        src/engine/Search.cpp
        src/engine/Zobrist.cpp
        src/engine/Fen.cpp
        include/engine/Board.h
        include/engine/CBitboardIterator.h
        include/engine/BitboardMovement.h
        include/engine/ChessMacros.h
        include/engine/Evaluation.h
        include/engine/Search.h
        include/engine/Zobrist.h
        include/engine/Fen.h)

target_include_directories(chess_engine PUBLIC include)

# App executable
add_executable(sfml_chess
        src/main.cpp
        src/ui/BoardRenderer.cpp
        include/ui/BoardRenderer.h
        include/ui/UiConstants.h)

target_link_libraries(sfml_chess PRIVATE chess_engine ${SFML_TARGETS})
target_include_directories(sfml_chess PRIVATE include)

# Tests
enable_testing()
add_executable(perft_tests tests/perft.cpp)
target_link_libraries(perft_tests PRIVATE chess_engine)
add_test(NAME perft COMMAND perft_tests)
