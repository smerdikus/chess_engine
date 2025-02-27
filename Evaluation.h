//
// Created by Petr Smerda on 24.02.2025.
//

#ifndef SFML_CHESS_EVALUATION_H
#define SFML_CHESS_EVALUATION_H

#include <iostream>
#include "CBoard.h"
#include "ChessMacros.h"
#include "BitboardMovement.h"

namespace chs {
  int popcount(Bitboard num);

  bool isEndgame(Board brd, int threshold = 12);

  int materialEvaluation(Board brd);

  int positionalEvaluation(Board brd);

  int kingSafetyEvaluation(Board brd);

  int pawnStructureEvaluation(Board brd);

  int evaluate(Board brd);
}

#endif //SFML_CHESS_EVALUATION_H
