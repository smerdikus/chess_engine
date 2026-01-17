//
// Created by Petr Smerda on 24.02.2025.
//
#include "engine/Evaluation.h"
#include "engine/Board.h"

#include <bit>

namespace chs {
  bool isEndgame(BoardState brd, int threshold) {
    // Return the count of not empty squares
    return (popcount(~brd.empty()) <= threshold);
  }


  inline int materialEvaluation(BoardState brd) {
    int materialScore = 0;

    materialScore += popcount(brd.wPawns) * PAWN_VALUE;
    materialScore += popcount(brd.wKnights) * KNIGHT_VALUE;
    materialScore += popcount(brd.wBishops) * BISHOP_VALUE;
    materialScore += popcount(brd.wRooks) * ROOK_VALUE;
    materialScore += popcount(brd.wQueens) * QUEEN_VALUE;

    materialScore -= popcount(brd.bPawns) * PAWN_VALUE;
    materialScore -= popcount(brd.bKnights) * KNIGHT_VALUE;
    materialScore -= popcount(brd.bBishops) * BISHOP_VALUE;
    materialScore -= popcount(brd.bRooks) * ROOK_VALUE;
    materialScore -= popcount(brd.bQueens) * QUEEN_VALUE;

    return materialScore;
  }

  int positionalEvaluation(BoardState brd) {
    int positionalScore = 0;

    auto applyPositionalScore = [&positionalScore](const uint64_t pieces, const int *table, int adjustment) {
      int mirrorOffset = (adjustment > 0) ? 0 : 63;
      for (uint64_t bb = pieces; bb; bb &= (bb - 1)) {
        int i = std::countr_zero(bb);
        positionalScore += adjustment * table[mirrorOffset ? mirrorOffset - i : i];
      }
    };

    const uint64_t whitePieces[6] = {brd.wPawns, brd.wKnights, brd.wBishops, brd.wRooks, brd.wQueens, brd.wKing};
    const uint64_t blackPieces[6] = {brd.bPawns, brd.bKnights, brd.bBishops, brd.bRooks, brd.bQueens, brd.bKing};
    const int *tables[6] = {pawnTable, knightTable, bishopTable, rookTable, queenTable, kingTable};

    for (int i = 0; i < 6; i++) applyPositionalScore(whitePieces[i], tables[i], 1);
    for (int i = 0; i < 6; i++) applyPositionalScore(blackPieces[i], tables[i], -1);


    return positionalScore;
  }


  int kingSafetyEvaluation(BoardState brd) {
    int kingSafetyScore = 0;

    // Castling bonus
    kingSafetyScore += 20 * popcount(brd.wCastling);
    kingSafetyScore -= 20 * popcount(brd.bCastling);

    // Exposed king penalty
    kingSafetyScore -= 10 * popcount(oneAround(brd.wKing) & brd.empty());
    kingSafetyScore += 10 * popcount(oneAround(brd.bKing) & brd.empty());

    kingSafetyScore -= Board::wKingSafe(brd, brd.wKing) ? 0 : 200;
    kingSafetyScore += Board::bKingSafe(brd, brd.bKing) ? 0 : 200;

    return kingSafetyScore;
  }


  int pawnStructureEvaluation(BoardState brd) {
    int pawnStructureScore = 0;

    // === ISOLATED PAWNS ===
    Bitboard wAdjacent = ((brd.wPawns >> 1) & 0x7F7F7F7F7F7F7F7F) | ((brd.wPawns << 1) & 0xFEFEFEFEFEFEFEFE);
    Bitboard wIsolated = brd.wPawns & ~wAdjacent;
    pawnStructureScore -= popcount(wIsolated) * 10;

    Bitboard bAdjacent = ((brd.bPawns >> 1) & 0x7F7F7F7F7F7F7F7F) | ((brd.bPawns << 1) & 0xFEFEFEFEFEFEFEFE);
    Bitboard bIsolated = brd.bPawns & ~bAdjacent;
    pawnStructureScore += popcount(bIsolated) * 10;


    // === DOUBLED PAWNS ===
    Bitboard wDoubled = brd.wPawns & (brd.wPawns >> 8);
    pawnStructureScore -= popcount(wDoubled) * 15;

    Bitboard bDoubled = brd.bPawns & (brd.bPawns >> 8);
    pawnStructureScore += popcount(bDoubled) * 15;


    // === SUPPORTED PAWNS ===
    Bitboard wSupported =
            brd.wPawns & (((brd.wPawns >> 9) & 0x7F7F7F7F7F7F7F7F) | ((brd.wPawns >> 7) & 0xFEFEFEFEFEFEFEFE));
    Bitboard bSupported =
            brd.bPawns & (((brd.bPawns << 9) & 0xFEFEFEFEFEFEFEFE) | ((brd.bPawns << 7) & 0x7F7F7F7F7F7F7F7F));

    pawnStructureScore += popcount(wSupported) * 10; // Reward for connected pawns
    pawnStructureScore -= popcount(bSupported) * 10;

    // === PAWN CHAINS ===
    Bitboard wPawnChains =
            wSupported & (((brd.wPawns << 9) & 0xFEFEFEFEFEFEFEFE) | ((brd.wPawns << 7) & 0x7F7F7F7F7F7F7F7F));
    Bitboard bPawnChains =
            bSupported & (((brd.bPawns >> 9) & 0x7F7F7F7F7F7F7F7F) | ((brd.bPawns >> 7) & 0xFEFEFEFEFEFEFEFE));

    pawnStructureScore += popcount(wPawnChains) * 15; // Extra reward for chains
    pawnStructureScore -= popcount(bPawnChains) * 15;

    // === BACKWARD PAWNS ===
    Bitboard wBackward = (brd.wPawns & ~wSupported) & (brd.bPawns >> 8);
    Bitboard bBackward = (brd.bPawns & ~bSupported) & (brd.wPawns << 8);

    pawnStructureScore -= popcount(wBackward) * 15; // Penalty for weak pawns
    pawnStructureScore += popcount(bBackward) * 15;

    return pawnStructureScore;
  }


  int evaluate(BoardState brd) {
    int score = 0;

    // Material evaluation (typically most important)
    score += materialEvaluation(brd);

    // Positional evaluation (piece-square tables)
    score += positionalEvaluation(brd);

    // King safety
    score += kingSafetyEvaluation(brd);

    // Pawn structure
    score += pawnStructureEvaluation(brd);


    // Return score relative to whose turn it is
    return brd.whiteToMove() ? score : -score;
  }

}
