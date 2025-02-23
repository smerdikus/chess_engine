//
// Created by Petr Smerda on 23.02.2025.
//

#ifndef SFML_CHESS_EVALUATIONTABLES_H
#define SFML_CHESS_EVALUATIONTABLES_H

#include <cstdlib>

namespace chs{

  typedef uint64_t Bitboard;

  constexpr Bitboard RANK_1 = 0x00000000000000FF;
  constexpr Bitboard RANK_2 = 0x000000000000FF00;
  constexpr Bitboard RANK_3 = 0x0000000000FF0000;
  constexpr Bitboard RANK_4 = 0x00000000FF000000;
  constexpr Bitboard RANK_5 = 0x000000FF00000000;
  constexpr Bitboard RANK_6 = 0x0000FF0000000000;
  constexpr Bitboard RANK_7 = 0x00FF000000000000;
  constexpr Bitboard RANK_8 = 0xFF00000000000000;

  constexpr Bitboard FILE_A = 0x0101010101010101;
  constexpr Bitboard FILE_B = 0x0202020202020202;
  constexpr Bitboard FILE_C = 0x0404040404040404;
  constexpr Bitboard FILE_D = 0x0808080808080808;
  constexpr Bitboard FILE_E = 0x1010101010101010;
  constexpr Bitboard FILE_F = 0x2020202020202020;
  constexpr Bitboard FILE_G = 0x4040404040404040;
  constexpr Bitboard FILE_H = 0x8080808080808080;

  constexpr Bitboard NOT_FILE_A = ~0x0101010101010101;
  constexpr Bitboard NOT_FILE_B = ~0x0202020202020202;
  constexpr Bitboard NOT_FILE_C = ~0x0404040404040404;
  constexpr Bitboard NOT_FILE_D = ~0x0808080808080808;
  constexpr Bitboard NOT_FILE_E = ~0x1010101010101010;
  constexpr Bitboard NOT_FILE_F = ~0x2020202020202020;
  constexpr Bitboard NOT_FILE_G = ~0x4040404040404040;
  constexpr Bitboard NOT_FILE_H = ~0x8080808080808080;

  constexpr Bitboard A1 = 1ULL << 0;
  constexpr Bitboard B1 = 1ULL << 1;
  constexpr Bitboard C1 = 1ULL << 2;
  constexpr Bitboard D1 = 1ULL << 3;
  constexpr Bitboard E1 = 1ULL << 4;
  constexpr Bitboard F1 = 1ULL << 5;
  constexpr Bitboard G1 = 1ULL << 6;
  constexpr Bitboard H1 = 1ULL << 7;

  constexpr Bitboard A2 = 1ULL << 8;
  constexpr Bitboard B2 = 1ULL << 9;
  constexpr Bitboard C2 = 1ULL << 10;
  constexpr Bitboard D2 = 1ULL << 11;
  constexpr Bitboard E2 = 1ULL << 12;
  constexpr Bitboard F2 = 1ULL << 13;
  constexpr Bitboard G2 = 1ULL << 14;
  constexpr Bitboard H2 = 1ULL << 15;

  constexpr Bitboard A3 = 1ULL << 16;
  constexpr Bitboard B3 = 1ULL << 17;
  constexpr Bitboard C3 = 1ULL << 18;
  constexpr Bitboard D3 = 1ULL << 19;
  constexpr Bitboard E3 = 1ULL << 20;
  constexpr Bitboard F3 = 1ULL << 21;
  constexpr Bitboard G3 = 1ULL << 22;
  constexpr Bitboard H3 = 1ULL << 23;

  constexpr Bitboard A4 = 1ULL << 24;
  constexpr Bitboard B4 = 1ULL << 25;
  constexpr Bitboard C4 = 1ULL << 26;
  constexpr Bitboard D4 = 1ULL << 27;
  constexpr Bitboard E4 = 1ULL << 28;
  constexpr Bitboard F4 = 1ULL << 29;
  constexpr Bitboard G4 = 1ULL << 30;
  constexpr Bitboard H4 = 1ULL << 31;

  constexpr Bitboard A5 = 1ULL << 32;
  constexpr Bitboard B5 = 1ULL << 33;
  constexpr Bitboard C5 = 1ULL << 34;
  constexpr Bitboard D5 = 1ULL << 35;
  constexpr Bitboard E5 = 1ULL << 36;
  constexpr Bitboard F5 = 1ULL << 37;
  constexpr Bitboard G5 = 1ULL << 38;
  constexpr Bitboard H5 = 1ULL << 39;

  constexpr Bitboard A6 = 1ULL << 40;
  constexpr Bitboard B6 = 1ULL << 41;
  constexpr Bitboard C6 = 1ULL << 42;
  constexpr Bitboard D6 = 1ULL << 43;
  constexpr Bitboard E6 = 1ULL << 44;
  constexpr Bitboard F6 = 1ULL << 45;
  constexpr Bitboard G6 = 1ULL << 46;
  constexpr Bitboard H6 = 1ULL << 47;

  constexpr Bitboard A7 = 1ULL << 48;
  constexpr Bitboard B7 = 1ULL << 49;
  constexpr Bitboard C7 = 1ULL << 50;
  constexpr Bitboard D7 = 1ULL << 51;
  constexpr Bitboard E7 = 1ULL << 52;
  constexpr Bitboard F7 = 1ULL << 53;
  constexpr Bitboard G7 = 1ULL << 54;
  constexpr Bitboard H7 = 1ULL << 55;

  constexpr Bitboard A8 = 1ULL << 56;
  constexpr Bitboard B8 = 1ULL << 57;
  constexpr Bitboard C8 = 1ULL << 58;
  constexpr Bitboard D8 = 1ULL << 59;
  constexpr Bitboard E8 = 1ULL << 60;
  constexpr Bitboard F8 = 1ULL << 61;
  constexpr Bitboard G8 = 1ULL << 62;
  constexpr Bitboard H8 = 1ULL << 63;



  constexpr int PAWN_VALUE = 100;
  constexpr int KNIGHT_VALUE = 320;
  constexpr int BISHOP_VALUE = 330;
  constexpr int ROOK_VALUE = 500;
  constexpr int QUEEN_VALUE = 900;
  constexpr int KING_VALUE = 20000;

  constexpr int pawnTable[64] = {
          0, 0, 0, 0, 0, 0, 0, 0,
          5, 10, 10, -20, -20, 10, 10, 5,
          5, -5, -10, 0, 0, -10, -5, 5,
          0, 0, 0, 20, 20, 0, 0, 0,
          5, 5, 10, 25, 25, 10, 5, 5,
          10, 10, 20, 30, 30, 20, 10, 10,
          50, 50, 50, 50, 50, 50, 50, 50,
          0, 0, 0, 0, 0, 0, 0, 0
  };

  constexpr int knightTable[64] = {
          -50, -40, -30, -30, -30, -30, -40, -50,
          -40, -20, 0, 0, 0, 0, -20, -40,
          -30, 0, 10, 15, 15, 10, 0, -30,
          -30, 5, 15, 20, 20, 15, 5, -30,
          -30, 0, 15, 20, 20, 15, 0, -30,
          -30, 5, 10, 15, 15, 10, 5, -30,
          -40, -20, 0, 5, 5, 0, -20, -40,
          -50, -40, -30, -30, -30, -30, -40, -50
  };

  constexpr int bishopTable[64] = {
          -20, -10, -10, -10, -10, -10, -10, -20,
          -10, 0, 0, 0, 0, 0, 0, -10,
          -10, 0, 5, 10, 10, 5, 0, -10,
          -10, 5, 5, 10, 10, 5, 5, -10,
          -10, 0, 10, 10, 10, 10, 0, -10,
          -10, 10, 10, 10, 10, 10, 10, -10,
          -10, 5, 0, 0, 0, 0, 5, -10,
          -20, -10, -10, -10, -10, -10, -10, -20
  };

  constexpr int rookTable[64] = {
          0, 0, 0, 0, 0, 0, 0, 0,
          5, 10, 10, 10, 10, 10, 10, 5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          0, 0, 0, 5, 5, 0, 0, 0
  };

  constexpr int queenTable[64] = {
          -20, -10, -10, -5, -5, -10, -10, -20,
          -10, 0, 0, 0, 0, 0, 0, -10,
          -10, 0, 5, 5, 5, 5, 0, -10,
          -5, 0, 5, 5, 5, 5, 0, -5,
          0, 0, 5, 5, 5, 5, 0, -5,
          -10, 5, 5, 5, 5, 5, 0, -10,
          -10, 0, 5, 0, 0, 0, 0, -10,
          -20, -10, -10, -5, -5, -10, -10, -20
  };

  constexpr int kingTable[64] = {
          20, 30, 10, 0, 0, 10, 30, 20,
          20, 20, 0, 0, 0, 0, 20, 20,
          -10, -20, -20, -20, -20, -20, -20, -10,
          -20, -30, -30, -40, -40, -30, -30, -20,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30
  };

  constexpr int kingEndgameTable[64] = {
          -50, -40, -30, -20, -20, -30, -40, -50,
          -30, -20, -10, 0, 0, -10, -20, -30,
          -30, -10, 20, 30, 30, 20, -10, -30,
          -30, -10, 30, 40, 40, 30, -10, -30,
          -30, -10, 30, 40, 40, 30, -10, -30,
          -30, -10, 20, 30, 30, 20, -10, -30,
          -30, -30, 0, 0, 0, 0, -30, -30,
          -50, -30, -30, -30, -30, -30, -30, -50
  };
}

#endif //SFML_CHESS_EVALUATIONTABLES_H
