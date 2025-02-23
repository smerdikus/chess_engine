//
// Created by Petr Smerda on 01.07.2024.
//

#include "CBoard.h"

using namespace chs;

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

void CBoard::initPos(Board board) {
  // Positioning pieces based on input structure
  wKing = board.wKing;
  bKing = board.bKing;

  wPawns = board.wPawns;
  bPawns = board.bPawns;

  wKnights = board.wKnights;
  bKnights = board.bKnights;

  wBishops = board.wBishops;
  bBishops = board.bBishops;

  wRooks = board.wRooks;
  bRooks = board.bRooks;

  wQueens = board.wQueens;
  bQueens = board.bQueens;

  wCastling = board.wCastling;
  bCastling = board.bCastling;

  enPassant = board.enPassant;

  onTurn = board.onTurn;
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


void CBoard::draw(sf::RenderWindow &window, Bitboard moveFrom, sf::Font &font) {
  // Drawing the board and squares
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      m_rectangle.setPosition(static_cast<float>(x * TILE + BORDER), static_cast<float>(y * TILE + BORDER));
      m_rectangle.setFillColor((x + y) % 2 == 0 ? lightSquareColor : darkSquareColor);
      window.draw(m_rectangle);
    }
  }

  if (moveFrom) {
    int pos = __builtin_ctzll(moveFrom); // Extracting the set bit to draw it on the board

    int x = pos % 8;
    int y = 7 - (pos / 8);

    m_rectangle.setPosition(static_cast<float>(x * TILE + BORDER), static_cast<float>(y * TILE + BORDER));
    m_rectangle.setFillColor(highlightSrcColor);
    window.draw(m_rectangle);

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
  Bitboard pieces[12] = {
          wPawns, wKing, wKnights, wBishops, wQueens, wRooks,
          bPawns, bKing, bKnights, bBishops, bQueens, bRooks
  };

  for (size_t i = 0; i < 12; ++i)
    posFromBitboard(m_sprites[i], pieces[i]);


  // Drawing the rectangle based on who is winning based on the eval function
  sf::RectangleShape winningRect(sf::Vector2f(4 * TILE + evaluate() * onTurn / 3, 30));

  winningRect.setPosition(0, 8 * TILE);
  winningRect.setFillColor(sf::Color::White);

  window.draw(winningRect);

  // Create the text, set its value, font, character size and color
  sf::Text text;
  text.setFont(font);
  text.setString(std::to_string(evaluate() * onTurn)); // Convert the number to a string
  text.setCharacterSize(24); // in pixels
  text.setFillColor(sf::Color(130, 130, 160, 255));

  // Get the local bounds of the text and rectangle, then set the position of the text according to these bounds
  text.setPosition((WIDTH - text.getLocalBounds().width) / 2, TILE * 8);

  // Draw the text
  window.draw(text);
}


Bitboard CBoard::white() const { return wPawns | wKnights | wBishops | wRooks | wQueens | wKing; }

Bitboard CBoard::black() const { return bPawns | bKnights | bBishops | bRooks | bQueens | bKing; }

bool CBoard::whiteToMove() const { return onTurn == 1; }

bool CBoard::blackToMove() const { return onTurn == -1; }

Bitboard CBoard::empty() const { return ~white() & ~black(); }


bool CBoard::isEndgame(int threshold = 12) const {
  // Return the count of not empty squares
  return (popcount(~empty()) <= threshold);
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


Bitboard CBoard::wKingSafe(Bitboard pos) const {
  Bitboard blackAttacks = 0;

  // Pawn must be just attacking
  blackAttacks |= bPawnEastAttacks(bPawns);
  blackAttacks |= bPawnWestAttacks(bPawns);
  blackAttacks |= bKnightMoves(bKnights, enemyOrEmpty<false>());
  blackAttacks |= bBishopMoves(bBishops, white(), empty());
  blackAttacks |= bRookMoves(bRooks, white(), empty());
  blackAttacks |= bQueenMoves(bQueens, white(), empty());
  blackAttacks |= oneAround(bKing);

  return pos & ~blackAttacks; // Return squares not attacked by white and bKing
}

Bitboard CBoard::bKingSafe(Bitboard pos) const {
  Bitboard whiteAttacks = 0;

  // Pawns must be just attacking
  whiteAttacks |= wPawnEastAttacks(wPawns);
  whiteAttacks |= wPawnWestAttacks(wPawns);
  whiteAttacks |= wKnightMoves(wKnights, enemyOrEmpty<true>());
  whiteAttacks |= wBishopMoves(wBishops, black(), empty());
  whiteAttacks |= wRookMoves(wRooks, black(), empty());
  whiteAttacks |= wQueenMoves(wQueens, black(), empty());
  whiteAttacks |= oneAround(wKing);

  return pos & ~whiteAttacks; // Return squares not attacked by white and bKing
}


Bitboard CBoard::wKingMoves(Bitboard pos) const {
  // King can move to empty or enemy-occupied safe squares
  Bitboard safeMoves = wKingSafe(oneAround(pos)) & enemyOrEmpty<true>();

  // King-side castling
  Bitboard kingSide = ((wKingSafe(pos) & wKingSafe(pos >> 1) & wKingSafe(pos >> 2)) *
                       (empty() & (pos >> 1) & (pos >> 2))) >> 2;

  // Queen-side castling
  Bitboard queenSide = ((wKingSafe(pos) & wKingSafe(pos << 1) & wKingSafe(pos << 2)) *
                        (empty() & (pos << 1) & (pos << 2) & (pos << 3))) << 2;

  return (safeMoves | kingSide | queenSide) & wCastling;
}


Bitboard CBoard::bKingMoves(Bitboard pos) const {
  // King can move to empty or enemy-occupied safe squares
  Bitboard safeMoves = bKingSafe(oneAround(pos)) & enemyOrEmpty<false>();

  // King-side castling
  Bitboard kingSide = ((bKingSafe(pos) & bKingSafe(pos >> 1) & bKingSafe(pos >> 2)) *
                       (empty() & (pos >> 1) & (pos >> 2))) >> 2;

  // Queen-side castling
  Bitboard queenSide = ((bKingSafe(pos) & bKingSafe(pos << 1) & bKingSafe(pos << 2)) *
                        (empty() & (pos << 1) & (pos << 2) & (pos << 3))) << 2;

  return (safeMoves | kingSide | queenSide) & bCastling;
}


Bitboard CBoard::pseudoLegalMoves(Bitboard pos) const {
  Bitboard pseudoLegalMoves = 0;

  pseudoLegalMoves |= wPawnMoves(pos & wPawns, black(), empty(), enPassant);
  pseudoLegalMoves |= wKnightMoves(pos & wKnights, enemyOrEmpty<true>());
  pseudoLegalMoves |= wBishopMoves(pos & wBishops, black(), empty());
  pseudoLegalMoves |= wRookMoves(pos & wRooks, black(), empty());
  pseudoLegalMoves |= wQueenMoves(pos & wQueens, black(), empty());
  pseudoLegalMoves |= wKingMoves(pos & wKing);

  pseudoLegalMoves |= bPawnMoves(pos & bPawns, white(), empty(), enPassant);
  pseudoLegalMoves |= bKnightMoves(pos & bKnights, enemyOrEmpty<false>());
  pseudoLegalMoves |= bBishopMoves(pos & bBishops, white(), empty());
  pseudoLegalMoves |= bRookMoves(pos & bRooks, white(), empty());
  pseudoLegalMoves |= bQueenMoves(pos & bQueens, white(), empty());
  pseudoLegalMoves |= bKingMoves(pos & bKing);

  return pseudoLegalMoves;
}


Bitboard CBoard::legalMoves(Bitboard pos) {
  Bitboard legalMoves = 0;

  for (auto moveFrom: CBitboardRange(pos)) {
    Bitboard possibleMoves = pseudoLegalMoves(moveFrom);

    // Adding all the moves from possibleMoves that are legal to result
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


bool CBoard::movePieceIfValid(Bitboard &pieceSet, Bitboard moveFrom, Bitboard moveTo) {
  if (pieceSet & moveFrom)
    return movePiece(pieceSet, moveFrom, moveTo);

  return false;
}

char CBoard::showPromotionWindow(sf::Font &font) {
  sf::RenderWindow promotionWindow(sf::VideoMode(150, 100), "Pawn Promotion");


  // Creating text objects for promotion options
  std::string texts[4] = {"Q - Queen", "R - Rook", "B - Bishop", "N - Knight"};

  sf::Text text("", font, 20);

  text.setPosition(20, 5);
  text.setFillColor(sf::Color::Black);

  char chosenPiece = '\0';

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
    for (int i = 0; i < 4; i++) {
      text.setString(texts[i]);
      text.setPosition(sf::Vector2f(20, i * 20 + 5));
      promotionWindow.draw(text);
    }
    promotionWindow.display();
  }

  if (chosenPiece == '\0')
    throw std::out_of_range("Unknown piece to promote: " + std::string(1, chosenPiece));

  return chosenPiece;
}

Bitboard CBoard::isWPromotion() const { return wPawns & RANK_8; }

Bitboard CBoard::isBPromotion() const { return bPawns & RANK_1; }

void CBoard::handlePromotion(char promotedPiece) {
  // No promotion is happening; exit early
  if (!isWPromotion() && !isBPromotion())
    return;

  // Reference to the appropriate pawn and piece bitboards
  Bitboard &pawns = isWPromotion() ? wPawns : bPawns;
  Bitboard *promotedTo = nullptr;  // Pointer to store which piece bitboard will be modified

  switch (promotedPiece) {
    case 'Q':
      promotedTo = isWPromotion() ? &wQueens : &bQueens;
      break;
    case 'R':
      promotedTo = isWPromotion() ? &wRooks : &bRooks;
      break;
    case 'B':
      promotedTo = isWPromotion() ? &wBishops : &bBishops;
      break;
    case 'N':
      promotedTo = isWPromotion() ? &wKnights : &bKnights;
      break;
    default:
      throw std::invalid_argument("Invalid promotion piece");
  }

  *promotedTo |= pawns & (isWPromotion() ? RANK_8 : RANK_1);

  // Set the move list to indicate which piece was promoted to
  m_moveList.top().promotedTo = promotedTo;

  // Mark that this was a promotion move
  m_moveList.top().wasPromotion = true;

  // Remove the pawn from the promotion rank
  pawns &= ~(isWPromotion() ? RANK_8 : RANK_1);
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

  if (!(moveTo & pseudoMoves)) return false;

  // Must store the info before the move
  MoveInfo moveInfo = {moveFrom, moveTo, 0, enPassant, onTurn == 1 ? wCastling : bCastling,
                       onTurn, false, 0, 0, nullptr};

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

    moveInfo.wasPromotion = isWPromotion();

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
    // It is unmaking, so we are moving from MoveTo, to moveFrom
    movePiece(pieceSet, lastMove.moveTo, lastMove.moveFrom);
    return true;
  }
  return false;
}

void CBoard::unmakeCastlingMove(Bitboard &rooks, const MoveInfo &lastMove) {
  // King-side castling
  if (lastMove.moveTo == lastMove.moveFrom << 2)
    movePiece(rooks, lastMove.moveFrom << 1, lastMove.moveFrom << 3);

    // Queen-side castling
  else if (lastMove.moveTo == lastMove.moveFrom >> 2)
    movePiece(rooks, lastMove.moveFrom >> 1, lastMove.moveFrom >> 4);

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

Bitboard CBoard::onMovePositions() const { return onTurn == 1 ? white() : black(); }

int CBoard::materialEvaluation() const {
  int materialScore = 0;

  materialScore += popcount(wPawns) * PAWN_VALUE;
  materialScore += popcount(wKnights) * KNIGHT_VALUE;
  materialScore += popcount(wBishops) * BISHOP_VALUE;
  materialScore += popcount(wRooks) * ROOK_VALUE;
  materialScore += popcount(wQueens) * QUEEN_VALUE;

  materialScore -= popcount(bPawns) * PAWN_VALUE;
  materialScore -= popcount(bKnights) * KNIGHT_VALUE;
  materialScore -= popcount(bBishops) * BISHOP_VALUE;
  materialScore -= popcount(bRooks) * ROOK_VALUE;
  materialScore -= popcount(bQueens) * QUEEN_VALUE;

  return materialScore;
}


int CBoard::positionalEvaluation() const {
  int positionalScore = 0;

  auto applyPositionalScore = [&positionalScore](const uint64_t pieces, const int *table, int adjustment) {
    int mirrorOffset = (adjustment > 0) ? 0 : 63;
    for (uint64_t bb = pieces; bb; bb &= (bb - 1)) {
      int i = std::countr_zero(bb);
      positionalScore += adjustment * table[mirrorOffset ? mirrorOffset - i : i];
    }
  };

  const uint64_t whitePieces[6] = {wPawns, wKnights, wBishops, wRooks, wQueens, wKing};
  const uint64_t blackPieces[6] = {bPawns, bKnights, bBishops, bRooks, bQueens, bKing};
  const int *tables[6] = {pawnTable, knightTable, bishopTable, rookTable, queenTable, kingTable};

  for (int i = 0; i < 6; i++) applyPositionalScore(whitePieces[i], tables[i], 1);
  for (int i = 0; i < 6; i++) applyPositionalScore(blackPieces[i], tables[i], -1);


  return positionalScore;
}


int CBoard::kingSafetyEvaluation() const {
  int kingSafetyScore = 0;

  // Castling bonus
  kingSafetyScore += 20 * popcount(wCastling);
  kingSafetyScore -= 20 * popcount(bCastling);

  // Exposed king penalty
  kingSafetyScore -= 10 * popcount(oneAround(wKing) & empty());
  kingSafetyScore += 10 * popcount(oneAround(wKing) & empty());

  kingSafetyScore -= wKingSafe(wKing) ? 0 : 200;
  kingSafetyScore += bKingSafe(wKing) ? 0 : 200;

  return kingSafetyScore;
}

int CBoard::pawnStructureEvaluation() const {
  int pawnStructureScore = 0;

  // === ISOLATED PAWNS ===
  Bitboard wAdjacent = ((wPawns >> 1) & 0x7F7F7F7F7F7F7F7F) | ((wPawns << 1) & 0xFEFEFEFEFEFEFEFE);
  Bitboard wIsolated = wPawns & ~wAdjacent;
  pawnStructureScore -= popcount(wIsolated) * 10;

  Bitboard bAdjacent = ((bPawns >> 1) & 0x7F7F7F7F7F7F7F7F) | ((bPawns << 1) & 0xFEFEFEFEFEFEFEFE);
  Bitboard bIsolated = bPawns & ~bAdjacent;
  pawnStructureScore += popcount(bIsolated) * 10;


  // === DOUBLED PAWNS ===
  Bitboard wDoubled = wPawns & (wPawns >> 8);
  pawnStructureScore -= popcount(wDoubled) * 15;

  Bitboard bDoubled = bPawns & (bPawns >> 8);
  pawnStructureScore += popcount(bDoubled) * 15;


  // === SUPPORTED PAWNS ===
  Bitboard wSupported = wPawns & (((wPawns >> 9) & 0x7F7F7F7F7F7F7F7F) | ((wPawns >> 7) & 0xFEFEFEFEFEFEFEFE));
  Bitboard bSupported = bPawns & (((bPawns << 9) & 0xFEFEFEFEFEFEFEFE) | ((bPawns << 7) & 0x7F7F7F7F7F7F7F7F));

  pawnStructureScore += popcount(wSupported) * 10; // Reward for connected pawns
  pawnStructureScore -= popcount(bSupported) * 10;

  // === PAWN CHAINS ===
  Bitboard wPawnChains = wSupported & (((wPawns << 9) & 0xFEFEFEFEFEFEFEFE) | ((wPawns << 7) & 0x7F7F7F7F7F7F7F7F));
  Bitboard bPawnChains = bSupported & (((bPawns >> 9) & 0x7F7F7F7F7F7F7F7F) | ((bPawns >> 7) & 0xFEFEFEFEFEFEFEFE));

  pawnStructureScore += popcount(wPawnChains) * 15; // Extra reward for chains
  pawnStructureScore -= popcount(bPawnChains) * 15;

  // === BACKWARD PAWNS ===
  Bitboard wBackward = (wPawns & ~wSupported) & (bPawns >> 8);
  Bitboard bBackward = (bPawns & ~bSupported) & (wPawns << 8);

  pawnStructureScore -= popcount(wBackward) * 15; // Penalty for weak pawns
  pawnStructureScore += popcount(bBackward) * 15;

  return pawnStructureScore;
}

int CBoard::mobilityEvaluation() {
  int mobilityScore = 0;

  int backupOnTurn = onTurn;

  onTurn = 1;
  // White mobility
  for (auto piece: CBitboardRange(white()))
    mobilityScore += popcount(legalMoves(piece));

  onTurn = -1;
  // Black mobility
  for (auto piece: CBitboardRange(black()))
    mobilityScore -= popcount(legalMoves(piece));

  onTurn = backupOnTurn;

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
  if (!isEndgame())
    score += mobilityEvaluation();



  // Return score relative to whose turn it is
  return whiteToMove() ? score : -score;
}

int CBoard::popcount(Bitboard num) { return __builtin_popcountll(num); }

std::pair<int, std::pair<Bitboard, Bitboard>> CBoard::negamax(int depth) {
  if (depth <= 0)
    return {evaluate(), {0, 0}}; // Return evaluation and a dummy move

  int maxEval = INT_MIN;
  std::pair<Bitboard, Bitboard> bestMove = {0, 0};

  Bitboard positions = onMovePositions();
  if (!legalMoves(positions))
    return {maxEval, {0, 0}};

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
