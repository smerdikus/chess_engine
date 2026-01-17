//
// Created by Petr Smerda on 01.07.2024.
//

#include "engine/Board.h"
#include "engine/Fen.h"

#include <stdexcept>

namespace chs {

  Board::Board() {
    // Place pieces on initial positions
    m_state = Board::getInitBoard();
  }

  void Board::initPos(BoardState board) {
    // Positioning pieces based on input structure
    m_state = board;
  }

  const BoardState &Board::state() const {
    return m_state;
  }

  bool Board::loadFen(const std::string &fen) {
    BoardState parsed;
    if (!parseFen(fen, parsed))
      return false;
    initPos(parsed);
    return true;
  }

  Bitboard Board::onMovePositions() const {
    return m_state.onMovePositions();
  }


  bool Board::movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo) {
    pieces &= ~moveFrom;
    pieces |= moveTo;
    return true;
  }

  char Board::promotionPieceFromPtr(const Bitboard *promotedTo) const {
    if (!promotedTo) return '\0';
    if (promotedTo == &m_state.wQueens || promotedTo == &m_state.bQueens) return 'Q';
    if (promotedTo == &m_state.wRooks || promotedTo == &m_state.bRooks) return 'R';
    if (promotedTo == &m_state.wBishops || promotedTo == &m_state.bBishops) return 'B';
    if (promotedTo == &m_state.wKnights || promotedTo == &m_state.bKnights) return 'N';
    return '\0';
  }

  void Board::clearRedoStack() {
    std::stack<RedoInfo> empty;
    std::swap(m_redoList, empty);
  }

  void Board::handleCapture(Bitboard moveTo, MoveInfo &moveInfo) {
    if (m_state.whiteToMove()) removeCapturedBlack(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
    else removeCapturedWhite(moveTo, moveInfo.capturedPiece, moveInfo.capturedPieceType);
  }

  void Board::removeCapturedBlack(Bitboard moveTo, Bitboard &removedFrom, char &pieceType) {
    // Array of bitboards for black pieces and corresponding moveTo types
    Bitboard *blackPieces[] = {&m_state.bPawns, &m_state.bKnights, &m_state.bBishops, &m_state.bRooks, &m_state.bQueens,
                               &m_state.bKing};
    char blackPieceTypes[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

    for (int i = 0; i < 6; ++i) {
      if (*blackPieces[i] & moveTo) {
        *blackPieces[i] &= ~moveTo;
        pieceType = blackPieceTypes[i];
        removedFrom = moveTo;
        if (pieceType == 'R') {
          if (moveTo & A8) m_state.bCastling &= ~C8;
          if (moveTo & H8) m_state.bCastling &= ~G8;
        }
        break;
      }
    }
  }

  void Board::removeCapturedWhite(Bitboard piece, Bitboard &removedFrom, char &pieceType) {
    // Array of bitboards for white pieces and corresponding piece types
    Bitboard *whitePieces[] = {&m_state.wPawns, &m_state.wKnights, &m_state.wBishops, &m_state.wRooks, &m_state.wQueens,
                               &m_state.wKing};
    char whitePieceTypes[] = {'P', 'N', 'B', 'R', 'Q', 'K'};

    for (int i = 0; i < 6; ++i) {
      if (*whitePieces[i] & piece) {
        *whitePieces[i] &= ~piece;
        pieceType = whitePieceTypes[i];
        removedFrom = piece;
        if (pieceType == 'R') {
          if (piece & A1) m_state.wCastling &= ~C1;
          if (piece & H1) m_state.wCastling &= ~G1;
        }
        break;
      }
    }
  }


  Bitboard Board::wKingSafe(BoardState brd, Bitboard pos) {
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

  Bitboard Board::bKingSafe(BoardState brd, Bitboard pos) {
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


  Bitboard Board::wKingMoves(Bitboard pos) const {
    // King can move to empty or enemy-occupied safe squares
    Bitboard safeMoves = wKingSafe(m_state, oneAround(pos)) & m_state.enemyOrEmpty<true>();

    Bitboard path = F1 | G1;
    Bitboard safe = pos | path;
    Bitboard kingSide = 0;

    // King-side castling
    if ((m_state.wCastling & G1) && (wKingSafe(m_state, safe) == safe) && ((m_state.empty() & path) == path))
      kingSide = pos << 2;


    path = D1 | C1 | B1;
    safe = pos | D1 | C1;
    Bitboard queenSide = 0;

    // Queen-side castling
    if ((m_state.wCastling & C1) && (wKingSafe(m_state, safe) == safe) && ((m_state.empty() & path) == path))
      queenSide = pos >> 2;


    return safeMoves | kingSide | queenSide;
  }


  Bitboard Board::bKingMoves(Bitboard pos) const {
    // King can move to empty or enemy-occupied safe squares
    Bitboard safeMoves = bKingSafe(m_state, oneAround(pos)) & m_state.enemyOrEmpty<false>();

    Bitboard path = F8 | G8;
    Bitboard safe = pos | path;
    Bitboard kingSide = 0;

    // King-side castling
    if ((m_state.bCastling & G8) && (bKingSafe(m_state, safe) == safe) && ((m_state.empty() & path) == path))
      kingSide = pos << 2;


    path = D8 | C8 | B8;
    safe = pos | D8 | C8;
    Bitboard queenSide = 0;

    // Queen-side castling
    if ((m_state.bCastling & C8) && (bKingSafe(m_state, safe) == safe) && ((m_state.empty() & path) == path))
      queenSide = pos >> 2;


    return safeMoves | kingSide | queenSide;
  }


  Bitboard Board::pseudoLegalMoves(Bitboard pos) const {
    Bitboard pseudoLegalMoves = 0;

    pseudoLegalMoves |= wPawnMoves(pos & m_state.wPawns, m_state.black(), m_state.empty(), m_state.enPassant);
    pseudoLegalMoves |= wKnightMoves(pos & m_state.wKnights, m_state.enemyOrEmpty<true>());
    pseudoLegalMoves |= wBishopMoves(pos & m_state.wBishops, m_state.black(), m_state.empty());
    pseudoLegalMoves |= wRookMoves(pos & m_state.wRooks, m_state.black(), m_state.empty());
    pseudoLegalMoves |= wQueenMoves(pos & m_state.wQueens, m_state.black(), m_state.empty());
    pseudoLegalMoves |= wKingMoves(pos & m_state.wKing);

    pseudoLegalMoves |= bPawnMoves(pos & m_state.bPawns, m_state.white(), m_state.empty(), m_state.enPassant);
    pseudoLegalMoves |= bKnightMoves(pos & m_state.bKnights, m_state.enemyOrEmpty<false>());
    pseudoLegalMoves |= bBishopMoves(pos & m_state.bBishops, m_state.white(), m_state.empty());
    pseudoLegalMoves |= bRookMoves(pos & m_state.bRooks, m_state.white(), m_state.empty());
    pseudoLegalMoves |= bQueenMoves(pos & m_state.bQueens, m_state.white(), m_state.empty());
    pseudoLegalMoves |= bKingMoves(pos & m_state.bKing);

    return pseudoLegalMoves;
  }


  Bitboard Board::legalMoves(Bitboard pos) {
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


  bool Board::isMoveLegal(Bitboard from, Bitboard to) {
    // If we cannot make the move we don't want to unmake it, so return false and the move is not legal
    if (!makeMove(from, to))
      return false;

    bool isValid = m_state.whiteToMove() ? bKingSafe(m_state, m_state.bKing) : wKingSafe(m_state, m_state.wKing);
    unmakeMove();

    return isValid;
  }


  std::vector<std::pair<Bitboard, Bitboard>> Board::generateMoves(Bitboard moveFrom) {

    std::vector<std::pair<Bitboard, Bitboard>> moves;
    for (auto pos: CBitboardRange(moveFrom)) {
      Bitboard possibleMoves = legalMoves(pos);

      for (auto moveTo: CBitboardRange(possibleMoves))
        moves.emplace_back(pos, moveTo);
    }

    return moves;
  }


  bool Board::movePieceIfValid(Bitboard &pieceSet, Bitboard moveFrom, Bitboard moveTo) {
    if (pieceSet & moveFrom)
      return movePiece(pieceSet, moveFrom, moveTo);

    return false;
  }

  Bitboard Board::isWPromotion() const { return m_state.wPawns & RANK_8; }

  Bitboard Board::isBPromotion() const { return m_state.bPawns & RANK_1; }

  void Board::handlePromotion(char promotedPiece) {
    // No promotion is happening; exit early
    if (!isWPromotion() && !isBPromotion())
      return;

    // Reference to the appropriate pawn and piece bitboards
    Bitboard &pawns = isWPromotion() ? m_state.wPawns : m_state.bPawns;
    Bitboard *promotedTo = nullptr;  // Pointer to store which piece bitboard will be modified

    switch (promotedPiece) {
      case 'Q':
        promotedTo = isWPromotion() ? &m_state.wQueens : &m_state.bQueens;
        break;
      case 'R':
        promotedTo = isWPromotion() ? &m_state.wRooks : &m_state.bRooks;
        break;
      case 'B':
        promotedTo = isWPromotion() ? &m_state.wBishops : &m_state.bBishops;
        break;
      case 'N':
        promotedTo = isWPromotion() ? &m_state.wKnights : &m_state.bKnights;
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


  bool Board::handlePawnMove(Bitboard &pawns, Bitboard moveFrom, Bitboard moveTo,
                              bool &enPassantSet, MoveInfo &moveInfo) {

    if (!(pawns & moveFrom)) return false;

    movePiece(pawns, moveFrom, moveTo);

    if (moveTo & m_state.enPassant) {
      // En-passant capture
      moveInfo.wasEnPassant = true;
      if (m_state.whiteToMove()) {
        removeCapturedBlack(soutOne(moveTo), moveInfo.capturedPiece, moveInfo.capturedPieceType);
      } else {
        removeCapturedWhite(nortOne(moveTo), moveInfo.capturedPiece, moveInfo.capturedPieceType);
      }
    } else if (m_state.whiteToMove() ? (moveTo & nortTwo(moveFrom)) : (moveTo & soutTwo(moveFrom))) {
      // Set en-passant possibility
      m_state.enPassant = m_state.whiteToMove() ? nortOne(moveFrom) : soutOne(moveFrom);
      enPassantSet = true;
    }

    return true;
  }


  bool Board::handleRookMove(Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo, Bitboard &castlingRights) {
    if (!(rooks & moveFrom)) return false;

    movePiece(rooks, moveFrom, moveTo);
    // Disable castling rights if the rook moved from its original square
    if (castlingRights & (C1 | G1)) {
      if (moveFrom & A1) castlingRights &= ~C1;
      if (moveFrom & H1) castlingRights &= ~G1;
    } else if (castlingRights & (C8 | G8)) {
      if (moveFrom & A8) castlingRights &= ~C8;
      if (moveFrom & H8) castlingRights &= ~G8;
    }

    return true;
  }

  bool Board::handleKingMove(Bitboard &king, Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo,
                              Bitboard &castlingRights) const {
    if (!(king & moveFrom)) return false;

    movePiece(king, moveFrom, moveTo);

    // Handle castling
    if (castlingRights & moveTo) {
      if (m_state.whiteToMove()) {
        if (moveTo == G1) movePiece(rooks, H1, F1);
        if (moveTo == C1) movePiece(rooks, A1, D1);
      } else {
        if (moveTo == G8) movePiece(rooks, H8, F8);
        if (moveTo == C8) movePiece(rooks, A8, D8);
      }
    }

    // Disable castling rights if the king moves
    castlingRights = 0;

    return true;
  }


  bool Board::makeMove(const Bitboard moveFrom, const Bitboard moveTo, bool shouldClearRedo) {
    Bitboard pseudoMoves = pseudoLegalMoves(moveFrom);

    if (!(moveTo & pseudoMoves)) return false;

    // Must store the info before the move
    MoveInfo moveInfo = {moveFrom, moveTo, 0, m_state.enPassant, m_state.onTurn == 1 ? m_state.wCastling : m_state.bCastling,
                         m_state.onTurn, false, 0, false, nullptr};

    bool isWhite = m_state.whiteToMove();
    bool enPassantSet = false;

    Bitboard &knights = isWhite ? m_state.wKnights : m_state.bKnights;
    Bitboard &bishops = isWhite ? m_state.wBishops : m_state.bBishops;
    Bitboard &queens = isWhite ? m_state.wQueens : m_state.bQueens;
    Bitboard &pawns = isWhite ? m_state.wPawns : m_state.bPawns;
    Bitboard &rooks = isWhite ? m_state.wRooks : m_state.bRooks;
    Bitboard &king = isWhite ? m_state.wKing : m_state.bKing;
    Bitboard &castling = isWhite ? m_state.wCastling : m_state.bCastling;

    // Move piece
    if (movePieceIfValid(knights, moveFrom, moveTo) || movePieceIfValid(bishops, moveFrom, moveTo) ||
        movePieceIfValid(queens, moveFrom, moveTo) ||
        handlePawnMove(pawns, moveFrom, moveTo, enPassantSet, moveInfo) ||
        handleRookMove(rooks, moveFrom, moveTo, castling) ||
        handleKingMove(king, rooks, moveFrom, moveTo, castling)) {

      handleCapture(moveTo, moveInfo);

      if (!enPassantSet)
        m_state.enPassant = 0;

      m_state.onTurn *= -1;

      moveInfo.wasPromotion = (isWPromotion() || isBPromotion());

      m_moveList.push(moveInfo);
      if (shouldClearRedo) clearRedoStack();

      return true;
    }

    return false;
  }

  bool Board::unmakeMove(bool recordRedo) {
    if (m_moveList.empty()) return false;

    MoveInfo lastMove = m_moveList.top();
    m_moveList.pop();

    const bool promotionApplied = lastMove.wasPromotion && lastMove.promotedTo != nullptr;

    if (recordRedo) {
      RedoInfo redo = {lastMove.moveFrom, lastMove.moveTo, '\0'};
      if (promotionApplied)
        redo.promotionPiece = promotionPieceFromPtr(lastMove.promotedTo);
      m_redoList.push(redo);
    }

    bool isWhiteMove = (lastMove.previousOnTurn == 1);

    Bitboard &pawns = isWhiteMove ? m_state.wPawns : m_state.bPawns;
    Bitboard &knights = isWhiteMove ? m_state.wKnights : m_state.bKnights;
    Bitboard &bishops = isWhiteMove ? m_state.wBishops : m_state.bBishops;
    Bitboard &rooks = isWhiteMove ? m_state.wRooks : m_state.bRooks;
    Bitboard &queens = isWhiteMove ? m_state.wQueens : m_state.bQueens;
    Bitboard &king = isWhiteMove ? m_state.wKing : m_state.bKing;

    Bitboard &opponentPawns = isWhiteMove ? m_state.bPawns : m_state.wPawns;
    Bitboard &opponentKnights = isWhiteMove ? m_state.bKnights : m_state.wKnights;
    Bitboard &opponentBishops = isWhiteMove ? m_state.bBishops : m_state.wBishops;
    Bitboard &opponentRooks = isWhiteMove ? m_state.bRooks : m_state.wRooks;
    Bitboard &opponentQueens = isWhiteMove ? m_state.bQueens : m_state.wQueens;
    Bitboard &opponentKing = isWhiteMove ? m_state.bKing : m_state.wKing;

    // Handle the promotion unmaking
    if (promotionApplied) {
      // Remove the promoted piece from its bitboard
      if (lastMove.promotedTo)
        *lastMove.promotedTo &= ~lastMove.moveTo;  // Remove promoted piece from its final square

      // Restore the pawn to its original position
      pawns |= lastMove.moveFrom;
    }

    // Unmake the move
    if (!unmakePieceMove(pawns, lastMove) && !unmakePieceMove(knights, lastMove) &&
        !unmakePieceMove(bishops, lastMove) && !unmakePieceMove(rooks, lastMove) &&
        !unmakePieceMove(queens, lastMove) && !unmakePieceMove(king, lastMove)) {
      if (!promotionApplied)
        return false;
    }


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
    m_state.enPassant = lastMove.previousEnPassant;

    if (isWhiteMove) m_state.wCastling = lastMove.previousCastlingRights;
    else m_state.bCastling = lastMove.previousCastlingRights;

    m_state.onTurn = lastMove.previousOnTurn;

    return true;
  }

  bool Board::makeNullMove() {
    NullMoveInfo info = {m_state.enPassant, m_state.onTurn};
    m_nullMoveList.push(info);
    m_state.enPassant = 0;
    m_state.onTurn *= -1;
    return true;
  }

  bool Board::unmakeNullMove() {
    if (m_nullMoveList.empty()) return false;
    NullMoveInfo info = m_nullMoveList.top();
    m_nullMoveList.pop();
    m_state.enPassant = info.previousEnPassant;
    m_state.onTurn = info.previousOnTurn;
    return true;
  }

  bool Board::redoMove() {
    if (m_redoList.empty()) return false;

    RedoInfo redo = m_redoList.top();
    m_redoList.pop();

    if (!makeMove(redo.moveFrom, redo.moveTo, false)) {
      m_redoList.push(redo);
      return false;
    }

    if (redo.promotionPiece != '\0' && (isWPromotion() || isBPromotion()))
      handlePromotion(redo.promotionPiece);

    return true;
  }

  bool Board::unmakePieceMove(Bitboard &pieceSet, const MoveInfo &lastMove) {
    if (pieceSet & lastMove.moveTo) {
      // It is unmaking, so we are moving from MoveTo, to moveFrom
      movePiece(pieceSet, lastMove.moveTo, lastMove.moveFrom);
      return true;
    }
    return false;
  }

  void Board::unmakeCastlingMove(Bitboard &rooks, const MoveInfo &lastMove) {
    // King-side castling
    if (lastMove.moveTo == lastMove.moveFrom << 2)
      movePiece(rooks, lastMove.moveFrom << 1, lastMove.moveFrom << 3);

      // Queen-side castling
    else if (lastMove.moveTo == lastMove.moveFrom >> 2)
      movePiece(rooks, lastMove.moveFrom >> 1, lastMove.moveFrom >> 4);

  }

  void Board::restoreCapturedPiece(const MoveInfo &lastMove, Bitboard &opponentPawns, Bitboard &opponentKnights,
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


//  int Board::mobilityEvaluation(BoardState brd) {
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

  int Board::evaluate() const {
    return chs::evaluate(m_state);
  }
}
