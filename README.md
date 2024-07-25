# Chess Engine

This is my first attempt to program a chess engine using bitboards and an easy implementation of the negamax AI algorithm.

## Table of Contents

- [Project Description](#project-description)
- [Prerequisites](#prerequisites)
- [Setup](#setup)
- [Usage](#usage)
- [Resolving Git Conflicts](#resolving-git-conflicts)
- [Ignoring Folders with `.gitignore`](#ignoring-folders-with-gitignore)
- [Contributing](#contributing)
- [License](#license)

## Project Description

This chess engine uses bitboards for efficient board representation and implements the negamax algorithm for AI decision-making. The project is written in C++ and is intended to provide a foundation for further development and experimentation in chess programming.

## Prerequisites

- C++ Compiler (e.g., GCC, Clang, MSVC)
- CMake
- Git

## Setup

1. Clone the repository:

   ```sh
   git clone https://github.com/yourusername/chess-engine.git
   cd chess-engine
   ```

2. Create a build directory and navigate into it:

   ```sh
   mkdir build
   cd build
   ```

3. Generate the build files using CMake:

   ```sh
   cmake ..
   ```

4. Build the project:

   ```sh
   make
   ```

## Usage

To run the chess engine executable:

   ```sh
   ./chess_engine
   ```

This will start the chess engine and prompt you to enter moves.
