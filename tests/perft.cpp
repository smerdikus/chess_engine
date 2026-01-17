#include "engine/Board.h"
#include "engine/Fen.h"

#include <cstdint>
#include <iostream>
#include <string>

namespace {
  uint64_t perft(chs::Board &board, int depth) {
    if (depth <= 0) return 1;
    uint64_t nodes = 0;
    auto moves = board.generateMoves(board.onMovePositions());
    for (const auto &move: moves) {
      if (!board.makeMove(move.first, move.second))
        continue;
      nodes += perft(board, depth - 1);
      board.unmakeMove();
    }
    return nodes;
  }

  void expectEq(const std::string &label, uint64_t actual, uint64_t expected, int &failures) {
    if (actual != expected) {
      std::cerr << "[fail] " << label << " expected=" << expected << " got=" << actual << std::endl;
      failures++;
    } else {
      std::cout << "[ok] " << label << " = " << actual << std::endl;
    }
  }
}

int main() {
  int failures = 0;

  {
    chs::Board board;
    expectEq("startpos depth1", perft(board, 1), 20, failures);
    expectEq("startpos depth2", perft(board, 2), 400, failures);
  }

  {
    chs::BoardState state;
    if (!chs::parseFen("8/8/8/8/8/8/8/4K3 w - - 0 1", state)) {
      std::cerr << "[fail] FEN parse basic position" << std::endl;
      failures++;
    } else {
      if (state.wKing != chs::E1 || state.onTurn != 1 || state.wCastling != 0 || state.bCastling != 0 ||
          state.enPassant != 0) {
        std::cerr << "[fail] FEN fields do not match expected values" << std::endl;
        failures++;
      } else {
        std::cout << "[ok] FEN parse basic position" << std::endl;
      }
    }
  }

  return failures == 0 ? 0 : 1;
}
