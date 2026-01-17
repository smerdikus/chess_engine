//
// Created by Petr Smerda on 24.02.2025.
//

#ifndef SFML_CHESS_EVALUATION_H
#define SFML_CHESS_EVALUATION_H

#include <iostream>
#include "engine/ChessMacros.h"
#include "engine/BitboardMovement.h"

namespace chs {
  inline int popcount(Bitboard num) {
    return __builtin_popcountll(num);
  }

  bool isEndgame(BoardState brd, int threshold = 12);

  int materialEvaluation(BoardState brd);

  int positionalEvaluation(BoardState brd);

  int kingSafetyEvaluation(BoardState brd);

  int pawnStructureEvaluation(BoardState brd);

  int evaluate(BoardState brd);
}

#endif //SFML_CHESS_EVALUATION_H
