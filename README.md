# Chess Engine

This is my first attempt to program a chess engine using bitboards and a negamax-based AI. The codebase is split into an engine library (no SFML dependency) and a UI layer (SFML).

## Table of Contents

- [Project Description](#project-description)
- [Prerequisites](#prerequisites)
- [Setup](#setup)
- [Usage](#usage)

## Project Description

This chess engine uses bitboards for efficient board representation, alpha-beta search with move ordering, and a transposition table. The project is written in C++ and is intended to provide a foundation for further development and experimentation in chess programming.

## Repository Structure

```
include/
  engine/   # Core engine headers (Board, Evaluation, Zobrist, FEN, move helpers)
  ui/       # Rendering helpers and UI constants
src/
  engine/   # Core engine implementation
  ui/       # SFML rendering code
  main.cpp  # Application entrypoint
tests/
  perft.cpp # Perft + FEN parsing checks
```

## Prerequisites

- C++ Compiler (e.g., GCC, Clang, MSVC)
- CMake
- Git
- SFML library

## Setup

#### If you have SFML framework installed use steps bellow, otherwise install SFML using some of these installers:

- brew:

   ```sh
   brew install sfml
   ```

- apt:

   ```sh
   sudo apt-get update
   sudo apt-get install libsfml-dev
   ```

- Or you can use some other package managers or clone SFML from Source: https://github.com/SFML/SFML.git


#### When you have SFML installed you can follow these steps:

1. Clone the repository:

   ```sh
   git clone https://github.com/smerdikus/chess_engine.git
   cd chess_engine
   ```

2. Use the install script:

   ```sh
   ./install.sh
   ```

## Usage

To run the chess engine executable:

   ```sh
   ./run.sh
   ```

This will start the chess engine UI.

## Tests

Run the perft/FEN checks:

```sh
cmake --build build
ctest --test-dir build
```
