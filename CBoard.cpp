//
// Created by Petr Smerda on 01.07.2024.
//

#include "CBoard.h"


CBoard::CBoard(sf::Texture textures[12]) : wPawns(0), wKnights(0), wBishops(0), wRooks(0), wQueens(0), wKing(0),
                                           bPawns(0), bKnights(0), bBishops(0), bRooks(0), bQueens(0), bKing(0),
                                           wCastling(0), bCastling(0) {
// Place pieces on initial positions
  wPawns = 0xFF00ULL;
  wKnights = 0x42ULL;
  wBishops = 0x24ULL;
  wRooks = 0x81ULL;
  wQueens = 0x8ULL;
  wKing = 0x10ULL;

  bPawns = 0xFF000000000000ULL;
  bKnights = 0x4200000000000000ULL;
  bBishops = 0x2400000000000000ULL;
  bRooks = 0x8100000000000000ULL;
  bQueens = 0x8ULL << 56;
  bKing = 0x10ULL << 56;


  wCastling = 0x44ULL;
  bCastling = 0x4400000000000000ULL;

  enPassant = 0;

  onTurn = 1;


  for (int i = 0; i < 12; ++i) {
    this->textures[i] = textures[i];
    sprites[i].setTexture(textures[i]);

    // Get the current texture size
    sf::Vector2u textureSize = textures[i].getSize();

    // Calculate the scale factor to fit within tileSize
    float scaleFactor = static_cast<float>(TILE) / std::max(textureSize.x, textureSize.y);

    sprites[i].setScale(scaleFactor, scaleFactor);
  }
}


void CBoard::draw(sf::RenderWindow &window, Bitboard moveFrom) {
  sf::Color tileCols[2];
  tileCols[0] = sf::Color(200, 100, 50, 255);
  tileCols[1] = sf::Color(220, 210, 180, 255);

  sf::RectangleShape rectangle(sf::Vector2f(TILE - BORDER, TILE - BORDER));
  rectangle.setOutlineColor(sf::Color::Black);
  rectangle.setOutlineThickness(BORDER / 2);





  // Drawing the board

  // We have brown screen, now we need o print white squares
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      if ((i + j) % 2 == 0) {
        rectangle.setPosition(i * TILE + BORDER / 2, j * TILE + BORDER / 2);
        rectangle.setFillColor(tileCols[1]);
        window.draw(rectangle);
      } else {
        rectangle.setPosition(i * TILE + BORDER / 2, j * TILE + BORDER / 2);
        rectangle.setFillColor(tileCols[0]);
        window.draw(rectangle);
      }
    }
  }


  Bitboard posMask = moveFrom;

  // Drawing a selected square
  int pos = 0;

  if (posMask != 0) {
    while (!(posMask & 1)) {
      posMask >>= 1;
      ++pos;
    }
  } else {
    pos = -1;
  }

  if (pos != -1) {
    int x = pos % 8;
    int y = 7 - (pos / 8);
    rectangle.setPosition(x * TILE + BORDER / 2, y * TILE + BORDER / 2);
    rectangle.setFillColor(sf::Color(70, 90, 170, 255));
    window.draw(rectangle);
  }


  Bitboard possibleMoves = legalMoves(moveFrom);
  // Highlighting the possible moves
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      if ((static_cast<Bitboard>(1) << (i + j * 8)) & possibleMoves) {
        rectangle.setPosition(i * TILE + BORDER / 2, (7 - j) * TILE + BORDER / 2);
        rectangle.setFillColor(sf::Color(80, 200, 100, 200));
        window.draw(rectangle);
      }
    }
  }



  // Function to position pieces based on bitboard
  auto posFromBitboard = [&](sf::Sprite &sprite, Bitboard bitboard) {
    for (int square = 0; square < 64; ++square) {
      if (bitboard & (1ULL << square)) {
        int rank = square / 8;
        int file = square % 8;
        sprite.setPosition(file * TILE, (7 - rank) * TILE);

        window.draw(sprite);
      }
    }
  };

  // Draw the pieces
  posFromBitboard(sprites[0], wPawns); // White pieces
  posFromBitboard(sprites[1], wKing);
  posFromBitboard(sprites[2], wKnights);
  posFromBitboard(sprites[3], wBishops);
  posFromBitboard(sprites[4], wQueens);
  posFromBitboard(sprites[5], wRooks);

  posFromBitboard(sprites[6], bPawns); // Black pieces
  posFromBitboard(sprites[7], bKing);
  posFromBitboard(sprites[8], bKnights);
  posFromBitboard(sprites[9], bBishops);
  posFromBitboard(sprites[10], bQueens);
  posFromBitboard(sprites[11], bRooks);
}

/*
 ************************************************************
 *                                                          *
 *                          Logic                           *
 *                          Logic                           *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::white() const {
  return wPawns | wKnights | wBishops | wRooks | wQueens | wKing;
}

Bitboard CBoard::black() const {
  return bPawns | bKnights | bBishops | bRooks | bQueens | bKing;
}

bool CBoard::whiteToMove() const {
  return onTurn == 1;
}

bool CBoard::blackToMove() const {
  return onTurn == -1;
}

bool CBoard::whiteWon() {
  return true;
  //return !(isCheckmate() && whiteToMove());
}


Bitboard CBoard::empty() const {
  return ~white() & ~black();
}

int CBoard::pieceCount() const {
  Bitboard board = white() | black();
  int counter = 0;

  while (board) {
    if (board & 1)
      ++counter;
    board >>= 1;
  }

  return counter;
}

void CBoard::movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo) {
  pieces &= ~moveFrom;  // Clear the source bit
  pieces |= moveTo;     // Set the destination bit
}


void CBoard::removeCapturedBlack(Bitboard piece, Bitboard &removedFrom, char &pieceType) {
  if (bPawns & piece) {
    bPawns &= ~piece;
    pieceType = 'P';
    removedFrom = piece;
  } else if (bKnights & piece) {
    bKnights &= ~piece;
    pieceType = 'N';
    removedFrom = piece;
  } else if (bBishops & piece) {
    bBishops &= ~piece;
    pieceType = 'B';
    removedFrom = piece;
  } else if (bRooks & piece) {
    bRooks &= ~piece;
    pieceType = 'R';
    removedFrom = piece;
  } else if (bQueens & piece) {
    bQueens &= ~piece;
    pieceType = 'Q';
    removedFrom = piece;
  } else if (bKing & piece) {
    bKing &= ~piece;
    pieceType = 'K';
    removedFrom = piece;
  }
}

void CBoard::removeCapturedWhite(Bitboard piece, Bitboard &removedFrom, char &pieceType) {
  if (wPawns & piece) {
    wPawns &= ~piece;
    pieceType = 'P';
    removedFrom = piece;
  } else if (wKnights & piece) {
    wKnights &= ~piece;
    pieceType = 'N';
    removedFrom = piece;
  } else if (wBishops & piece) {
    wBishops &= ~piece;
    pieceType = 'B';
    removedFrom = piece;
  } else if (wRooks & piece) {
    wRooks &= ~piece;
    pieceType = 'R';
    removedFrom = piece;
  } else if (wQueens & piece) {
    wQueens &= ~piece;
    pieceType = 'Q';
    removedFrom = piece;
  } else if (wKing & piece) {
    wKing &= ~piece;
    pieceType = 'K';
    removedFrom = piece;
  }
}


/*
 ************************************************************
 *                                                          *
 *                       Pawn movement                      *
 *                       Pawn movement                      *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::wSinglePush(Bitboard pawns) const {
  return nortOne(pawns) & empty();
}

Bitboard CBoard::bSinglePush(Bitboard pawns) const {
  return soutOne(pawns) & empty();
}

Bitboard CBoard::wDoublePush(Bitboard pawns) const {
  return nortOne(wSinglePush(pawns)) & empty() & RANK_4;
}

Bitboard CBoard::bDoublePush(Bitboard pawns) const {
  return soutOne(bSinglePush(pawns)) & empty() & RANK_5;
}


Bitboard CBoard::wPawnMoves(Bitboard pos) const {
  return wSinglePush(pos) |
         wDoublePush(pos) |
         ((wPawnWestAttacks(pos) |
           wPawnEastAttacks(pos)) &
          (black() | enPassant));
}


Bitboard CBoard::bPawnMoves(Bitboard pos) const {
  return bSinglePush(pos) |
         bDoublePush(pos) |
         ((bPawnWestAttacks(pos) |
           bPawnEastAttacks(pos)) &
          (white() | enPassant));
}


/*
 ************************************************************
 *                                                          *
 *                    Knight movement                       *
 *                    Knight movement                       *
 *                                                          *
 ************************************************************
 */



Bitboard CBoard::wKnightMoves(Bitboard pos) const {
  return (noNoEa(pos) & enemyOrEmpty<true>(*this)) |
         (noEaEa(pos) & enemyOrEmpty<true>(*this)) |
         (soEaEa(pos) & enemyOrEmpty<true>(*this)) |
         (soSoEa(pos) & enemyOrEmpty<true>(*this)) |
         (soSoWe(pos) & enemyOrEmpty<true>(*this)) |
         (soWeWe(pos) & enemyOrEmpty<true>(*this)) |
         (noWeWe(pos) & enemyOrEmpty<true>(*this)) |
         (noNoWe(pos) & enemyOrEmpty<true>(*this));
}

Bitboard CBoard::bKnightMoves(Bitboard pos) const {
  return (noNoEa(pos) & enemyOrEmpty<false>(*this)) |
         (noEaEa(pos) & enemyOrEmpty<false>(*this)) |
         (soEaEa(pos) & enemyOrEmpty<false>(*this)) |
         (soSoEa(pos) & enemyOrEmpty<false>(*this)) |
         (soSoWe(pos) & enemyOrEmpty<false>(*this)) |
         (soWeWe(pos) & enemyOrEmpty<false>(*this)) |
         (noWeWe(pos) & enemyOrEmpty<false>(*this)) |
         (noNoWe(pos) & enemyOrEmpty<false>(*this));
}


/*
 ************************************************************
 *                                                          *
 *                    Bishop movement                       *
 *                    Bishop movement                       *
 *                                                          *
 ************************************************************
 */



Bitboard CBoard::wBishopMoves(Bitboard pos) const {
  Bitboard res = 0;

  Bitboard currentPos = noWe(pos);


  // North-West direction
  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = noWe(currentPos);
  }

  currentPos = noEa(pos);

  // North-East direction
  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = noEa(currentPos);
  }

  currentPos = soWe(pos);

  // South-West direction
  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = soWe(currentPos);
  }

  currentPos = soEa(pos);

  // South-East direction
  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = soEa(currentPos);
  }


  // Here we need to clear the start position
  return res & ~pos;
}

Bitboard CBoard::bBishopMoves(Bitboard pos) const {
  Bitboard res = 0;

  Bitboard currentPos = noWe(pos);

  // North-West direction
  while (currentPos & enemyOrEmpty<false>(*this)) {
    res |= currentPos;
    if (currentPos & white())
      break;
    currentPos = noWe(currentPos);
  }


  // North-East direction
  currentPos = noEa(pos);

  while (currentPos & enemyOrEmpty<false>(*this)) {
    res |= currentPos;
    if (currentPos & white())
      break;
    currentPos = noEa(currentPos);
  }


  // South-West direction
  currentPos = soWe(pos);

  while (currentPos & enemyOrEmpty<false>(*this)) {
    res |= currentPos;
    if (currentPos & white())
      break;
    currentPos = soWe(currentPos);
  }


  // South-East direction
  currentPos = soEa(pos);

  while (currentPos & enemyOrEmpty<false>(*this)) {
    res |= currentPos;
    if (currentPos & white())
      break;
    currentPos = soEa(currentPos);
  }


  // Here we need to clear the start position
  return res & ~pos;
}


/*
 ************************************************************
 *                                                          *
 *                      Rook movement                       *
 *                      Rook movement                       *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::wRookMoves(Bitboard pos) const {
  Bitboard res = 0;

  // North direction
  Bitboard currentPos = nortOne(pos);

  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = nortOne(currentPos);
  }


  // East direction
  currentPos = eastOne(pos);

  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = eastOne(currentPos);
  }


  // South direction
  currentPos = soutOne(pos);

  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = soutOne(currentPos);
  }


  // West direction
  currentPos = westOne(pos);

  while (currentPos & enemyOrEmpty<true>(*this)) {
    res |= currentPos;
    if (currentPos & black())
      break;
    currentPos = westOne(currentPos);
  }


  // Here we need to clear the start position
  return res & ~pos;
}

Bitboard CBoard::bRookMoves(Bitboard pos) const {
  Bitboard res = 0;
  Bitboard newPos = nortOne(pos);


  // North direction
  while (newPos & enemyOrEmpty<false>(*this)) {
    res |= newPos;
    if (newPos & white())
      break;
    newPos = nortOne(newPos);
  }


  // East direction
  newPos = eastOne(pos);

  while (newPos & enemyOrEmpty<false>(*this)) {
    res |= newPos;
    if (newPos & white())
      break;
    newPos = eastOne(newPos);
  }


  // South direction
  newPos = soutOne(pos);


  while (newPos & enemyOrEmpty<false>(*this)) {
    res |= newPos;
    if (newPos & white())
      break;
    newPos = soutOne(newPos);
  }


  // West direction
  newPos = westOne(pos);

  while (newPos & enemyOrEmpty<false>(*this)) {
    res |= newPos;
    if (newPos & white())
      break;
    newPos = westOne(newPos);
  }


  // Here we need to clear the start position
  return res & ~pos;
}


/*
 ************************************************************
 *                                                          *
 *                     Queen movement                       *
 *                     Queen movement                       *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::wQueenMoves(Bitboard pos) const { return wBishopMoves(pos) | wRookMoves(pos); }

Bitboard CBoard::bQueenMoves(Bitboard pos) const { return bBishopMoves(pos) | bRookMoves(pos); }


/*
 ************************************************************
 *                                                          *
 *                      King movement                       *
 *                      King movement                       *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::wKingSafe(Bitboard pos) const {
  Bitboard blackAttacks = 0;

  // Pawn must be just attacking
  blackAttacks |= bPawnEastAttacks(black() & bPawns);
  blackAttacks |= bPawnWestAttacks(black() & bPawns);
  blackAttacks |= bKnightMoves(black() & bKnights);
  blackAttacks |= bBishopMoves(black() & bBishops);
  blackAttacks |= bRookMoves(black() & bRooks);
  blackAttacks |= bQueenMoves(black() & bQueens);

  // I need to add the black king movement
  blackAttacks |= bKing >> 7 | bKing >> 8 | bKing >> 9 |
                  bKing << 7 | bKing << 8 | bKing << 9 |
                  bKing << 1 | bKing >> 1;

  return pos & ~blackAttacks;
}

Bitboard CBoard::bKingSafe(Bitboard pos) const {
  Bitboard whiteAttacks = 0;

  // Pawns must be just attacking
  whiteAttacks |= wPawnEastAttacks(white() & wPawns);
  whiteAttacks |= wPawnWestAttacks(white() & wPawns);
  whiteAttacks |= wKnightMoves(white() & wKnights);
  whiteAttacks |= wBishopMoves(white() & wBishops);
  whiteAttacks |= wRookMoves(white() & wRooks);
  whiteAttacks |= wQueenMoves(white() & wQueens);

  // I need to add the white king movement
  whiteAttacks |= wKing >> 7 | wKing >> 8 | wKing >> 9 |
                  wKing << 7 | wKing << 8 | wKing << 9 |
                  wKing << 1 | wKing >> 1;

  return pos & ~whiteAttacks;
}

Bitboard CBoard::wKingMoves(Bitboard pos) const {
  Bitboard castling = 0;


  if (pos & 0x10ULL) {

    // If the king is on starting position -> might be castling if not zero
    if (wKingSafe(pos) && wKingSafe(pos >> 1 & empty()) && wKingSafe(pos >> 2 & empty()) &&
        empty() & pos >> 3)
      castling = wCastling & pos >> 2;

    if (wKingSafe(pos) && wKingSafe(pos << 1 & empty()) && wKingSafe(pos << 2 & empty()))
      castling |= wCastling & pos << 2;
  }

  Bitboard res = (nortOne(pos) | soutOne(pos) |
                  eastOne(pos) | westOne(pos) |
                  noWe(pos) | noEa(pos) |
                  soWe(pos) | soEa(pos)) & enemyOrEmpty<true>(*this);

  return res | castling;
}

Bitboard CBoard::bKingMoves(Bitboard pos) const {
  Bitboard castling = 0;

  if (pos & 0x10ULL << 56) {
    // If the king is on starting position -> might be castling if not zero
    if (bKingSafe(pos) && bKingSafe(pos >> 1 & empty()) && bKingSafe(pos >> 2 & empty()) &&
        empty() & pos >> 3)
      castling = bCastling & pos >> 2;

    if (bKingSafe(pos) && bKingSafe(pos << 1 & empty()) && bKingSafe(pos << 2 & empty()))
      castling |= bCastling & pos << 2;
  }


  Bitboard res = (nortOne(pos) | soutOne(pos) |
                  eastOne(pos) | westOne(pos) |
                  noWe(pos) | noEa(pos) |
                  soWe(pos) | soEa(pos)) & enemyOrEmpty<false>(*this);

  return res | castling;
}


/*
 ************************************************************
 *                                                          *
 *                      Move Generator                      *
 *                      Move Generator                      *
 *                                                          *
 ************************************************************
 */


Bitboard CBoard::pseudoLegalMoves(Bitboard pos) const {
  Bitboard pseudoLegalMoves = 0;


  pseudoLegalMoves |= wPawnMoves(pos & wPawns);
  pseudoLegalMoves |= wKnightMoves(pos & wKnights);
  pseudoLegalMoves |= wBishopMoves(pos & wBishops);
  pseudoLegalMoves |= wRookMoves(pos & wRooks);
  pseudoLegalMoves |= wQueenMoves(pos & wQueens);
  pseudoLegalMoves |= wKingMoves(pos & wKing);

  pseudoLegalMoves |= bPawnMoves(pos & bPawns);
  pseudoLegalMoves |= bKnightMoves(pos & bKnights);
  pseudoLegalMoves |= bBishopMoves(pos & bBishops);
  pseudoLegalMoves |= bRookMoves(pos & bRooks);
  pseudoLegalMoves |= bQueenMoves(pos & bQueens);
  pseudoLegalMoves |= bKingMoves(pos & bKing);

  return pseudoLegalMoves;
}


Bitboard CBoard::legalMoves(Bitboard pos) {
  Bitboard legalMoves = 0;

  while (pos) {
    Bitboard moveFrom = pos & ~pos;
    pos &= pos - 1;

    Bitboard possibleMoves = pseudoLegalMoves(moveFrom);

    while (possibleMoves) {
      Bitboard moveTo = possibleMoves & -possibleMoves;
      possibleMoves &= possibleMoves - 1;

      // If the move from the moveFrom to moveTo position is not legal, then we need to remove it from result
      if (!isMoveLegal(moveFrom, moveTo))
        legalMoves |= moveTo;
    }
  }

  return legalMoves;
}


bool CBoard::isMoveLegal(Bitboard from, Bitboard to) {
  if (!makeMove(from, to)) return false;

  bool isSafe = whiteToMove() ? bKingSafe(bKing) : wKingSafe(wKing);
  unmakeMove();

  return isSafe;
}


bool CBoard::makeMove(const Bitboard moveFrom, const Bitboard moveTo) {
  Bitboard pseudoMoves = pseudoLegalMoves(moveFrom);

  if (!(moveTo & pseudoMoves))
    return false;


  // Must store the info before the move, to revert the move and have the same properties
  MoveInfo moveInfo = {moveFrom, moveTo, 0, enPassant, onTurn == 1 ? wCastling : bCastling, onTurn, false};

  bool wasEnPassant = false;

  if (wPawns & moveFrom) {
    // En-passant
    movePiece(wPawns, moveFrom, moveTo);

    if (moveTo & enPassant) {
      removeCapturedBlack(moveTo >> 8, moveInfo.capturedPiece, moveInfo.capturedPieceType);
    } else if (moveTo & moveFrom << 16) {
      wasEnPassant = true;
      enPassant = moveFrom << 8;
    }


  } else if (wKnights & moveFrom) {
    movePiece(wKnights, moveFrom, moveTo);

  } else if (wBishops & moveFrom) {
    movePiece(wBishops, moveFrom, moveTo);

  } else if (wQueens & moveFrom) {
    movePiece(wQueens, moveFrom, moveTo);

  } else if (wRooks & moveFrom) {
    movePiece(wRooks, moveFrom, moveTo);
    // Disable castling on the side the rook moved;
    wCastling &= eastTwo(wRooks) | westOne(wRooks);

  } else if (wKing & moveFrom) {
    // Castling
    movePiece(wKing, moveFrom, moveTo);

    // bKing == bCastling => castling just happened -> move rook
    if (wKing & wCastling) {
      // If castled to position 2 => rook on pos 0, else on pos 7
      Bitboard rookFrom = wKing & 1ULL << 2 ? 1ULL : 1ULL << 7;
      // If rook on pos 0 is selected, then it moves to pos 3, otherwise to 5
      Bitboard rookTo = rookFrom & 1ULL ? 1ULL << 3 : 1ULL << 5;

      movePiece(wRooks, rookFrom, rookTo);
    }

    wCastling = 0; // If king moves, we need to reset castling


  } else if (bPawns & moveFrom) { // Black pieces
    // En-passant
    movePiece(bPawns, moveFrom, moveTo);

    if (moveTo & enPassant) {
      removeCapturedBlack(moveTo << 8, moveInfo.capturedPiece, moveInfo.capturedPieceType);
    } else if (moveTo & moveFrom >> 16) {
      wasEnPassant = true;
      enPassant = moveFrom >> 8;
    }

  } else if (bKnights & moveFrom) {
    movePiece(bKnights, moveFrom, moveTo);

  } else if (bBishops & moveFrom) {
    movePiece(bBishops, moveFrom, moveTo);

  } else if (bQueens & moveFrom) {
    movePiece(bQueens, moveFrom, moveTo);

  } else if (bRooks & moveFrom) {
    movePiece(bRooks, moveFrom, moveTo);
    // Disable castling on the side the rook moved;
    bCastling &= eastTwo(bRooks) | westOne(bRooks);

  } else if (bKing & moveFrom) {
    movePiece(bKing, moveFrom, moveTo);

    // bKing == bCastling => castling just happened -> move rook
    if (bKing & bCastling) {
      // The same as white, but shifted by 56
      Bitboard rookFrom = bKing & 1ULL << 58 ? 1ULL : 1ULL << 63;
      Bitboard rookTo = rookFrom & 1ULL << 56 ? 1ULL << 59 : 1ULL << 61;

      movePiece(bRooks, rookFrom, rookTo);
    }

    bCastling = 0; // If king moves, we need to reset castling

  }

  if (whiteToMove())
    removeCapturedBlack(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
  else
    removeCapturedWhite(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);


  if (!wasEnPassant)
    enPassant = 0;

  onTurn *= -1;
  m_moveList.push(moveInfo);

  return true;
}

void CBoard::MoveInfo::print() const {
  std::cout << "\n\nmove info: " << std::endl;
  std::cout << "previousOnTurn: " << previousOnTurn << std::endl;
  std::cout << "capturedPiece: " << capturedPiece << std::endl;
  std::cout << "capturedPieceType: " << capturedPieceType << std::endl;
  std::cout << "moveFrom: " << moveFrom << std::endl;
  std::cout << "moveTo: " << moveTo << std::endl;
  std::cout << "wasEnPassant: " << wasEnPassant << std::endl;
  std::cout << "previousEnPassant: " << previousEnPassant << std::endl;
  std::cout << "previousCastlingRights: " << previousCastlingRights << std::endl;
  std::cout << "\n\n";
}


bool CBoard::unmakeMove() {
  if (m_moveList.empty()) return false;

  MoveInfo lastMove = m_moveList.top();
  m_moveList.pop();


  // Determine if the last move was made by white or black
  bool isWhiteMove = (lastMove.previousOnTurn == 1);

  Bitboard &pawns = isWhiteMove ? wPawns : bPawns;
  Bitboard &knights = isWhiteMove ? wKnights : bKnights;
  Bitboard &bishops = isWhiteMove ? wBishops : bBishops;
  Bitboard &rooks = isWhiteMove ? wRooks : bRooks;
  Bitboard &queens = isWhiteMove ? wQueens : bQueens;
  Bitboard &king = isWhiteMove ? wKing : bKing;

  Bitboard &opponentPawns = isWhiteMove ? bPawns : wPawns;
  Bitboard &opponentKnights = isWhiteMove ? bKnights : wKnights;
  Bitboard &opponentBishops = isWhiteMove ? bBishops : wBishops;
  Bitboard &opponentRooks = isWhiteMove ? bRooks : wRooks;
  Bitboard &opponentQueens = isWhiteMove ? bQueens : wQueens;
  Bitboard &opponentKing = isWhiteMove ? bKing : wKing;

  // Unmake the move -> moveTo to moveFrom
  if (pawns & lastMove.moveTo) movePiece(pawns, lastMove.moveTo, lastMove.moveFrom);
  else if (knights & lastMove.moveTo) movePiece(knights, lastMove.moveTo, lastMove.moveFrom);
  else if (bishops & lastMove.moveTo) movePiece(bishops, lastMove.moveTo, lastMove.moveFrom);
  else if (rooks & lastMove.moveTo) movePiece(rooks, lastMove.moveTo, lastMove.moveFrom);
  else if (queens & lastMove.moveTo) movePiece(queens, lastMove.moveTo, lastMove.moveFrom);
  else if (king & lastMove.moveTo) movePiece(king, lastMove.moveTo, lastMove.moveFrom);

  // Handle castling
  if (king & lastMove.moveFrom) {
    if (lastMove.moveTo == (lastMove.moveFrom << 2)) { // Queenside castling
      movePiece(rooks, lastMove.moveFrom >> 3, lastMove.moveFrom >> 1);
    } else if (lastMove.moveTo == (lastMove.moveFrom >> 2)) { // Kingside castling
      movePiece(rooks, lastMove.moveFrom << 3, lastMove.moveFrom << 1);
    }
  }

  // Restore the captured piece, if any
  if (lastMove.capturedPiece) {
    switch (lastMove.capturedPieceType) {
      case 'P':
        movePiece(opponentPawns, 0, lastMove.moveTo);
        break;
      case 'N':
        movePiece(opponentKnights, 0, lastMove.moveTo);
        break;
      case 'B':
        movePiece(opponentBishops, 0, lastMove.moveTo);
        break;
      case 'R':
        movePiece(opponentRooks, 0, lastMove.moveTo);
        break;
      case 'Q':
        movePiece(opponentQueens, 0, lastMove.moveTo);
        break;
      case 'K':
        movePiece(opponentKing, 0, lastMove.moveTo);
        break;
    }
  }

  // Handle en passant
  if ((pawns & lastMove.moveFrom) && (lastMove.moveTo == enPassant)) {
    Bitboard enPassantCapturedPawn = isWhiteMove ? (lastMove.moveTo >> 8) : (lastMove.moveTo << 8);
    movePiece(opponentPawns, 0, enPassantCapturedPawn);
  }

  // Restore previous game state
  enPassant = lastMove.previousEnPassant;
  if (isWhiteMove) wCastling = lastMove.previousCastlingRights;
  else bCastling = lastMove.previousCastlingRights;
  onTurn = lastMove.previousOnTurn;

  return true;
}


Bitboard CBoard::onMovePositions() const {
  return onTurn == 1 ? white() : black();
}


/*
 ************************************************************
 *                                                          *
 *                      AI Methods                          *
 *                      AI Methods                          *
 *                                                          *
 ************************************************************
 */

std::vector<std::pair<Bitboard, Bitboard>> CBoard::generateMoves(Bitboard moveFrom) {
  std::vector<std::pair<Bitboard, Bitboard>> moves;
  Bitboard possibleMoves = legalMoves(moveFrom);

  Bitboard moveTo = 1ULL;

  while (moveTo) {
    if (possibleMoves & moveTo)
      moves.emplace_back(moveFrom, moveTo);

    moveTo <<= 1;
  }

  return moves;
}

CBoard::GameStatus CBoard::isCheckmate() {
  if (whiteToMove()) {
    if (!wKingSafe(wKing)) {
      if (!legalMoves(white()))
        return GameStatus::BlackWon;
      else
        return GameStatus::InProgress;
    } else {
      if (!legalMoves(white()))
        return GameStatus::Draw;
      else
        return GameStatus::InProgress;
    }
  } else {
    if (!bKingSafe(bKing)) {
      if (!legalMoves(black()))
        return GameStatus::WhiteWon;
      else
        return GameStatus::InProgress;
    } else {
      if (!legalMoves(black()))
        return GameStatus::Draw;
      else
        return GameStatus::InProgress;
    }
  }
}


int CBoard::evaluate() {
  int score = 0;

  // Material Count
  score += PAWN_VALUE * popcount(wPawns);
  score += KNIGHT_VALUE * popcount(wKnights);
  score += BISHOP_VALUE * popcount(wBishops);
  score += ROOK_VALUE * popcount(wRooks);
  score += QUEEN_VALUE * popcount(wQueens);
  score += KING_VALUE * popcount(wKing);

  score -= PAWN_VALUE * popcount(bPawns);
  score -= KNIGHT_VALUE * popcount(bKnights);
  score -= BISHOP_VALUE * popcount(bBishops);
  score -= ROOK_VALUE * popcount(bRooks);
  score -= QUEEN_VALUE * popcount(bQueens);
  score -= KING_VALUE * popcount(bKing);

  // Positional values
  score += pieceSquareValue(wPawns, pawnTable);
  score += pieceSquareValue(wKnights, knightTable);
  score += pieceSquareValue(wBishops, bishopTable);
  score += pieceSquareValue(wRooks, rookTable);
  score += pieceSquareValue(wQueens, queenTable);
  score += pieceSquareValue(wKing, kingTable);

  score -= pieceSquareValue(bPawns, pawnTable);
  score -= pieceSquareValue(bKnights, knightTable);
  score -= pieceSquareValue(bBishops, bishopTable);
  score -= pieceSquareValue(bRooks, rookTable);
  score -= pieceSquareValue(bQueens, queenTable);
  score -= pieceSquareValue(bKing, kingTable);

  // Pawn Structure
  score += evaluatePawnStructure(wPawns, bPawns);
  score -= evaluatePawnStructure(bPawns, wPawns);

  // Mobility
  score += evaluateMobility(1);
  score -= evaluateMobility(-1);

  return score;
}

int CBoard::evaluatePawnStructure(Bitboard pawns, Bitboard opponentPawns) {
  int score = 0;

  // Doubled Pawns
  Bitboard doubledPawns = pawns & (pawns >> 8);
  score -= 50 * popcount(doubledPawns);

  // Blocked Pawns
  Bitboard blockedPawns = pawns & (opponentPawns >> 8);
  score -= 30 * popcount(blockedPawns);

  // Isolated Pawns
  Bitboard isolatedPawns = pawns & ~((pawns << 1) | (pawns >> 1));
  score -= 30 * popcount(isolatedPawns);

  return score;
}

int CBoard::evaluateMobility(int side) {
  int mobilityScore = 0;

  Bitboard pieces = (side == 1) ? white() : black();

  Bitboard moveFrom = 1ULL;
  while (moveFrom) {
    if (moveFrom & pieces) {
      mobilityScore += popcount(legalMoves(moveFrom));
    }
    moveFrom <<= 1;
  }

  return mobilityScore;
}

int CBoard::popcount(Bitboard bb) {
  return __builtin_popcountll(bb);
}

int CBoard::pieceSquareValue(Bitboard pieces, const int table[64]) {
  int score = 0;
  Bitboard moveFrom = 1ULL;

  int index = 0;
  while (moveFrom) {
    if (moveFrom & pieces) {
      score += table[index];
    }
    moveFrom <<= 1;
    index++;
  }
  return score;
}


int CBoard::negamax(int depth) {
  if (depth == 0)
    return evaluate();


  int maxEval = INT_MIN;
  Bitboard moveFrom = 1ULL;

  while (moveFrom) {
    if (moveFrom & onMovePositions()) {
      auto moves = generateMoves(moveFrom);
      for (const auto &move: moves) {
        makeMove(move.first, move.second);
        int eval = -negamax(depth - 1);
        unmakeMove();
        maxEval = std::max(maxEval, eval);
      }
    }
    moveFrom <<= 1;
  }

  return maxEval;
}