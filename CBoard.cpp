//
// Created by Petr Smerda on 01.07.2024.
//

#include "CBoard.h"


CBoard::CBoard() {
  // Place pieces on initial positions
  wKing = 0x10ULL;
  bKing = 0x1000000000000000ULL;

  wPawns = 0xFF00ULL;
  bPawns = 0xFF000000000000ULL;

  wKnights = 0x42ULL;
  bKnights = 0x4200000000000000ULL;

  wBishops = 0x24ULL;
  bBishops = 0x2400000000000000ULL;

  wRooks = 0x81ULL;
  bRooks = 0x8100000000000000ULL;

  wQueens = 0x8ULL;
  bQueens = 0x800000000000000ULL;

  wCastling = 0x44ULL;
  bCastling = 0x4400000000000000ULL;

  enPassant = 0;


  onTurn = 1;
}


bool CBoard::loadTextures(const std::string texturePath[12]) const {

  for (int i = 0; i < 12; ++i) {
    if (!m_textures[i].loadFromFile(texturePath[i])) {
      std::cerr << "Failed to load texture: " << texturePath[i] << std::endl;
      return false;
    }

    // Assign and scale the textures to sprites
    m_sprites[i].setTexture(m_textures[i]);
    sf::Vector2u textureSize = m_textures[i].getSize();

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

  return true;
}


void CBoard::draw(sf::RenderWindow &window, Bitboard moveFrom) {

  // If white won draw a white screen if black won, draw a black screen
//  if ()


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
    int pos = __builtin_ctzll(moveFrom); // Extracting the set bit to draw it on the board

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


Bitboard CBoard::white() const { return wPawns | wKnights | wBishops | wRooks | wQueens | wKing; }

Bitboard CBoard::black() const { return bPawns | bKnights | bBishops | bRooks | bQueens | bKing; }

bool CBoard::whiteToMove() const { return onTurn == 1; }

bool CBoard::blackToMove() const { return onTurn == -1; }

Bitboard CBoard::empty() const { return ~white() & ~black(); }


bool CBoard::isEndgame() const {
  int whiteMaterial = countMaterial(true);
  int blackMaterial = countMaterial(false);

  // Threshold for endgame (simplified heuristic)
  const int endgameThreshold = 15;  // Example value, can be tuned

  // Consider the game in the endgame if both players have low material
  return (whiteMaterial <= endgameThreshold && blackMaterial <= endgameThreshold);
}

int CBoard::countMaterial(bool isWhite) const {
  int material = 0;

  // Define the material value of each piece
  const int pawnValue = 1;
  const int knightValue = 3;
  const int bishopValue = 3;
  const int rookValue = 5;
  const int queenValue = 9;

  if (isWhite) {
    material += __builtin_popcountll(wPawns) * pawnValue;
    material += __builtin_popcountll(wKnights) * knightValue;
    material += __builtin_popcountll(wBishops) * bishopValue;
    material += __builtin_popcountll(wRooks) * rookValue;
    material += __builtin_popcountll(wQueens) * queenValue;
  } else {
    material += __builtin_popcountll(bPawns) * pawnValue;
    material += __builtin_popcountll(bKnights) * knightValue;
    material += __builtin_popcountll(bBishops) * bishopValue;
    material += __builtin_popcountll(bRooks) * rookValue;
    material += __builtin_popcountll(bQueens) * queenValue;
  }

  return material;
}


bool CBoard::movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo) {
  pieces &= ~moveFrom;  // Clear the source bit
  pieces |= moveTo;     // Set the destination bit
  return true;
}

void CBoard::handleCapture(Bitboard moveTo, MoveInfo &moveInfo) {
  if (whiteToMove()) removeCapturedBlack(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
  else removeCapturedWhite(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
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

Bitboard CBoard::wSinglePush(Bitboard pawns) const { return nortOne(pawns) & empty(); }

Bitboard CBoard::bSinglePush(Bitboard pawns) const { return soutOne(pawns) & empty(); }

Bitboard CBoard::wDoublePush(Bitboard pawns) const { return nortOne(wSinglePush(pawns)) & empty() & RANK_4; }

Bitboard CBoard::bDoublePush(Bitboard pawns) const { return soutOne(bSinglePush(pawns)) & empty() & RANK_5; }

Bitboard CBoard::wPawnMoves(Bitboard pos) const {
  return wSinglePush(pos) | wDoublePush(pos) |
         ((wPawnWestAttacks(pos) | wPawnEastAttacks(pos)) & (black() | enPassant));
}

Bitboard CBoard::bPawnMoves(Bitboard pos) const {
  return bSinglePush(pos) | bDoublePush(pos) |
         ((bPawnWestAttacks(pos) | bPawnEastAttacks(pos)) & (white() | enPassant));
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

bool CBoard::castleRights(bool isWhite) const {
  if (isWhite) {
    // White King-side Castling
    if (!(wKingMoved || wRookMovedKingSide)) {
      // Check that the squares between the king and rook are empty and the king is not in check
      if ((empty() & (1ULL << F1) & (1ULL << G1)) &&
          wKingSafe(1ULL << E1) && wKingSafe(1ULL << F1) && wKingSafe(1ULL << G1)) {
        return true;  // King-side castling possible
      }
    }

    // White Queen-side Castling
    if (!(wKingMoved || wRookMovedQueenSide)) {
      if ((empty() & (1ULL << D1) & (1ULL << C1) & (1ULL << B1)) &&
          wKingSafe(1ULL << E1) && wKingSafe(1ULL << D1) && wKingSafe(1ULL << C1)) {
        return true;  // Queen-side castling possible
      }
    }
  } else {
    // Black King-side Castling
    if (!(bKingMoved || bRookMovedKingSide)) {
      if ((empty() & (1ULL << F8) & (1ULL << G8)) &&
          bKingSafe(1ULL << E8) && bKingSafe(1ULL << F8) && bKingSafe(1ULL << G8)) {
        return true;  // King-side castling possible
      }
    }

    // Black Queen-side Castling
    if (!(bKingMoved || bRookMovedQueenSide)) {
      if ((empty() & (1ULL << D8) & (1ULL << C8) & (1ULL << B8)) &&
          bKingSafe(1ULL << E8) && bKingSafe(1ULL << D8) && bKingSafe(1ULL << C8)) {
        return true;  // Queen-side castling possible
      }
    }
  }

  return false;
}


Bitboard CBoard::oneAround(Bitboard pos) {
  return nortOne(pos) | soutOne(pos) | eastOne(pos) | westOne(pos) | noWe(pos) | noEa(pos) | soWe(pos) | soEa(pos);
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

// Function to calculate white king's legal moves, including castling
Bitboard CBoard::wKingMoves(Bitboard pos) const {
  Bitboard safeMoves = wKingSafe(oneAround(pos)) & enemyOrEmpty<true>(); // King can move to empty or enemy-occupied safe squares
  Bitboard castling = 0;

  // King-side castling check
  if ((pos & wKing) && (wCastling & pos) && (empty() & (pos >> 1)) && (empty() & (pos >> 2)) &&
      wKingSafe(pos) && wKingSafe(pos >> 1) && wKingSafe(pos >> 2)) {
    castling |= wCastling & (pos >> 2); // King-side castling
  }

  // Queen-side castling check
  if ((pos & wKing) && (wCastling & pos) && (empty() & (pos << 1)) && (empty() & (pos << 2)) &&
      empty() & (pos << 3) && wKingSafe(pos) && wKingSafe(pos << 1) && wKingSafe(pos << 2)) {
    castling |= wCastling & (pos << 2); // Queen-side castling
  }

  return safeMoves | castling;
}

// Function to calculate black king's legal moves, including castling
Bitboard CBoard::bKingMoves(Bitboard pos) const {
  Bitboard safeMoves = bKingSafe(oneAround(pos)) & enemyOrEmpty<false>(); // King can move to empty or enemy-occupied safe squares
  Bitboard castling = 0;

  // King-side castling check
  if ((pos & bKing) && (bCastling & pos) && (empty() & (pos >> 1)) && (empty() & (pos >> 2)) &&
      bKingSafe(pos) && bKingSafe(pos >> 1) && bKingSafe(pos >> 2)) {
    castling |= bCastling & (pos >> 2); // King-side castling
  }

  // Queen-side castling check
  if ((pos & bKing) && (bCastling & pos) && (empty() & (pos << 1)) && (empty() & (pos << 2)) &&
      empty() & (pos << 3) && bKingSafe(pos) && bKingSafe(pos << 1) && bKingSafe(pos << 2)) {
    castling |= bCastling & (pos << 2); // Queen-side castling
  }

  return safeMoves | castling;
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

  for (auto moveFrom: CBitboardRange(pos)) {

    Bitboard possibleMoves = pseudoLegalMoves(moveFrom);

    // Appending all the moves from possibleMoves that are legal to result
    for (auto moveTo: CBitboardRange(possibleMoves))
      if (isMoveLegal(moveFrom, moveTo))
        legalMoves |= moveTo;
  }

  return legalMoves;
}


bool CBoard::isMoveLegal(Bitboard from, Bitboard to) {
  // If we cannot make the move we don't want to unmake it, so return false and the move is not legal
  if (!makeMove(from, to))
    return false;

  bool isValid = whiteToMove() ? bKingSafe(bKing) : wKingSafe(wKing);
  unmakeMove();

  return isValid;
}


std::vector<std::pair<Bitboard, Bitboard>> CBoard::generateMoves(Bitboard moveFrom) {
  // in moveFrom must be just one bit set
  if ((moveFrom & (moveFrom - 1)) != 0)
    throw std::invalid_argument("expected 1 bit set, got more");

  std::vector<std::pair<Bitboard, Bitboard>> moves;
  Bitboard possibleMoves = legalMoves(moveFrom);

  for (auto moveTo: CBitboardRange(possibleMoves))
    moves.emplace_back(moveFrom, moveTo);


  return moves;
}


/*
 ************************************************************
 *                                                          *
 *               Move making and Unmaking                   *
 *               Move making and Unmaking                   *
 *                                                          *
 ************************************************************
 */



bool CBoard::movePieceIfValid(Bitboard &pieceSet, Bitboard moveFrom, Bitboard moveTo) {
  if (pieceSet & moveFrom)
    return movePiece(pieceSet, moveFrom, moveTo);

  return false;
}

char CBoard::showPromotionWindow() {
  sf::RenderWindow promotionWindow(sf::VideoMode(400, 100), "Pawn Promotion");

  sf::Font font;
  if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
    std::cerr << "Error loading font\n";
    std::cout << "font cannot be loaded" << std::endl;
    return 'Q';  // Default to Queen if font fails to load
  }

  // Creating text objects for promotion options
  sf::Text queenText("Q - Queen", font, 20);
  sf::Text rookText("R - Rook", font, 20);
  sf::Text bishopText("B - Bishop", font, 20);
  sf::Text knightText("N - Knight", font, 20);

  queenText.setPosition(20, 5);
  rookText.setPosition(20, 25);
  bishopText.setPosition(20, 45);
  knightText.setPosition(20, 65);

  queenText.setFillColor(sf::Color::Black);
  rookText.setFillColor(sf::Color::Black);
  bishopText.setFillColor(sf::Color::Black);
  knightText.setFillColor(sf::Color::Black);

  char chosenPiece = 'Q';  // Default promotion to Queen

  while (promotionWindow.isOpen()) {
    sf::Event event = sf::Event();
    while (promotionWindow.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        promotionWindow.close();

      if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
          case sf::Keyboard::Q:
            chosenPiece = 'Q';
            promotionWindow.close();
            break;
          case sf::Keyboard::R:
            chosenPiece = 'R';
            promotionWindow.close();
            break;
          case sf::Keyboard::B:
            chosenPiece = 'B';
            promotionWindow.close();
            break;
          case sf::Keyboard::N:
            chosenPiece = 'N';
            promotionWindow.close();
            break;
          default:
            break;
        }
      }
    }

    promotionWindow.clear(sf::Color::White);
    promotionWindow.draw(queenText);
    promotionWindow.draw(rookText);
    promotionWindow.draw(bishopText);
    promotionWindow.draw(knightText);
    promotionWindow.display();
  }

  return chosenPiece;
}


Bitboard CBoard::isPromotion() const { return (bPawns | wPawns) & (RANK_8 | RANK_1); }


void CBoard::handlePromotion(char promotedPiece) {
  // Determine if a white pawn or black pawn is promoting
  bool isWhitePromotion = (wPawns & RANK_8);
  bool isBlackPromotion = (bPawns & RANK_1);

  if (!isWhitePromotion && !isBlackPromotion) {
    // No promotion is happening; exit early
    return;
  }

  // Reference to the appropriate pawn and piece bitboards
  Bitboard &pawns = isWhitePromotion ? wPawns : bPawns;
  Bitboard *promotedTo = nullptr;  // Pointer to store which piece bitboard will be modified

  switch (promotedPiece) {
    case 'Q':
      promotedTo = isWhitePromotion ? &wQueens : &bQueens;
      *promotedTo |= pawns & (isWhitePromotion ? RANK_8 : RANK_1);
      break;
    case 'R':
      promotedTo = isWhitePromotion ? &wRooks : &bRooks;
      *promotedTo |= pawns & (isWhitePromotion ? RANK_8 : RANK_1);
      break;
    case 'B':
      promotedTo = isWhitePromotion ? &wBishops : &bBishops;
      *promotedTo |= pawns & (isWhitePromotion ? RANK_8 : RANK_1);
      break;
    case 'N':
      promotedTo = isWhitePromotion ? &wKnights : &bKnights;
      *promotedTo |= pawns & (isWhitePromotion ? RANK_8 : RANK_1);
      break;
    default:
      throw std::invalid_argument("Invalid promotion piece");
  }

  // Set the move list to indicate which piece was promoted to
  m_moveList.top().promotedTo = promotedTo;

  // Mark that this was a promotion move
  m_moveList.top().wasPromotion = true;

  // Remove the pawn from the promotion rank
  pawns &= ~(isWhitePromotion ? RANK_8 : RANK_1);
}



bool CBoard::handlePawnMove(Bitboard &pawns, Bitboard moveFrom, Bitboard moveTo,
                            bool &enPassantSet, MoveInfo &moveInfo) {

  if (!(pawns & moveFrom)) return false;

  movePiece(pawns, moveFrom, moveTo);

  if (moveTo & enPassant) {
    // En-passant capture
    removeCapturedBlack(whiteToMove() ? soutOne(moveTo) : nortOne(moveTo), moveInfo.capturedPiece,
                        moveInfo.capturedPieceType);
  } else if (whiteToMove() ? (moveTo & nortTwo(moveFrom)) : (moveTo & soutTwo(moveFrom))) {
    // Set en-passant possibility
    enPassant = whiteToMove() ? nortOne(moveFrom) : soutOne(moveFrom);
    enPassantSet = true;
  }

  return true;
}


bool CBoard::handleRookMove(Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo, Bitboard &castlingRights) {
  if (!(rooks & moveFrom)) return false;

  movePiece(rooks, moveFrom, moveTo);
  // Disable castling rights on the side the rook moved
  castlingRights &= eastTwo(rooks) | westOne(rooks);

  return true;
}

bool CBoard::handleKingMove(Bitboard &king, Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo,
                            Bitboard &castlingRights) const {
  if (!(king & moveFrom)) return false;

  movePiece(king, moveFrom, moveTo);

  // Handle castling
  if (king & castlingRights) {
    Bitboard rookFrom = (moveTo & (whiteToMove() ? (1ULL << 2) : (1ULL << 58))) ? 1ULL : (whiteToMove() ? (1ULL << 7)
                                                                                                        : (1ULL << 63));
    Bitboard rookTo = (rookFrom & 1ULL) ? (whiteToMove() ? (1ULL << 3) : (1ULL << 59)) : (whiteToMove() ? (1ULL << 5)
                                                                                                        : (1ULL << 61));
    movePiece(rooks, rookFrom, rookTo);
  }

  // Disable castling rights if the king moves
  castlingRights = 0;

  return true;
}


bool CBoard::makeMove(const Bitboard moveFrom, const Bitboard moveTo) {
  Bitboard pseudoMoves = pseudoLegalMoves(moveFrom);

  if (!(moveTo & pseudoMoves))
    return false;

  // Must store the info before the move
  MoveInfo moveInfo = {moveFrom, moveTo, 0, enPassant, onTurn == 1 ? wCastling : bCastling, onTurn, false, 0, 0,
                       nullptr};

  bool isWhite = whiteToMove();
  bool enPassantSet = false;

  Bitboard &knights = isWhite ? wKnights : bKnights;
  Bitboard &bishops = isWhite ? wBishops : bBishops;
  Bitboard &queens = isWhite ? wQueens : bQueens;
  Bitboard &pawns = isWhite ? wPawns : bPawns;
  Bitboard &rooks = isWhite ? wRooks : bRooks;
  Bitboard &king = isWhite ? wKing : bKing;
  Bitboard &castling = isWhite ? wCastling : bCastling;

  // Move piece
  if (movePieceIfValid(knights, moveFrom, moveTo) || movePieceIfValid(bishops, moveFrom, moveTo) ||
      movePieceIfValid(queens, moveFrom, moveTo) ||
      handlePawnMove(pawns, moveFrom, moveTo, enPassantSet, moveInfo) ||
      handleRookMove(rooks, moveFrom, moveTo, castling) ||
      handleKingMove(king, rooks, moveFrom, moveTo, castling)) {

    handleCapture(moveTo, moveInfo);

    if (!enPassantSet)
      enPassant = 0;

    onTurn *= -1;

    moveInfo.wasPromotion = isPromotion();

    m_moveList.push(moveInfo);

    return true;
  }

  return false;
}

bool CBoard::unmakeMove() {
  if (m_moveList.empty()) return false;

  MoveInfo lastMove = m_moveList.top();
  m_moveList.pop();

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

  // Handle the promotion unmaking
  if (lastMove.wasPromotion) {
    // Remove the promoted piece from its bitboard
    if (lastMove.promotedTo)
      *lastMove.promotedTo &= ~lastMove.moveTo;  // Remove promoted piece from its final square


    // Restore the pawn to its original position
    pawns |= lastMove.moveFrom;
  }

  // Unmake the move
  if (!unmakePieceMove(pawns, lastMove) && !unmakePieceMove(knights, lastMove) &&
      !unmakePieceMove(bishops, lastMove) && !unmakePieceMove(rooks, lastMove) &&
      !unmakePieceMove(queens, lastMove) && !unmakePieceMove(king, lastMove))
    return false;



  // Handle castling
  if (king & lastMove.moveFrom)
    unmakeCastlingMove(rooks, lastMove);


  // Restore the captured piece, if any
  if (lastMove.capturedPiece)
    restoreCapturedPiece(lastMove, opponentPawns, opponentKnights, opponentBishops,
                         opponentRooks, opponentQueens, opponentKing);


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

bool CBoard::unmakePieceMove(Bitboard &pieceSet, const MoveInfo &lastMove) {
  if (pieceSet & lastMove.moveTo) {
    movePiece(pieceSet, lastMove.moveTo, lastMove.moveFrom);
    return true;
  }
  return false;
}

void CBoard::unmakeCastlingMove(Bitboard &rooks, const MoveInfo &lastMove) {
  if (lastMove.moveTo == lastMove.moveFrom << 2) {
    // King-side castling
    movePiece(rooks, lastMove.moveFrom << 1, lastMove.moveFrom << 3);
  } else if (lastMove.moveTo == lastMove.moveFrom >> 2) {
    // Queen-side castling
    movePiece(rooks, lastMove.moveFrom >> 1, lastMove.moveFrom >> 4);
  }
}

void CBoard::restoreCapturedPiece(const MoveInfo &lastMove, Bitboard &opponentPawns, Bitboard &opponentKnights,
                                  Bitboard &opponentBishops, Bitboard &opponentRooks, Bitboard &opponentQueens,
                                  Bitboard &opponentKing) {
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


Bitboard CBoard::onMovePositions() const {
  return onTurn == 1 ? white() : black();
}

/*
 ************************************************************
 *                                                          *
 *                 Evaluation Methods                       *
 *                 Evaluation Methods                       *
 *                                                          *
 ************************************************************
 */

int CBoard::materialEvaluation() const {
  int materialScore = 0;

  // Piece values (you can tweak these for better performance)
  constexpr int pawnValue = 100;
  constexpr int knightValue = 320;
  constexpr int bishopValue = 330;
  constexpr int rookValue = 500;
  constexpr int queenValue = 900;

  // White piece material
  materialScore += __builtin_popcountll(wPawns) * pawnValue;
  materialScore += __builtin_popcountll(wKnights) * knightValue;
  materialScore += __builtin_popcountll(wBishops) * bishopValue;
  materialScore += __builtin_popcountll(wRooks) * rookValue;
  materialScore += __builtin_popcountll(wQueens) * queenValue;

  // Black piece material (subtract from white score)
  materialScore -= __builtin_popcountll(bPawns) * pawnValue;
  materialScore -= __builtin_popcountll(bKnights) * knightValue;
  materialScore -= __builtin_popcountll(bBishops) * bishopValue;
  materialScore -= __builtin_popcountll(bRooks) * rookValue;
  materialScore -= __builtin_popcountll(bQueens) * queenValue;

  return materialScore;
}


int CBoard::positionalEvaluation() const {
  int positionalScore = 0;

  for (int i = 0; i < 64; ++i) {
    // White pawns
    if (wPawns & (1ULL << i)) {
      positionalScore += pawnTable[i];
    }

    // Black pawns (adjust for black's perspective, as they start on the opposite side)
    if (bPawns & (1ULL << i)) {
      positionalScore -= pawnTable[63 - i];
    }

    // Similar logic can be applied for knights, bishops, etc.
  }

  return positionalScore;
}



int CBoard::kingSafetyEvaluation() const {
  int kingSafetyScore = 0;

  // King safety logic for white
  if (wKing & castleRights(true)) {
    // King castled safely (bonus)
    kingSafetyScore += 50;
  } else if (oneAround(wKing) & empty()) {
    // Exposed king penalty
    kingSafetyScore -= 50;
  }

  // King safety logic for black
  if (bKing & castleRights(false)) {
    kingSafetyScore -= 50; // Black castling bonus (penalized from white's perspective)
  } else if (oneAround(bKing) & empty()) {
    kingSafetyScore += 50; // Exposed black king penalty
  }

  return kingSafetyScore;
}

int CBoard::pawnStructureEvaluation() const {
  int pawnStructureScore = 0;

  // Isolated pawns for white
  Bitboard isolatedPawns = wPawns & ~(wPawns >> 1) & ~(wPawns << 1);
  pawnStructureScore -= __builtin_popcountll(isolatedPawns) * 20;

  // Doubled pawns for white
  Bitboard doubledPawns = wPawns & (wPawns >> 8);
  pawnStructureScore -= __builtin_popcountll(doubledPawns) * 15;

  // Similar logic for black's pawns
  isolatedPawns = bPawns & ~(bPawns >> 1) & ~(bPawns << 1);
  pawnStructureScore += __builtin_popcountll(isolatedPawns) * 20;

  doubledPawns = bPawns & (bPawns >> 8);
  pawnStructureScore += __builtin_popcountll(doubledPawns) * 15;

  return pawnStructureScore;
}

int CBoard::mobilityEvaluation() {
  int mobilityScore = 0;

  // White mobility
  for (auto piece : CBitboardRange(white())) {
    mobilityScore += __builtin_popcountll(legalMoves(piece));
  }

  // Black mobility
  for (auto piece : CBitboardRange(black())) {
    mobilityScore -= __builtin_popcountll(legalMoves(piece));
  }

  return mobilityScore;
}


int CBoard::evaluate() {
  int score = 0;

  // Material evaluation (typically most important)
  score += materialEvaluation();

  // Positional evaluation (piece-square tables)
  score += positionalEvaluation();

  // King safety
  score += kingSafetyEvaluation();

  // Pawn structure
  score += pawnStructureEvaluation();

  // Mobility (only if in the middlegame or earlier)
  if (!isEndgame()) {
    score += mobilityEvaluation();
  }

  // Return score relative to whose turn it is
  return whiteToMove() ? score : -score;
}

int CBoard::popcount(Bitboard bb) { return __builtin_popcountll(bb); }


/*
 ************************************************************
 *                                                          *
 *                        AI Methods                        *
 *                        AI Methods                        *
 *                                                          *
 ************************************************************
 */


std::pair<int, std::pair<Bitboard, Bitboard>> CBoard::negamax(int depth) {
  if (depth < 0)
    return {evaluate(), {0, 0}}; // Return evaluation and a dummy move


  int maxEval = INT_MIN;
  std::pair<Bitboard, Bitboard> bestMove = {0, 0};

  Bitboard positions = onMovePositions();

  while (positions) {
    Bitboard moveFrom = positions & -positions;
    positions &= positions - 1;

    auto moves = generateMoves(moveFrom);
    for (const auto &move: moves) {
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
