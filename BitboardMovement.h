//
// Created by Petr Smerda on 22.02.2025.
//

#ifndef SFML_CHESS_BITBOARDMOVEMENT_H
#define SFML_CHESS_BITBOARDMOVEMENT_H

#include <iostream>
#include <bitset>
#include <stack>
#include <cstdint>
#include "ChessMacros.h"
#include "CBitboardIterator.h"


namespace chs {


  inline Bitboard nortOne(Bitboard pos) { return pos << 8; }

  inline Bitboard soutOne(Bitboard pos) { return pos >> 8; }

  inline Bitboard westOne(Bitboard pos) { return (pos & NOT_FILE_H) << 1; }

  inline Bitboard eastOne(Bitboard pos) { return (pos & NOT_FILE_A) >> 1; }


  inline Bitboard nortTwo(Bitboard pos) { return pos << 16; }

  inline Bitboard soutTwo(Bitboard pos) { return pos >> 16; }

  inline Bitboard westTwo(Bitboard pos) { return (pos & NOT_FILE_H & NOT_FILE_G) << 2; }

  inline Bitboard eastTwo(Bitboard pos) { return (pos & NOT_FILE_B & NOT_FILE_A) >> 2; }


  inline Bitboard noWe(Bitboard pos) { return nortOne(westOne(pos)); }

  inline Bitboard noEa(Bitboard pos) { return nortOne(eastOne(pos)); }

  inline Bitboard soWe(Bitboard pos) { return soutOne(westOne(pos)); }

  inline Bitboard soEa(Bitboard pos) { return soutOne(eastOne(pos)); }


  // ---------------- PAWNS ----------------


  inline Bitboard wSinglePush(Bitboard pawns, Bitboard empty) { return nortOne(pawns) & empty; }

  inline Bitboard bSinglePush(Bitboard pawns, Bitboard empty) { return soutOne(pawns) & empty; }

  inline Bitboard wDoublePush(Bitboard pawns, Bitboard empty) { return nortOne(wSinglePush(pawns, empty)) & empty & RANK_4; }

  inline Bitboard bDoublePush(Bitboard pawns, Bitboard empty) { return soutOne(bSinglePush(pawns, empty)) & empty & RANK_5; }


  inline Bitboard wPawnWestAttacks(Bitboard pawns) { return noWe(pawns); }

  inline Bitboard wPawnEastAttacks(Bitboard pawns) { return noEa(pawns); }

  inline Bitboard bPawnWestAttacks(Bitboard pawns) { return soWe(pawns); }

  inline Bitboard bPawnEastAttacks(Bitboard pawns) { return soEa(pawns); }

  inline Bitboard wPawnMoves(Bitboard pos, Bitboard black, Bitboard empty, Bitboard enPassant) {
    return wSinglePush(pos, empty) | wDoublePush(pos, empty) |
           ((wPawnWestAttacks(pos) | wPawnEastAttacks(pos)) & (black | enPassant));
  }

  inline Bitboard bPawnMoves(Bitboard pos, Bitboard white, Bitboard empty, Bitboard enPassant) {
    return bSinglePush(pos, empty) | bDoublePush(pos, empty) |
           ((bPawnWestAttacks(pos) | bPawnEastAttacks(pos)) & (white | enPassant));
  }


  // ---------------- KNIGHTS ----------------


  inline Bitboard noNoEa(Bitboard pos) { return (pos & NOT_FILE_H) << 17; }

  inline Bitboard noEaEa(Bitboard pos) { return (pos & (NOT_FILE_H & NOT_FILE_G)) << 10; }

  inline Bitboard soEaEa(Bitboard pos) { return (pos & (NOT_FILE_H & NOT_FILE_G)) >> 6; }

  inline Bitboard soSoEa(Bitboard pos) { return (pos & NOT_FILE_H) >> 15; }

  inline Bitboard soSoWe(Bitboard pos) { return (pos & NOT_FILE_A) >> 17; }

  inline Bitboard soWeWe(Bitboard pos) { return (pos & (NOT_FILE_A & NOT_FILE_B)) >> 10; }

  inline Bitboard noWeWe(Bitboard pos) { return (pos & (NOT_FILE_A & NOT_FILE_B)) << 6; }

  inline Bitboard noNoWe(Bitboard pos) { return (pos & NOT_FILE_A) << 15; }

  inline Bitboard wKnightMoves(Bitboard pos, Bitboard enemyOrEmpty) {
    return (noNoEa(pos) & enemyOrEmpty) | (noEaEa(pos) & enemyOrEmpty) |
           (soEaEa(pos) & enemyOrEmpty) | (soSoEa(pos) & enemyOrEmpty) |
           (soSoWe(pos) & enemyOrEmpty) | (soWeWe(pos) & enemyOrEmpty) |
           (noWeWe(pos) & enemyOrEmpty) | (noNoWe(pos) & enemyOrEmpty);
  }

  inline Bitboard bKnightMoves(Bitboard pos, Bitboard enemyOrEmpty) {
    return (noNoEa(pos) & enemyOrEmpty) | (noEaEa(pos) & enemyOrEmpty) |
           (soEaEa(pos) & enemyOrEmpty) | (soSoEa(pos) & enemyOrEmpty) |
           (soSoWe(pos) & enemyOrEmpty) | (soWeWe(pos) & enemyOrEmpty) |
           (noWeWe(pos) & enemyOrEmpty) | (noNoWe(pos) & enemyOrEmpty);
  }


  // ---------------- BISHOPS ----------------


  inline Bitboard sliderMoves(Bitboard pos, Bitboard (*directionFunc)(Bitboard), Bitboard enemies, Bitboard empty) {
    Bitboard res = 0;
    Bitboard currentPos = directionFunc(pos);

    while (currentPos & (enemies | empty)) {
      res |= currentPos;
      if (currentPos & enemies)
        break;

      currentPos = directionFunc(currentPos);
    }

    return res;
  }


  inline Bitboard bishopMoves(Bitboard pos, Bitboard enemies, Bitboard empty) {
    Bitboard res = 0;

    res |= sliderMoves(pos, noWe, enemies, empty);
    res |= sliderMoves(pos, noEa, enemies, empty);
    res |= sliderMoves(pos, soWe, enemies, empty);
    res |= sliderMoves(pos, soEa, enemies, empty);

    return res & ~pos;  // Clear the start position
  }

// Wrapper functions for white and black bishops
  inline Bitboard wBishopMoves(Bitboard pos, Bitboard black, Bitboard empty) {
    return bishopMoves(pos, black, empty);
  }

  inline Bitboard bBishopMoves(Bitboard pos, Bitboard white, Bitboard empty) {
    return bishopMoves(pos, white, empty);
  }


  // ---------------- ROOK ----------------


  inline Bitboard rookMoves(Bitboard pos, Bitboard enemies, Bitboard empty) {
    Bitboard res = 0;


    // Apply the lambda for each straight direction
    res |= sliderMoves(pos, nortOne, enemies, empty);
    res |= sliderMoves(pos, eastOne, enemies, empty);
    res |= sliderMoves(pos, soutOne, enemies, empty);
    res |= sliderMoves(pos, westOne, enemies, empty);

    return res & ~pos;  // Clear the start position
  }

  // Wrapper functions for white and black rooks
  inline Bitboard wRookMoves(Bitboard pos, Bitboard black, Bitboard empty) {
    return rookMoves(pos, black, empty);
  }

  inline Bitboard bRookMoves(Bitboard pos, Bitboard white, Bitboard empty) {
    return rookMoves(pos, white, empty);
  }

  // ---------------- QUEEN ----------------

  inline Bitboard wQueenMoves(Bitboard pos, Bitboard black, Bitboard empty) {
    return wBishopMoves(pos, black, empty) | wRookMoves(pos, black, empty);
  }

  inline Bitboard bQueenMoves(Bitboard pos, Bitboard white, Bitboard empty) {
    return bBishopMoves(pos, white, empty) | bRookMoves(pos, white, empty);
  }


  inline Bitboard oneAround(Bitboard pos) {
    return nortOne(pos) | soutOne(pos) | eastOne(pos) | westOne(pos) | noWe(pos) | noEa(pos) | soWe(pos) | soEa(pos);
  }

}

#endif //SFML_CHESS_BITBOARDMOVEMENT_H
