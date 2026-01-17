//
// FEN parsing helpers.
//

#include "engine/Fen.h"

#include <cctype>
#include <sstream>
#include <vector>

namespace chs {

  namespace {
    bool setPiece(BoardState &out, char piece, int square) {
      if (square < 0 || square >= 64) return false;
      Bitboard bit = 1ULL << square;

      switch (piece) {
        case 'P': out.wPawns |= bit; return true;
        case 'N': out.wKnights |= bit; return true;
        case 'B': out.wBishops |= bit; return true;
        case 'R': out.wRooks |= bit; return true;
        case 'Q': out.wQueens |= bit; return true;
        case 'K': out.wKing |= bit; return true;
        case 'p': out.bPawns |= bit; return true;
        case 'n': out.bKnights |= bit; return true;
        case 'b': out.bBishops |= bit; return true;
        case 'r': out.bRooks |= bit; return true;
        case 'q': out.bQueens |= bit; return true;
        case 'k': out.bKing |= bit; return true;
        default: return false;
      }
    }

    bool parseSquare(const std::string &token, Bitboard &out) {
      if (token.size() != 2) return false;
      char file = static_cast<char>(std::tolower(token[0]));
      char rank = token[1];

      if (file < 'a' || file > 'h') return false;
      if (rank < '1' || rank > '8') return false;

      int fileIndex = file - 'a';
      int rankIndex = rank - '1';

      out = 1ULL << (rankIndex * 8 + fileIndex);
      return true;
    }
  } // namespace

  bool parseFen(const std::string &fen, BoardState &out) {
    out = {};
    out.onTurn = 1;

    std::istringstream iss(fen);
    std::vector<std::string> fields;
    std::string field;
    while (iss >> field)
      fields.push_back(field);

    if (fields.size() < 4)
      return false;

    const std::string &placement = fields[0];
    const std::string &side = fields[1];
    const std::string &castling = fields[2];
    const std::string &enPassant = fields[3];

    int rank = 7;
    int fileIndex = 0;
    for (char c: placement) {
      if (c == '/') {
        if (fileIndex != 8) return false;
        rank--;
        fileIndex = 0;
        continue;
      }

      if (std::isdigit(static_cast<unsigned char>(c))) {
        int empty = c - '0';
        if (empty <= 0 || empty > 8) return false;
        fileIndex += empty;
        if (fileIndex > 8) return false;
        continue;
      }

      if (rank < 0 || fileIndex >= 8) return false;
      int square = rank * 8 + fileIndex;
      if (!setPiece(out, c, square)) return false;
      fileIndex++;
    }

    if (rank != 0 || fileIndex != 8)
      return false;

    if (side == "w") out.onTurn = 1;
    else if (side == "b") out.onTurn = -1;
    else return false;

    out.wCastling = 0;
    out.bCastling = 0;
    if (castling != "-") {
      for (char c: castling) {
        switch (c) {
          case 'K': out.wCastling |= G1; break;
          case 'Q': out.wCastling |= C1; break;
          case 'k': out.bCastling |= G8; break;
          case 'q': out.bCastling |= C8; break;
          default: return false;
        }
      }
    }

    out.enPassant = 0;
    if (enPassant != "-") {
      if (!parseSquare(enPassant, out.enPassant))
        return false;
    }

    return true;
  }

} // namespace chs
