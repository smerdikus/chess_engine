#ifndef SFML_CHESS_FEN_H
#define SFML_CHESS_FEN_H

#include <string>
#include "engine/ChessMacros.h"

namespace chs {

  bool parseFen(const std::string &fen, BoardState &out);

} // namespace chs

#endif //SFML_CHESS_FEN_H
