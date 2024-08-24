//
// Created by Petr Smerda on 01.07.2024.
//

#include "CBoard.h"


CBoard::CBoard(sf::Texture textures[12]) {
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
  bQueens = 0x800000000000000ULL;
  bKing = 0x1000000000000000ULL;

  wCastling = 0x44ULL;
  bCastling = 0x4400000000000000ULL;
  enPassant = 0;

  onTurn = 1;

  // Assign and scale the textures
  for (int i = 0; i < 12; ++i) {
    m_sprites[i].setTexture(textures[i]);

    sf::Vector2u textureSize = textures[i].getSize();

    // Calculate the scale factor to fit within tileSize
    float scaleFactor = static_cast<float>(TILE) / static_cast<float>(std::max(textureSize.x, textureSize.y));

    m_sprites[i].setScale(scaleFactor, scaleFactor);
  }


  lightSquareColor = sf::Color(240, 248, 255);  // Alice blue
  darkSquareColor = sf::Color(70, 130, 180);    // Steel blue
  borderColor = sf::Color(60, 100, 150);        // Deep blue
  highlightSrcColor = sf::Color(0, 191, 255);   // Deep sky blue
  highlightDstColor = sf::Color(30, 144, 255);  // Dodger blue


  m_rectangle = sf::RectangleShape(sf::Vector2f(TILE - BORDER * 2, TILE - BORDER * 2));
  m_rectangle.setOutlineColor(borderColor);
  m_rectangle.setOutlineThickness(BORDER);
}


void CBoard::draw(sf::RenderWindow &window, Bitboard moveFrom) {

  // Drawing the board and squares
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      m_rectangle.setPosition(static_cast<float>(x * TILE + BORDER), static_cast<float>(y * TILE + BORDER));
      m_rectangle.setFillColor((x + y) % 2 == 0 ? lightSquareColor : darkSquareColor);
      window.draw(m_rectangle);
    }
  }

  // Drawing the selected square, if any
  if (moveFrom) {
    int pos = __builtin_ctzll(moveFrom);
    int x = pos % 8;
    int y = 7 - (pos / 8);

    m_rectangle.setPosition(static_cast<float>(x * TILE + BORDER), static_cast<float>(y * TILE + BORDER));
    m_rectangle.setFillColor(highlightSrcColor);
    window.draw(m_rectangle);
  }

  // Highlighting possible moves
  Bitboard possibleMoves = legalMoves(moveFrom);
  for (int square = 0; square < 64; ++square) {
    if (possibleMoves & (1ULL << square)) {
      int x = square % 8;
      int y = 7 - (square / 8);

      m_rectangle.setPosition(static_cast<float>(x * TILE + BORDER), static_cast<float>(y * TILE + BORDER));
      m_rectangle.setFillColor(highlightDstColor); // Highlight color for possible moves
      window.draw(m_rectangle);
    }
  }

  // Function to position pieces based on bitboard
  auto posFromBitboard = [&](sf::Sprite &sprite, Bitboard bitboard) {
    for (int square = 0; square < 64; ++square) {
      if (bitboard & (1ULL << square)) {
        int rank = square / 8;
        int file = square % 8;

        // Here I need to reverse the positions -> 7 - rank
        sprite.setPosition(static_cast<float>(file) * TILE, static_cast<float>(7 - rank) * TILE);

        window.draw(sprite);
      }
    }
  };

  // Draw the pieces
  posFromBitboard(m_sprites[0], wPawns);
  posFromBitboard(m_sprites[1], wKing);
  posFromBitboard(m_sprites[2], wKnights);
  posFromBitboard(m_sprites[3], wBishops);
  posFromBitboard(m_sprites[4], wQueens);
  posFromBitboard(m_sprites[5], wRooks);

  posFromBitboard(m_sprites[6], bPawns);
  posFromBitboard(m_sprites[7], bKing);
  posFromBitboard(m_sprites[8], bKnights);
  posFromBitboard(m_sprites[9], bBishops);
  posFromBitboard(m_sprites[10], bQueens);
  posFromBitboard(m_sprites[11], bRooks);
}


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


Bitboard CBoard::empty() const {
  return ~white() & ~black();
}

int CBoard::pieceCount() const {
  return popcount(white() | black());
}

void CBoard::movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo) {
  pieces &= ~moveFrom;  // Clear the source bit
  pieces |= moveTo;     // Set the destination bit
}

void CBoard::removeCapturedBlack(Bitboard moveTo, Bitboard &removedFrom, char &pieceType) {
  // Array of bitboards for black pieces and corresponding moveTo types
  Bitboard *blackPieces[] = {&bPawns, &bKnights, &bBishops, &bRooks, &bQueens, &bKing};
  char blackPieceTypes[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

  for (int i = 0; i < 6; ++i) {
    if (*blackPieces[i] & moveTo) {
      *blackPieces[i] &= ~moveTo;
      pieceType = blackPieceTypes[i];
      removedFrom = moveTo;
      break;
    }
  }
}

void CBoard::removeCapturedWhite(Bitboard piece, Bitboard &removedFrom, char &pieceType) {
  // Array of bitboards for white pieces and corresponding piece types
  Bitboard *whitePieces[] = {&wPawns, &wKnights, &wBishops, &wRooks, &wQueens, &wKing};
  char whitePieceTypes[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

  for (int i = 0; i < 6; ++i) {
    if (*whitePieces[i] & piece) {
      *whitePieces[i] &= ~piece;
      pieceType = whitePieceTypes[i];
      removedFrom = piece;
      break;
    }
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
  return wSinglePush(pos) | wDoublePush(pos) |
         ((wPawnWestAttacks(pos) |
           wPawnEastAttacks(pos)) &
          (black() | enPassant));
}


Bitboard CBoard::bPawnMoves(Bitboard pos) const {
  return bSinglePush(pos) | bDoublePush(pos) |
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
  return (noNoEa(pos) & enemyOrEmpty<true>()) | (noEaEa(pos) & enemyOrEmpty<true>()) |
         (soEaEa(pos) & enemyOrEmpty<true>()) | (soSoEa(pos) & enemyOrEmpty<true>()) |
         (soSoWe(pos) & enemyOrEmpty<true>()) | (soWeWe(pos) & enemyOrEmpty<true>()) |
         (noWeWe(pos) & enemyOrEmpty<true>()) | (noNoWe(pos) & enemyOrEmpty<true>());
}

Bitboard CBoard::bKnightMoves(Bitboard pos) const {
  return (noNoEa(pos) & enemyOrEmpty<false>()) | (noEaEa(pos) & enemyOrEmpty<false>()) |
         (soEaEa(pos) & enemyOrEmpty<false>()) | (soSoEa(pos) & enemyOrEmpty<false>()) |
         (soSoWe(pos) & enemyOrEmpty<false>()) | (soWeWe(pos) & enemyOrEmpty<false>()) |
         (noWeWe(pos) & enemyOrEmpty<false>()) | (noNoWe(pos) & enemyOrEmpty<false>());
}


/*
 ************************************************************
 *                                                          *
 *                    Bishop movement                       *
 *                    Bishop movement                       *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::sliderMoves(Bitboard pos, Bitboard (*directionFunc)(Bitboard), Bitboard enemies, Bitboard empty) {
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


Bitboard CBoard::bishopMoves(Bitboard pos, Bitboard enemies, Bitboard empty) {
  Bitboard res = 0;

  res |= sliderMoves(pos, noWe, enemies, empty);
  res |= sliderMoves(pos, noEa, enemies, empty);
  res |= sliderMoves(pos, soWe, enemies, empty);
  res |= sliderMoves(pos, soEa, enemies, empty);

  return res & ~pos;  // Clear the start position
}

// Wrapper functions for white and black bishops
Bitboard CBoard::wBishopMoves(Bitboard pos) const {
  return bishopMoves(pos, black(), empty());
}

Bitboard CBoard::bBishopMoves(Bitboard pos) const {
  return bishopMoves(pos, white(), empty());
}


/*
 ************************************************************
 *                                                          *
 *                      Rook movement                       *
 *                      Rook movement                       *
 *                                                          *
 ************************************************************
 */

Bitboard CBoard::rookMoves(Bitboard pos, Bitboard enemies, Bitboard empty) {
  Bitboard res = 0;


  // Apply the lambda for each straight direction
  res |= sliderMoves(pos, nortOne, enemies, empty);
  res |= sliderMoves(pos, eastOne, enemies, empty);
  res |= sliderMoves(pos, soutOne, enemies, empty);
  res |= sliderMoves(pos, westOne, enemies, empty);

  return res & ~pos;  // Clear the start position
}

// Wrapper functions for white and black rooks
Bitboard CBoard::wRookMoves(Bitboard pos) const {
  return rookMoves(pos, black(), empty());
}

Bitboard CBoard::bRookMoves(Bitboard pos) const {
  return rookMoves(pos, white(), empty());
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

Bitboard CBoard::oneAround(Bitboard pos) {
  return nortOne(pos) | soutOne(pos) | eastOne(pos) | westOne(pos) |
         noWe(pos) | noEa(pos) | soWe(pos) | soEa(pos);
}

Bitboard CBoard::wKingSafe(Bitboard pos) const {
  Bitboard blackAttacks = 0;

  // Pawn must be just attacking
  blackAttacks |= bPawnEastAttacks(bPawns);
  blackAttacks |= bPawnWestAttacks(bPawns);
  blackAttacks |= bKnightMoves(bKnights);
  blackAttacks |= bBishopMoves(bBishops);
  blackAttacks |= bRookMoves(bRooks);
  blackAttacks |= bQueenMoves(bQueens);
  blackAttacks |= oneAround(bKing);

  return pos & ~blackAttacks; // Return squares not attacked by white and bKing
}

Bitboard CBoard::bKingSafe(Bitboard pos) const {
  Bitboard whiteAttacks = 0;

  // Pawns must be just attacking
  whiteAttacks |= wPawnEastAttacks(wPawns);
  whiteAttacks |= wPawnWestAttacks(wPawns);
  whiteAttacks |= wKnightMoves(wKnights);
  whiteAttacks |= wBishopMoves(wBishops);
  whiteAttacks |= wRookMoves(wRooks);
  whiteAttacks |= wQueenMoves(wQueens);
  whiteAttacks |= oneAround(wKing);

  return pos & ~whiteAttacks; // Return squares not attacked by white and bKing
}

Bitboard CBoard::wKingMoves(Bitboard pos) const {
  Bitboard castling = 0;

  // If the king is on starting position -> might be castling if not zero
  if (wKingSafe(pos) && wKingSafe(pos >> 1 & empty()) && wKingSafe(pos >> 2 & empty()) &&
      empty() & pos >> 3)
    castling = wCastling & pos >> 2;

  if (wKingSafe(pos) && wKingSafe(pos << 1 & empty()) && wKingSafe(pos << 2 & empty()))
    castling |= wCastling & pos << 2;


  Bitboard res = oneAround(pos) & enemyOrEmpty<true>();

  return res | castling;
}

Bitboard CBoard::bKingMoves(Bitboard pos) const {
  Bitboard castling = 0;

  // If the king is on starting position -> might be castling if not zero
  if (bKingSafe(pos) && bKingSafe(pos >> 1 & empty()) && bKingSafe(pos >> 2 & empty()) &&
      empty() & pos >> 3)
    castling = bCastling & pos >> 2;

  if (bKingSafe(pos) && bKingSafe(pos << 1 & empty()) && bKingSafe(pos << 2 & empty()))
    castling |= bCastling & pos << 2;


  Bitboard res = oneAround(pos) & enemyOrEmpty<false>();

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
    // Separate the lowest one from the position
    Bitboard moveFrom = pos & -pos;
    // And delete it from the pos
    pos &= pos - 1;

    Bitboard possibleMoves = pseudoLegalMoves(moveFrom);

    while (possibleMoves) {
      Bitboard moveTo = possibleMoves & -possibleMoves;
      possibleMoves &= possibleMoves - 1;

      // If the move from the moveFrom to moveTo position is not legal, then we need to remove it from result
      if (isMoveLegal(moveFrom, moveTo))
        legalMoves |= moveTo;
    }
  }

  return legalMoves;
}


bool CBoard::isMoveLegal(Bitboard from, Bitboard to) {

  makeMove(from, to);
  bool isValid = whiteToMove() ? bKingSafe(bKing) : wKingSafe(wKing);
  unmakeMove();

  return isValid;
}


std::vector<std::pair<Bitboard, Bitboard>> CBoard::generateMoves(Bitboard moveFrom) {
  std::vector<std::pair<Bitboard, Bitboard>> moves;
  Bitboard possibleMoves = legalMoves(moveFrom);

  while (possibleMoves) {
    // Isolate the least significant set bit
    Bitboard moveTo = possibleMoves & -possibleMoves;

    // Add the move from moveFrom to moveTo
    moves.emplace_back(moveFrom, moveTo);

    // Remove the least significant set bit from possibleMoves
    possibleMoves &= possibleMoves - 1;
  }

  std::cout << "we have " << moves.size() << "moves\n";

  return moves;
}


bool CBoard::canMakeMove(Bitboard moveFrom, Bitboard moveTo) {
  return moveTo & legalMoves(moveFrom);
}


bool CBoard::makeMove(const Bitboard moveFrom, const Bitboard moveTo) {
  Bitboard pseudoMoves = pseudoLegalMoves(moveFrom);

  if (!(moveTo & pseudoMoves))
    return false;


  // Must store the info before the move
  MoveInfo moveInfo = {moveFrom, moveTo, 0, enPassant, onTurn == 1 ? wCastling : bCastling, onTurn, false, 0};

  bool enPassantSet = false;

  if (wPawns & moveFrom) {
    // En-passant
    movePiece(wPawns, moveFrom, moveTo);

    if (moveTo & enPassant) {
      // Capture the pawn behind the position the actual pawn moved to
      removeCapturedBlack(soutOne(moveTo), moveInfo.capturedPiece, moveInfo.capturedPieceType);
    } else if (moveTo & nortTwo(moveFrom)) {
      // Here we need to set the position of possible en-passant
      enPassant = nortOne(moveFrom);
      enPassantSet = true;
    }


    // Handling of promotion
    if (moveTo & RANK_8) {
      // Promotion occured
      std::cout << "white promotion\n";
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

    // If king moves, we need to disable castling
    wCastling = 0;


  } else if (bPawns & moveFrom) { // Black pieces
    // En-passant
    movePiece(bPawns, moveFrom, moveTo);

    // If the destination matches en-passant move then handle it
    if (moveTo & enPassant) {
      // Capture the pawn behind the position the actual pawn moved to
      removeCapturedBlack(nortOne(moveTo), moveInfo.capturedPiece, moveInfo.capturedPieceType);
    } else if (moveTo & soutTwo(moveFrom)) {
      // Here we need to set en-passant to enable it in the next move if played
      enPassant = soutOne(moveFrom);
      enPassantSet = true;
    }


    // Handling of promotion
    if (moveTo & RANK_1) {
      // Promotion occured
      std::cout << "black promotion\n";
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

    // If king moves, we need to disable castling
    bCastling = 0;

  }

  if (whiteToMove())
    removeCapturedBlack(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
  else
    removeCapturedWhite(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);


  if (!enPassantSet)
    enPassant = 0;


  onTurn *= -1;
  m_moveList.push(moveInfo);

  return true;
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

  Bitboard revertMove[2] = { lastMove.moveTo, lastMove.moveFrom };

  // Unmake the move -> moveTo to moveFrom
  if (pawns & lastMove.moveTo) movePiece(pawns, revertMove[0], revertMove[1]);
  else if (knights & lastMove.moveTo) movePiece(knights, revertMove[0], revertMove[1]);
  else if (bishops & lastMove.moveTo) movePiece(bishops, revertMove[0], revertMove[1]);
  else if (rooks & lastMove.moveTo) movePiece(rooks, revertMove[0], revertMove[1]);
  else if (queens & lastMove.moveTo) movePiece(queens, revertMove[0], revertMove[1]);
  else if (king & lastMove.moveTo) movePiece(king, revertMove[0], revertMove[1]);


  // Handle castling
  if (king & lastMove.moveFrom) {

    if (lastMove.moveTo == lastMove.moveFrom << 2) {
      // KingSide castling
      Bitboard rookInitialPos = lastMove.moveFrom << 3;
      Bitboard rookFinalPos = lastMove.moveFrom << 1;
      movePiece(rooks, rookFinalPos, rookInitialPos);

    } else if (lastMove.moveTo == lastMove.moveFrom >> 2) {
      // QueenSide castling
      Bitboard rookInitialPos = lastMove.moveFrom >> 4;
      Bitboard rookFinalPos = lastMove.moveFrom >> 1;
      movePiece(rooks, rookFinalPos, rookInitialPos);
    }
  }

  // Restore the captured piece, if any
  if (lastMove.capturedPiece) {
    switch (lastMove.capturedPieceType) {
      case 'P':
        movePiece(opponentPawns, 0, lastMove.capturedPiece);
        break;
      case 'N':
        movePiece(opponentKnights, 0, lastMove.capturedPiece);
        break;
      case 'B':
        movePiece(opponentBishops, 0, lastMove.capturedPiece);
        break;
      case 'R':
        movePiece(opponentRooks, 0, lastMove.capturedPiece);
        break;
      case 'Q':
        movePiece(opponentQueens, 0, lastMove.capturedPiece);
        break;
      case 'K':
        movePiece(opponentKing, 0, lastMove.capturedPiece);
        break;
    }
  }

  // Handle en passant
  if (lastMove.wasEnPassant) {
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

CBoard::GameStatus CBoard::isCheckmate() {
  bool isWhiteTurn = whiteToMove();
  bool kingInCheck = isWhiteTurn ? !wKingSafe(wKing) : !bKingSafe(bKing);
  bool hasLegalMoves = isWhiteTurn ? legalMoves(white()) : legalMoves(black());

  if (kingInCheck)
    return hasLegalMoves ? GameStatus::InProgress : (isWhiteTurn ? GameStatus::BlackWon : GameStatus::WhiteWon);
  return hasLegalMoves ? GameStatus::InProgress : GameStatus::Draw;
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
  score += evaluateMobility(wPawns);
  score -= evaluateMobility(bPawns);

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

int CBoard::evaluateMobility(Bitboard onMove) {
  int mobilityScore = 0;


  Bitboard moveFrom = 1ULL;
  while (moveFrom) {
    mobilityScore += popcount(legalMoves(moveFrom & onMove));
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
    if (moveFrom & pieces)
      score += table[index];

    moveFrom <<= 1;
    index++;
  }
  return score;
}

std::pair<int, std::pair<Bitboard, Bitboard>> CBoard::negamax(int depth) {
  if (depth == 0) {
    return {evaluate(), {0, 0}}; // Return evaluation and a dummy move
  }

  std::cout << "we are at depth: " << depth << std::endl;

  int maxEval = INT_MIN;
  std::pair<Bitboard, Bitboard> bestMove = {0, 0};

  Bitboard positions = onMovePositions();

  while (positions) {
    Bitboard moveFrom = positions & -positions;
    positions &= positions - 1;

    auto moves = generateMoves(moveFrom);
    for (const auto &move : moves) {
      makeMove(move.first, move.second);
      int eval = -negamax(depth - 1).first;
      unmakeMove();

      if (eval > maxEval) {
        maxEval = eval;
        bestMove = move;
      }
    }
  }

  return {maxEval, bestMove};
}
