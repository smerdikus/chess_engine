//
// Created by Petr Smerda on 01.07.2024.
//

#ifndef SFML_CHESS_BOARD_H
#define SFML_CHESS_BOARD_H


#include <stack>
#include <vector>
#include <utility>
#include <string>
#include <cstdint>
#include "engine/BitboardMovement.h"
#include "engine/CBitboardIterator.h"
#include "engine/ChessMacros.h"
#include "engine/Evaluation.h"

namespace chs {

  class Board {
  private:

    struct MoveInfo {
      Bitboard moveFrom;
      Bitboard moveTo;
      Bitboard capturedPiece;
      Bitboard previousEnPassant;
      Bitboard previousCastlingRights;
      int previousOnTurn;
      bool wasEnPassant;
      char capturedPieceType; // Store type of captured piece ('P', 'N', 'B', 'R', 'Q', 'K')
      bool wasPromotion;
      Bitboard *promotedTo;
    };

    struct RedoInfo {
      Bitboard moveFrom;
      Bitboard moveTo;
      char promotionPiece;
    };

    struct NullMoveInfo {
      Bitboard previousEnPassant;
      int previousOnTurn;
    };

    BoardState m_state;
    std::stack<MoveInfo> m_moveList;
    std::stack<RedoInfo> m_redoList;
    std::stack<NullMoveInfo> m_nullMoveList;

    void removeCapturedWhite(Bitboard pos, Bitboard &removedFrom, char &pieceType);

    void removeCapturedBlack(Bitboard moveTo, Bitboard &removedFrom, char &pieceType);


    Bitboard wKingMoves(Bitboard pos) const;

    Bitboard bKingMoves(Bitboard pos) const;


    inline static bool movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo);

    static bool handleRookMove(Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo, Bitboard &castlingRights);

    bool handleKingMove(Bitboard &king, Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo,
                        Bitboard &castlingRights) const;

    bool handlePawnMove(Bitboard &pawns, Bitboard moveFrom, Bitboard moveTo, bool &enPassantSet, MoveInfo &moveInfo);

    static bool movePieceIfValid(Bitboard &pieceSet, Bitboard moveFrom, Bitboard moveTo);

    static bool unmakePieceMove(Bitboard &pieceSet, const MoveInfo &lastMove);

    static void unmakeCastlingMove(Bitboard &rooks, const MoveInfo &lastMove);

    static void restoreCapturedPiece(const MoveInfo &lastMove, Bitboard &opponentPawns, Bitboard &opponentKnights,
                                     Bitboard &opponentBishops, Bitboard &opponentRooks, Bitboard &opponentQueens,
                                     Bitboard &opponentKing);

    char promotionPieceFromPtr(const Bitboard *promotedTo) const;

    void clearRedoStack();

  public:
    explicit Board();

    void initPos(BoardState board);

    const BoardState &state() const;

    bool loadFen(const std::string &fen);

    Bitboard onMovePositions() const;

    Bitboard isWPromotion() const;

    Bitboard isBPromotion() const;

    static Bitboard wKingSafe(BoardState brd, Bitboard pos);

    static Bitboard bKingSafe(BoardState brd, Bitboard pos);

    void handlePromotion(char promotionPiece);

    void handleCapture(Bitboard moveTo, MoveInfo &moveInfo);

    bool makeMove(Bitboard moveFrom, Bitboard moveTo, bool shouldClearRedo = false);

    bool unmakeMove(bool recordRedo = false);

    bool redoMove();

    bool makeNullMove();

    bool unmakeNullMove();

    Bitboard pseudoLegalMoves(Bitboard pos) const;

    Bitboard legalMoves(Bitboard pos);

    bool isMoveLegal(Bitboard from, Bitboard to);

    std::vector<std::pair<Bitboard, Bitboard>> generateMoves(Bitboard moveFrom);

//    int mobilityEvaluation(BoardState brd);

    int evaluate() const;

    static inline BoardState getInitBoard() {
      return {
              0x10ULL, 0x1000000000000000ULL,
              0xFF00ULL, 0xFF000000000000ULL,
              0x42ULL, 0x4200000000000000ULL,
              0x24ULL, 0x2400000000000000ULL,
              0x81ULL, 0x8100000000000000ULL,
              0x8ULL, 0x800000000000000ULL,
              C1 | G1, C8 | G8,
              0, 1
      };
    }

  };

}

#endif //SFML_CHESS_BOARD_H
