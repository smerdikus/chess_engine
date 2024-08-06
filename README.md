# Chess Engine

This is my first attempt to program a chess engine using bitboards and an easy implementation of the negamax AI algorithm.

## Table of Contents

- [Project Description](#project-description)
- [Prerequisites](#prerequisites)
- [Setup](#setup)
- [Usage](#usage)

## Project Description

This chess engine uses bitboards for efficient board representation and implements the negamax algorithm for AI decision-making. The project is written in C++ and is intended to provide a foundation for further development and experimentation in chess programming.

## Prerequisites

- C++ Compiler (e.g., GCC, Clang, MSVC)
- CMake
- Git
- SFML library

## Setup

### if you have SFML framework installed use steps bellow, otherwise install SFML using some of these installers:

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


1. Clone the repository:

   ```sh
   git clone https://github.com/yourusername/chess-engine.git
   cd chess-engine
   ```

2. Use the install script:

   ```sh
   ./install.sh
   ```

## Usage

To run the chess engine executable:

   ```sh
   ./chess_engine
   ```

This will start the chess engine and prompt you to enter moves.
