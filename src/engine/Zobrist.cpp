//
// Zobrist hashing utilities.
//

#include "engine/Zobrist.h"

#include <random>

namespace chs {

  const Zobrist &Zobrist::instance() {
    static Zobrist instance;
    return instance;
  }

  Zobrist::Zobrist() {
    std::mt19937_64 rng(0xC0D3'5EED'BEEF'1234ULL);

    for (auto &pieceArray: m_pieceKeys) for (auto &key: pieceArray) key = rng();

    for (auto &key: m_castlingKeys) key = rng();

    for (auto &key: m_enPassantKeys) key = rng();

    m_sideKey = rng();
  }

  int Zobrist::castlingMask(const BoardState &brd) {
    int mask = 0;
    mask |= static_cast<int>((brd.wCastling & G1) != 0);
    mask |= static_cast<int>((brd.wCastling & C1) != 0) << 1;
    mask |= static_cast<int>((brd.bCastling & G8) != 0) << 2;
    mask |= static_cast<int>((brd.bCastling & C8) != 0) << 3;
    return mask;
  }

  uint64_t Zobrist::hashBoard(const BoardState &brd) const {
    uint64_t hash = 0;

    const Bitboard pieceBoards[12] = {
            brd.wPawns, brd.wKnights, brd.wBishops, brd.wRooks, brd.wQueens, brd.wKing,
            brd.bPawns, brd.bKnights, brd.bBishops, brd.bRooks, brd.bQueens, brd.bKing
    };

    for (int pieceIndex = 0; pieceIndex < 12; ++pieceIndex) {
      Bitboard bb = pieceBoards[pieceIndex];
      while (bb) {
        int square = __builtin_ctzll(bb);

        hash ^= m_pieceKeys[pieceIndex][square];
        bb &= bb - 1;
      }
    }

    hash ^= m_castlingKeys[castlingMask(brd)];

    if (brd.enPassant) {
      int square = __builtin_ctzll(brd.enPassant);
      hash ^= m_enPassantKeys[square];
    }

    if (brd.blackToMove()) hash ^= m_sideKey;

    return hash;
  }

} // namespace chs
