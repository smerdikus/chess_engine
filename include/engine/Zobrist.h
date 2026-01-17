#ifndef SFML_CHESS_ZOBRIST_H
#define SFML_CHESS_ZOBRIST_H

#include <array>
#include <cstdint>
#include "engine/ChessMacros.h"

namespace chs {

  class Zobrist {
  public:
    static const Zobrist &instance();

    uint64_t hashBoard(const BoardState &brd) const;

  private:
    Zobrist();

    std::array<std::array<uint64_t, 64>, 12> m_pieceKeys{};
    std::array<uint64_t, 16> m_castlingKeys{};
    std::array<uint64_t, 64> m_enPassantKeys{};
    uint64_t m_sideKey{};

    static int castlingMask(const BoardState &brd);
  };

} // namespace chs

#endif //SFML_CHESS_ZOBRIST_H
