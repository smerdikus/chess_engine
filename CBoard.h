//
// Created by Petr Smerda on 01.07.2024.
//

#ifndef SFML_CHESS_CBOARD_H
#define SFML_CHESS_CBOARD_H


#include <SFML/Graphics.hpp>
#include <iostream>
#include <bitset>
#include <stack>
#include <cstdint>
#include <filesystem>
#include "BitboardMovement.h"
#include "CBitboardIterator.h"
#include "EvaluationTables.h"

namespace chs {

#define TILE    70
#define WIDTH   (8 * TILE) // 8 because We have 8 rectangles here
#define HEIGHT  WIDTH + 30
#define BORDER  1



  class CBoard {
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
      Bitboard wasPromotion;
      Bitboard *promotedTo;
    };


    // Create piece bitboards
    Bitboard wPawns, wKnights, wBishops, wRooks, wQueens, wKing;
    Bitboard bPawns, bKnights, bBishops, bRooks, bQueens, bKing;

    Bitboard wCastling;
    Bitboard bCastling;

    Bitboard enPassant;

    bool wKingMoved = false;
    bool bKingMoved = false;

    bool wRookMovedKingSide = false;
    bool bRookMovedKingSide = false;

    bool wRookMovedQueenSide = false;
    bool bRookMovedQueenSide = false;

    int onTurn;

    std::stack<MoveInfo> m_moveList;

    // Colors for the palette
    mutable sf::Color lightSquareColor; // Just for drawing -> mutable
    mutable sf::Color darkSquareColor; // Just for drawing -> mutable
    mutable sf::Color borderColor; // Just for drawing -> mutable
    mutable sf::Color highlightSrcColor; // Just for drawing -> mutable
    mutable sf::Color highlightDstColor; // Just for drawing -> mutable

    // Create an array to store sprites
    mutable sf::Sprite m_sprites[12];     // Just for drawing -> mutable
    mutable sf::Texture m_textures[12];     // Just for drawing -> mutable
    mutable sf::RectangleShape m_rectangle; // Just for drawing -> mutable



    void removeCapturedWhite(Bitboard pos, Bitboard &removedFrom, char &pieceType);

    void removeCapturedBlack(Bitboard moveTo, Bitboard &removedFrom, char &pieceType);


    bool wcastleRights() const;

    bool bcastleRights() const;

    Bitboard wKingSafe(Bitboard pos) const;

    Bitboard bKingSafe(Bitboard pos) const;

    Bitboard wKingMoves(Bitboard pos) const;

    Bitboard bKingMoves(Bitboard pos) const;


    static bool movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo);

    template<bool isWhite>
    constexpr Bitboard enemyOrEmpty() const {
      if constexpr (isWhite)
        return ~white();
      return ~black();
    }


    static bool handleRookMove(Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo, Bitboard &castlingRights);

    bool
    handleKingMove(Bitboard &king, Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo, Bitboard &castlingRights) const;

    bool handlePawnMove(Bitboard &pawns, Bitboard moveFrom, Bitboard moveTo, bool &enPassantSet, MoveInfo &moveInfo);

    static bool movePieceIfValid(Bitboard &pieceSet, Bitboard moveFrom, Bitboard moveTo);

    static bool unmakePieceMove(Bitboard &pieceSet, const MoveInfo &lastMove);

    static void unmakeCastlingMove(Bitboard &rooks, const MoveInfo &lastMove);

    static void restoreCapturedPiece(const MoveInfo &lastMove, Bitboard &opponentPawns, Bitboard &opponentKnights,
                                     Bitboard &opponentBishops, Bitboard &opponentRooks, Bitboard &opponentQueens,
                                     Bitboard &opponentKing);


  public:
    struct Board {
      Bitboard wKing, bKing;
      Bitboard wPawns, bPawns;
      Bitboard wKnights, bKnights;
      Bitboard wBishops, bBishops;
      Bitboard wRooks, bRooks;
      Bitboard wQueens, bQueens;

      Bitboard wCastling, bCastling;
      Bitboard enPassant;
      int onTurn;
    };

    explicit CBoard();

    void initPos(Board board);

    bool loadTextures(const std::string texturePath[12]) const;

    void draw(sf::RenderWindow &window, Bitboard moveFrom, sf::Font &font);

    static char showPromotionWindow(sf::Font &font);

    inline Bitboard isWPromotion() const;

    inline Bitboard isBPromotion() const;

    void handlePromotion(char promotionPiece);

    inline bool whiteToMove() const;

    inline bool blackToMove() const;

    void handleCapture(Bitboard moveTo, MoveInfo &moveInfo);

    inline Bitboard white() const;

    inline Bitboard black() const;

    inline Bitboard empty() const;

    bool makeMove(Bitboard moveFrom, Bitboard moveTo);

    bool unmakeMove();

    Bitboard pseudoLegalMoves(Bitboard pos) const;

    Bitboard legalMoves(Bitboard pos);

    bool isMoveLegal(Bitboard from, Bitboard to);

    Bitboard onMovePositions() const;

    std::vector<std::pair<Bitboard, Bitboard>> generateMoves(Bitboard moveFrom);

    bool isEndgame(int threshold) const;

    int evalWMaterial() const;

    int evalBMaterial() const;

    int materialEvaluation() const;

    int positionalEvaluation() const;

    int kingSafetyEvaluation() const;

    int pawnStructureEvaluation() const;

    int mobilityEvaluation();

    int evaluate();

    static inline int popcount(Bitboard num);

    std::pair<int, std::pair<Bitboard, Bitboard>> negamax(int depth);
  };

}

#endif //SFML_CHESS_CBOARD_H