//
// Created by Petr Smerda on 01.07.2024.
//

#include "CBoard.h"

namespace chs {

  CBoard::CBoard() {
    // Place pieces on initial positions
    m_brd = CBoard::getInitBoard();
  }

  void CBoard::initPos(Board board) {
    // Positioning pieces based on input structure
    m_brd = board;
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
    window.setSize(sf::Vector2u(WIDTH, HEIGHT));

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
            m_brd.wPawns, m_brd.wKing, m_brd.wKnights, m_brd.wBishops, m_brd.wQueens, m_brd.wRooks,
            m_brd.bPawns, m_brd.bKing, m_brd.bKnights, m_brd.bBishops, m_brd.bQueens, m_brd.bRooks
    };

    for (size_t i = 0; i < 12; ++i)
      posFromBitboard(m_sprites[i], pieces[i]);


    // Drawing the rectangle based on who is winning based on the eval function
    sf::RectangleShape winningRect(sf::Vector2f(4 * TILE + evaluate() * m_brd.onTurn / 3, 30));

    winningRect.setPosition(0, 8 * TILE);
    winningRect.setFillColor(sf::Color::White);

    window.draw(winningRect);

    // Create the text, set its value, font, character size and color
    sf::Text text;
    text.setFont(font);
    text.setString(std::to_string(evaluate() * m_brd.onTurn)); // Convert the number to a string
    text.setCharacterSize(24); // in pixels
    text.setFillColor(sf::Color(130, 130, 160, 255));

    // Get the local bounds of the text and rectangle, then set the position of the text according to these bounds
    text.setPosition((WIDTH - text.getLocalBounds().width) / 2, TILE * 8);

    // Draw the text
    window.draw(text);
  }

  Bitboard CBoard::onMovePositions() const {
    return m_brd.onMovePositions();
  }


  bool CBoard::movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo) {
    pieces &= ~moveFrom;
    pieces |= moveTo;
    return true;
  }

  void CBoard::handleCapture(Bitboard moveTo, MoveInfo &moveInfo) {
    if (m_brd.whiteToMove()) removeCapturedBlack(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
    else removeCapturedWhite(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
  }

  void CBoard::removeCapturedBlack(Bitboard moveTo, Bitboard &removedFrom, char &pieceType) {
    // Array of bitboards for black pieces and corresponding moveTo types
    Bitboard *blackPieces[] = {&m_brd.bPawns, &m_brd.bKnights, &m_brd.bBishops, &m_brd.bRooks, &m_brd.bQueens,
                               &m_brd.bKing};
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
    Bitboard *whitePieces[] = {&m_brd.wPawns, &m_brd.wKnights, &m_brd.wBishops, &m_brd.wRooks, &m_brd.wQueens,
                               &m_brd.wKing};
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


  Bitboard CBoard::wKingSafe(Board brd, Bitboard pos) {
    Bitboard blackAttacks = 0;

    // Pawn must be just attacking
    blackAttacks |= bPawnEastAttacks(brd.bPawns);
    blackAttacks |= bPawnWestAttacks(brd.bPawns);
    blackAttacks |= bKnightMoves(brd.bKnights, brd.enemyOrEmpty<false>());
    blackAttacks |= bBishopMoves(brd.bBishops, brd.white(), brd.empty());
    blackAttacks |= bRookMoves(brd.bRooks, brd.white(), brd.empty());
    blackAttacks |= bQueenMoves(brd.bQueens, brd.white(), brd.empty());
    blackAttacks |= oneAround(brd.bKing);

    return pos & ~blackAttacks; // Return squares not attacked by white and bKing
  }

  Bitboard CBoard::bKingSafe(Board brd, Bitboard pos) {
    Bitboard whiteAttacks = 0;

    // Pawns must be just attacking
    whiteAttacks |= wPawnEastAttacks(brd.wPawns);
    whiteAttacks |= wPawnWestAttacks(brd.wPawns);
    whiteAttacks |= wKnightMoves(brd.wKnights, brd.enemyOrEmpty<true>());
    whiteAttacks |= wBishopMoves(brd.wBishops, brd.black(), brd.empty());
    whiteAttacks |= wRookMoves(brd.wRooks, brd.black(), brd.empty());
    whiteAttacks |= wQueenMoves(brd.wQueens, brd.black(), brd.empty());
    whiteAttacks |= oneAround(brd.wKing);

    return pos & ~whiteAttacks; // Return squares not attacked by white and bKing
  }


  Bitboard CBoard::wKingMoves(Bitboard pos) const {
    // King can move to empty or enemy-occupied safe squares
    Bitboard safeMoves = wKingSafe(m_brd, oneAround(pos)) & m_brd.enemyOrEmpty<true>();

    Bitboard path = F1 | G1;
    Bitboard kingSide = 0;

    // King-side castling
    if (wKingSafe(m_brd, path) && ((m_brd.empty() & path) == path))
      kingSide = pos >> 2 & m_brd.wCastling;


    path = D1 | C1 | B1;
    Bitboard queenSide = 0;

    // Queen-side castling
    if (wKingSafe(m_brd, path) && ((m_brd.empty() & path) == path))
      queenSide = pos << 2 & m_brd.wCastling;


    return safeMoves | kingSide | queenSide;
  }


  Bitboard CBoard::bKingMoves(Bitboard pos) const {
    // King can move to empty or enemy-occupied safe squares
    Bitboard safeMoves = bKingSafe(m_brd, oneAround(pos)) & m_brd.enemyOrEmpty<false>();

    Bitboard path = F8 | G8;
    Bitboard kingSide = 0;

    // King-side castling
    if (bKingSafe(m_brd, path) && ((m_brd.empty() & path) == path))
      kingSide = pos >> 2 & m_brd.bCastling;


    path = D8 | C8 | B8;
    Bitboard queenSide = 0;

    // Queen-side castling
    if (bKingSafe(m_brd, path) && ((m_brd.empty() & path) == path))
      queenSide = pos << 2 & m_brd.bCastling;


    return safeMoves | kingSide | queenSide;
  }


  Bitboard CBoard::pseudoLegalMoves(Bitboard pos) const {
    Bitboard pseudoLegalMoves = 0;

    pseudoLegalMoves |= wPawnMoves(pos & m_brd.wPawns, m_brd.black(), m_brd.empty(), m_brd.enPassant);
    pseudoLegalMoves |= wKnightMoves(pos & m_brd.wKnights, m_brd.enemyOrEmpty<true>());
    pseudoLegalMoves |= wBishopMoves(pos & m_brd.wBishops, m_brd.black(), m_brd.empty());
    pseudoLegalMoves |= wRookMoves(pos & m_brd.wRooks, m_brd.black(), m_brd.empty());
    pseudoLegalMoves |= wQueenMoves(pos & m_brd.wQueens, m_brd.black(), m_brd.empty());
    pseudoLegalMoves |= wKingMoves(pos & m_brd.wKing);

    pseudoLegalMoves |= bPawnMoves(pos & m_brd.bPawns, m_brd.white(), m_brd.empty(), m_brd.enPassant);
    pseudoLegalMoves |= bKnightMoves(pos & m_brd.bKnights, m_brd.enemyOrEmpty<false>());
    pseudoLegalMoves |= bBishopMoves(pos & m_brd.bBishops, m_brd.white(), m_brd.empty());
    pseudoLegalMoves |= bRookMoves(pos & m_brd.bRooks, m_brd.white(), m_brd.empty());
    pseudoLegalMoves |= bQueenMoves(pos & m_brd.bQueens, m_brd.white(), m_brd.empty());
    pseudoLegalMoves |= bKingMoves(pos & m_brd.bKing);

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

    bool isValid = m_brd.whiteToMove() ? bKingSafe(m_brd, m_brd.bKing) : wKingSafe(m_brd, m_brd.wKing);
    unmakeMove();

    return isValid;
  }


  std::vector<std::pair<Bitboard, Bitboard>> CBoard::generateMoves(Bitboard moveFrom) {

    std::vector<std::pair<Bitboard, Bitboard>> moves;
    for (auto pos: CBitboardRange(moveFrom)) {
      Bitboard possibleMoves = legalMoves(pos);

      for (auto moveTo: CBitboardRange(possibleMoves))
        moves.emplace_back(pos, moveTo);
    }

    return moves;
  }


  bool CBoard::movePieceIfValid(Bitboard &pieceSet, Bitboard moveFrom, Bitboard moveTo) {
    if (pieceSet & moveFrom)
      return movePiece(pieceSet, moveFrom, moveTo);

    return false;
  }

  Bitboard CBoard::isWPromotion() const { return m_brd.wPawns & RANK_8; }

  Bitboard CBoard::isBPromotion() const { return m_brd.bPawns & RANK_1; }

  void CBoard::handlePromotion(char promotedPiece) {
    // No promotion is happening; exit early
    if (!isWPromotion() && !isBPromotion())
      return;

    // Reference to the appropriate pawn and piece bitboards
    Bitboard &pawns = isWPromotion() ? m_brd.wPawns : m_brd.bPawns;
    Bitboard *promotedTo = nullptr;  // Pointer to store which piece bitboard will be modified

    switch (promotedPiece) {
      case 'Q':
        promotedTo = isWPromotion() ? &m_brd.wQueens : &m_brd.bQueens;
        break;
      case 'R':
        promotedTo = isWPromotion() ? &m_brd.wRooks : &m_brd.bRooks;
        break;
      case 'B':
        promotedTo = isWPromotion() ? &m_brd.wBishops : &m_brd.bBishops;
        break;
      case 'N':
        promotedTo = isWPromotion() ? &m_brd.wKnights : &m_brd.bKnights;
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

    if (moveTo & m_brd.enPassant) {
      // En-passant capture
      removeCapturedBlack(m_brd.whiteToMove() ? soutOne(moveTo) : nortOne(moveTo), moveInfo.capturedPiece,
                          moveInfo.capturedPieceType);
    } else if (m_brd.whiteToMove() ? (moveTo & nortTwo(moveFrom)) : (moveTo & soutTwo(moveFrom))) {
      // Set en-passant possibility
      m_brd.enPassant = m_brd.whiteToMove() ? nortOne(moveFrom) : soutOne(moveFrom);
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
      Bitboard rookFrom = (moveTo & (m_brd.whiteToMove() ? (1ULL << 2) : (1ULL << 58))) ? 1ULL :
                          (m_brd.whiteToMove() ? (1ULL << 7) : (1ULL << 63));
      Bitboard rookTo = (rookFrom & 1ULL) ? (m_brd.whiteToMove() ? (1ULL << 3) : (1ULL << 59)) :
                        (m_brd.whiteToMove() ? (1ULL << 5) : (1ULL << 61));
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
    MoveInfo moveInfo = {moveFrom, moveTo, 0, m_brd.enPassant, m_brd.onTurn == 1 ? m_brd.wCastling : m_brd.bCastling,
                         m_brd.onTurn, false, 0, 0, nullptr};

    bool isWhite = m_brd.whiteToMove();
    bool enPassantSet = false;

    Bitboard &knights = isWhite ? m_brd.wKnights : m_brd.bKnights;
    Bitboard &bishops = isWhite ? m_brd.wBishops : m_brd.bBishops;
    Bitboard &queens = isWhite ? m_brd.wQueens : m_brd.bQueens;
    Bitboard &pawns = isWhite ? m_brd.wPawns : m_brd.bPawns;
    Bitboard &rooks = isWhite ? m_brd.wRooks : m_brd.bRooks;
    Bitboard &king = isWhite ? m_brd.wKing : m_brd.bKing;
    Bitboard &castling = isWhite ? m_brd.wCastling : m_brd.bCastling;

    // Move piece
    if (movePieceIfValid(knights, moveFrom, moveTo) || movePieceIfValid(bishops, moveFrom, moveTo) ||
        movePieceIfValid(queens, moveFrom, moveTo) ||
        handlePawnMove(pawns, moveFrom, moveTo, enPassantSet, moveInfo) ||
        handleRookMove(rooks, moveFrom, moveTo, castling) ||
        handleKingMove(king, rooks, moveFrom, moveTo, castling)) {

      handleCapture(moveTo, moveInfo);

      if (!enPassantSet)
        m_brd.enPassant = 0;

      m_brd.onTurn *= -1;

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

    Bitboard &pawns = isWhiteMove ? m_brd.wPawns : m_brd.bPawns;
    Bitboard &knights = isWhiteMove ? m_brd.wKnights : m_brd.bKnights;
    Bitboard &bishops = isWhiteMove ? m_brd.wBishops : m_brd.bBishops;
    Bitboard &rooks = isWhiteMove ? m_brd.wRooks : m_brd.bRooks;
    Bitboard &queens = isWhiteMove ? m_brd.wQueens : m_brd.bQueens;
    Bitboard &king = isWhiteMove ? m_brd.wKing : m_brd.bKing;

    Bitboard &opponentPawns = isWhiteMove ? m_brd.bPawns : m_brd.wPawns;
    Bitboard &opponentKnights = isWhiteMove ? m_brd.bKnights : m_brd.wKnights;
    Bitboard &opponentBishops = isWhiteMove ? m_brd.bBishops : m_brd.wBishops;
    Bitboard &opponentRooks = isWhiteMove ? m_brd.bRooks : m_brd.wRooks;
    Bitboard &opponentQueens = isWhiteMove ? m_brd.bQueens : m_brd.wQueens;
    Bitboard &opponentKing = isWhiteMove ? m_brd.bKing : m_brd.wKing;

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
    m_brd.enPassant = lastMove.previousEnPassant;

    if (isWhiteMove) m_brd.wCastling = lastMove.previousCastlingRights;
    else m_brd.bCastling = lastMove.previousCastlingRights;

    m_brd.onTurn = lastMove.previousOnTurn;

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


//  int CBoard::mobilityEvaluation(Board brd) {
//    int mobilityScore = 0;
//
//    int backupOnTurn = brd.onTurn;
//
//    brd.onTurn = 1;
//    // White mobility
//    for (auto piece: CBitboardRange(brd.white()))
//      mobilityScore += popcount(legalMoves(piece)) * 5;
//
//    brd.onTurn = -1;
//    // Black mobility
//    for (auto piece: CBitboardRange(brd.black()))
//      mobilityScore -= popcount(legalMoves(piece)) * 5;
//
//    brd.onTurn = backupOnTurn;
//
//    return mobilityScore;
//  }

  int CBoard::evaluate() const {
    return chs::evaluate(m_brd);
  }

  std::pair<int, std::pair<Bitboard, Bitboard>> CBoard::negamax(int depth) {
    if (depth <= 0)
      return {evaluate(), {0, 0}}; // Return evaluation and a dummy move

    int maxEval = INT_MIN;
    std::pair<Bitboard, Bitboard> bestMove = {0, 0};

    auto moves = generateMoves(m_brd.onMovePositions());

    if (moves.empty())
      return {maxEval, {0, 0}};

    std::cout << moves.size() << std::endl;

    for (const auto &move: moves) {
      makeMove(move.first, move.second);
      int eval = -negamax(depth - 1).first;
      unmakeMove();

      if (eval > maxEval) {
        maxEval = eval;
        bestMove = move;
      }
    }


    return {maxEval, bestMove};
  }
}