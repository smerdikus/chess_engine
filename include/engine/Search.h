#ifndef SFML_CHESS_SEARCH_H
#define SFML_CHESS_SEARCH_H

#include <array>
#include <cstdint>
#include <utility>
#include <vector>
#include "engine/Board.h"

namespace chs {

  class Search {
  public:
    explicit Search(size_t ttSize = 1 << 19, int noPrunePieces = 10);

    std::pair<int, std::pair<Bitboard, Bitboard>> negamax(Board &board, int depth);
    std::pair<int, std::pair<Bitboard, Bitboard>> iterativeDeepening(Board &board, int maxDepth);
    char lastPromotion() const { return m_lastPromotion; }

  private:
    static constexpr int MAX_PLY = 64;

    struct TTEntry {
      uint64_t key = 0;
      int depth = -1;
      int score = 0;
      uint8_t flag = 0;
      Bitboard bestFrom = 0;
      Bitboard bestTo = 0;
    };

    struct ScoredMove {
      Bitboard from;
      Bitboard to;
      int score;
    };

    struct KillerMove {
      Bitboard from = 0;
      Bitboard to = 0;
    };

    std::vector<TTEntry> m_tt;
    int m_noPrunePieces;
    std::array<std::array<KillerMove, 2>, MAX_PLY> m_killers{};
    std::array<std::array<std::array<int, 64>, 64>, 2> m_history{};
    char m_lastPromotion = '\0';

    static inline int squareIndex(Bitboard square) {
      return __builtin_ctzll(square);
    }

    uint64_t positionKey(const BoardState &state) const;

    TTEntry *probeTT(uint64_t key);

    void storeTT(uint64_t key, int depth, int score, uint8_t flag, Bitboard bestFrom, Bitboard bestTo);

    int pieceValueAt(const BoardState &state, Bitboard square, bool isWhite) const;

    std::vector<ScoredMove> orderedMoves(Board &board, const TTEntry *ttEntry, int ply);

    void recordHistory(bool isWhite, Bitboard from, Bitboard to, int depth);

    void recordKiller(Bitboard from, Bitboard to, int ply);

    int negamaxSearch(Board &board, int depth, int alpha, int beta, Bitboard &bestFrom, Bitboard &bestTo,
                      char &bestPromotion, int ply);
  };

} // namespace chs

#endif //SFML_CHESS_SEARCH_H
