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
#include "CBitboardIterator.h"


#define TILE    70
#define WIDTH   (8 * TILE) // 8 because We have 8 rectangles here
#define HEIGHT  WIDTH + 30
#define BORDER  1

typedef uint64_t Bitboard;


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

  Bitboard m_enPassant;

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




/*
 ************************************************************
 *                                                          *
 *                       Board positions                    *
 *                                                          *
 ************************************************************
 */

  static constexpr Bitboard RANK_1 = 0x00000000000000FF;
  static constexpr Bitboard RANK_2 = 0x000000000000FF00;
  static constexpr Bitboard RANK_3 = 0x0000000000FF0000;
  static constexpr Bitboard RANK_4 = 0x00000000FF000000;
  static constexpr Bitboard RANK_5 = 0x000000FF00000000;
  static constexpr Bitboard RANK_6 = 0x0000FF0000000000;
  static constexpr Bitboard RANK_7 = 0x00FF000000000000;
  static constexpr Bitboard RANK_8 = 0xFF00000000000000;

  static constexpr Bitboard FILE_A = 0x0101010101010101;
  static constexpr Bitboard FILE_B = 0x0202020202020202;
  static constexpr Bitboard FILE_C = 0x0404040404040404;
  static constexpr Bitboard FILE_D = 0x0808080808080808;
  static constexpr Bitboard FILE_E = 0x1010101010101010;
  static constexpr Bitboard FILE_F = 0x2020202020202020;
  static constexpr Bitboard FILE_G = 0x4040404040404040;
  static constexpr Bitboard FILE_H = 0x8080808080808080;

  static constexpr Bitboard NOT_FILE_A = ~0x0101010101010101;
  static constexpr Bitboard NOT_FILE_B = ~0x0202020202020202;
  static constexpr Bitboard NOT_FILE_C = ~0x0404040404040404;
  static constexpr Bitboard NOT_FILE_D = ~0x0808080808080808;
  static constexpr Bitboard NOT_FILE_E = ~0x1010101010101010;
  static constexpr Bitboard NOT_FILE_F = ~0x2020202020202020;
  static constexpr Bitboard NOT_FILE_G = ~0x4040404040404040;
  static constexpr Bitboard NOT_FILE_H = ~0x8080808080808080;

  const Bitboard A1 = 1ULL << 0;
  const Bitboard B1 = 1ULL << 1;
  const Bitboard C1 = 1ULL << 2;
  const Bitboard D1 = 1ULL << 3;
  const Bitboard E1 = 1ULL << 4;
  const Bitboard F1 = 1ULL << 5;
  const Bitboard G1 = 1ULL << 6;
  const Bitboard H1 = 1ULL << 7;

  const Bitboard A2 = 1ULL << 8;
  const Bitboard B2 = 1ULL << 9;
  const Bitboard C2 = 1ULL << 10;
  const Bitboard D2 = 1ULL << 11;
  const Bitboard E2 = 1ULL << 12;
  const Bitboard F2 = 1ULL << 13;
  const Bitboard G2 = 1ULL << 14;
  const Bitboard H2 = 1ULL << 15;

  const Bitboard A3 = 1ULL << 16;
  const Bitboard B3 = 1ULL << 17;
  const Bitboard C3 = 1ULL << 18;
  const Bitboard D3 = 1ULL << 19;
  const Bitboard E3 = 1ULL << 20;
  const Bitboard F3 = 1ULL << 21;
  const Bitboard G3 = 1ULL << 22;
  const Bitboard H3 = 1ULL << 23;

  const Bitboard A4 = 1ULL << 24;
  const Bitboard B4 = 1ULL << 25;
  const Bitboard C4 = 1ULL << 26;
  const Bitboard D4 = 1ULL << 27;
  const Bitboard E4 = 1ULL << 28;
  const Bitboard F4 = 1ULL << 29;
  const Bitboard G4 = 1ULL << 30;
  const Bitboard H4 = 1ULL << 31;

  const Bitboard A5 = 1ULL << 32;
  const Bitboard B5 = 1ULL << 33;
  const Bitboard C5 = 1ULL << 34;
  const Bitboard D5 = 1ULL << 35;
  const Bitboard E5 = 1ULL << 36;
  const Bitboard F5 = 1ULL << 37;
  const Bitboard G5 = 1ULL << 38;
  const Bitboard H5 = 1ULL << 39;

  const Bitboard A6 = 1ULL << 40;
  const Bitboard B6 = 1ULL << 41;
  const Bitboard C6 = 1ULL << 42;
  const Bitboard D6 = 1ULL << 43;
  const Bitboard E6 = 1ULL << 44;
  const Bitboard F6 = 1ULL << 45;
  const Bitboard G6 = 1ULL << 46;
  const Bitboard H6 = 1ULL << 47;

  const Bitboard A7 = 1ULL << 48;
  const Bitboard B7 = 1ULL << 49;
  const Bitboard C7 = 1ULL << 50;
  const Bitboard D7 = 1ULL << 51;
  const Bitboard E7 = 1ULL << 52;
  const Bitboard F7 = 1ULL << 53;
  const Bitboard G7 = 1ULL << 54;
  const Bitboard H7 = 1ULL << 55;

  const Bitboard A8 = 1ULL << 56;
  const Bitboard B8 = 1ULL << 57;
  const Bitboard C8 = 1ULL << 58;
  const Bitboard D8 = 1ULL << 59;
  const Bitboard E8 = 1ULL << 60;
  const Bitboard F8 = 1ULL << 61;
  const Bitboard G8 = 1ULL << 62;
  const Bitboard H8 = 1ULL << 63;

/*
 ************************************************************
 *                                                          *
 *                     Evaluation values                    *
 *                                                          *
 ************************************************************
 */


  const int PAWN_VALUE = 100;
  const int KNIGHT_VALUE = 320;
  const int BISHOP_VALUE = 330;
  const int ROOK_VALUE = 500;
  const int QUEEN_VALUE = 900;
  const int KING_VALUE = 20000;

  // Piece-square tables for evaluating positions
  static constexpr int pawnTable[64] = {
          0, 0, 0, 0, 0, 0, 0, 0,
          5, 10, 10, -20, -20, 10, 10, 5,
          5, -5, -10, 0, 0, -10, -5, 5,
          0, 0, 0, 20, 20, 0, 0, 0,
          5, 5, 10, 25, 25, 10, 5, 5,
          10, 10, 20, 30, 30, 20, 10, 10,
          50, 50, 50, 50, 50, 50, 50, 50,
          0, 0, 0, 0, 0, 0, 0, 0
  };

  static constexpr int knightTable[64] = {
          -50, -40, -30, -30, -30, -30, -40, -50,
          -40, -20, 0, 0, 0, 0, -20, -40,
          -30, 0, 10, 15, 15, 10, 0, -30,
          -30, 5, 15, 20, 20, 15, 5, -30,
          -30, 0, 15, 20, 20, 15, 0, -30,
          -30, 5, 10, 15, 15, 10, 5, -30,
          -40, -20, 0, 5, 5, 0, -20, -40,
          -50, -40, -30, -30, -30, -30, -40, -50
  };

  static constexpr int bishopTable[64] = {
          -20, -10, -10, -10, -10, -10, -10, -20,
          -10, 0, 0, 0, 0, 0, 0, -10,
          -10, 0, 5, 10, 10, 5, 0, -10,
          -10, 5, 5, 10, 10, 5, 5, -10,
          -10, 0, 10, 10, 10, 10, 0, -10,
          -10, 10, 10, 10, 10, 10, 10, -10,
          -10, 5, 0, 0, 0, 0, 5, -10,
          -20, -10, -10, -10, -10, -10, -10, -20
  };

  static constexpr int rookTable[64] = {
          0, 0, 0, 0, 0, 0, 0, 0,
          5, 10, 10, 10, 10, 10, 10, 5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          -5, 0, 0, 0, 0, 0, 0, -5,
          0, 0, 0, 5, 5, 0, 0, 0
  };

  static constexpr int queenTable[64] = {
          -20, -10, -10, -5, -5, -10, -10, -20,
          -10, 0, 0, 0, 0, 0, 0, -10,
          -10, 0, 5, 5, 5, 5, 0, -10,
          -5, 0, 5, 5, 5, 5, 0, -5,
          0, 0, 5, 5, 5, 5, 0, -5,
          -10, 5, 5, 5, 5, 5, 0, -10,
          -10, 0, 5, 0, 0, 0, 0, -10,
          -20, -10, -10, -5, -5, -10, -10, -20
  };

  static constexpr int kingTable[64] = {
          20, 30, 10, 0, 0, 10, 30, 20,
          20, 20, 0, 0, 0, 0, 20, 20,
          -10, -20, -20, -20, -20, -20, -20, -10,
          -20, -30, -30, -40, -40, -30, -30, -20,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30,
          -30, -40, -40, -50, -50, -40, -40, -30
  };

  static constexpr int kingEndgameTable[64] = {
          -50, -40, -30, -20, -20, -30, -40, -50,
          -30, -20, -10, 0, 0, -10, -20, -30,
          -30, -10, 20, 30, 30, 20, -10, -30,
          -30, -10, 30, 40, 40, 30, -10, -30,
          -30, -10, 30, 40, 40, 30, -10, -30,
          -30, -10, 20, 30, 30, 20, -10, -30,
          -30, -30, 0, 0, 0, 0, -30, -30,
          -50, -30, -30, -30, -30, -30, -30, -50
  };


/*
 ************************************************************
 *                                                          *
 *                    Bitwise Logic                         *
 *                                                          *
 ************************************************************
 */

  void removeCapturedWhite(Bitboard pos, Bitboard &removedFrom, char &pieceType);

  void removeCapturedBlack(Bitboard moveTo, Bitboard &removedFrom, char &pieceType);


  inline static Bitboard nortOne(Bitboard pos) { return pos << 8; }

  inline static Bitboard soutOne(Bitboard pos) { return pos >> 8; }

  inline static Bitboard westOne(Bitboard pos) { return (pos & NOT_FILE_H) << 1; }

  inline static Bitboard eastOne(Bitboard pos) { return (pos & NOT_FILE_A) >> 1; }


  inline static Bitboard nortTwo(Bitboard pos) { return pos << 16; }

  inline static Bitboard soutTwo(Bitboard pos) { return pos >> 16; }

  inline static Bitboard westTwo(Bitboard pos) { return (pos & NOT_FILE_H & NOT_FILE_G) << 2; }

  inline static Bitboard eastTwo(Bitboard pos) { return (pos & NOT_FILE_B & NOT_FILE_A) >> 2; }


  inline static Bitboard noWe(Bitboard pos) { return nortOne(westOne(pos)); }

  inline static Bitboard noEa(Bitboard pos) { return nortOne(eastOne(pos)); }

  inline static Bitboard soWe(Bitboard pos) { return soutOne(westOne(pos)); }

  inline static Bitboard soEa(Bitboard pos) { return soutOne(eastOne(pos)); }


/*
 ************************************************************
 *                                                          *
 *                       Pawn movement                      *
 *                                                          *
 ************************************************************
 */

  inline Bitboard wSinglePush(Bitboard pawns) const;

  inline Bitboard bSinglePush(Bitboard pawns) const;

  inline Bitboard wDoublePush(Bitboard pawns) const;

  inline Bitboard bDoublePush(Bitboard pawns) const;

  inline static Bitboard wPawnWestAttacks(Bitboard pawns) { return noWe(pawns); }

  inline static Bitboard wPawnEastAttacks(Bitboard pawns) { return noEa(pawns); }

  inline static Bitboard bPawnWestAttacks(Bitboard pawns) { return soWe(pawns); }

  inline static Bitboard bPawnEastAttacks(Bitboard pawns) { return soEa(pawns); }


  Bitboard wPawnMoves(Bitboard pos) const;

  Bitboard bPawnMoves(Bitboard pos) const;


/*
 ************************************************************
 *                                                          *
 *                    Knight movement                       *
 *                                                          *
 ************************************************************
 */

  inline static Bitboard noNoEa(Bitboard pos) { return (pos & NOT_FILE_H) << 17; }

  inline static Bitboard noEaEa(Bitboard pos) { return (pos & (NOT_FILE_H & NOT_FILE_G)) << 10; }

  inline static Bitboard soEaEa(Bitboard pos) { return (pos & (NOT_FILE_H & NOT_FILE_G)) >> 6; }

  inline static Bitboard soSoEa(Bitboard pos) { return (pos & NOT_FILE_H) >> 15; }

  inline static Bitboard soSoWe(Bitboard pos) { return (pos & NOT_FILE_A) >> 17; }

  inline static Bitboard soWeWe(Bitboard pos) { return (pos & (NOT_FILE_A & NOT_FILE_B)) >> 10; }

  inline static Bitboard noWeWe(Bitboard pos) { return (pos & (NOT_FILE_A & NOT_FILE_B)) << 6; }

  inline static Bitboard noNoWe(Bitboard pos) { return (pos & NOT_FILE_A) << 15; }

  Bitboard wKnightMoves(Bitboard pos) const;

  Bitboard bKnightMoves(Bitboard pos) const;

/*
 ************************************************************
 *                                                          *
 *                    Bishop movement                       *
 *                                                          *
 ************************************************************
 */

  static Bitboard sliderMoves(Bitboard pos, Bitboard (*directionFunc)(Bitboard), Bitboard enemies, Bitboard empty);

  static Bitboard bishopMoves(Bitboard pos, Bitboard enemies, Bitboard empty);

  Bitboard wBishopMoves(Bitboard pos) const;

  Bitboard bBishopMoves(Bitboard pos) const;

/*
 ************************************************************
 *                                                          *
 *                      Rook movement                       *
 *                                                          *
 ************************************************************
 */

  static Bitboard rookMoves(Bitboard pos, Bitboard enemies, Bitboard empty);

  Bitboard wRookMoves(Bitboard pos) const;

  Bitboard bRookMoves(Bitboard pos) const;

/*
 ************************************************************
 *                                                          *
 *                     Queen movement                       *
 *                                                          *
 ************************************************************
 */

  Bitboard wQueenMoves(Bitboard pos) const;

  Bitboard bQueenMoves(Bitboard pos) const;

/*
 ************************************************************
 *                                                          *
 *                      King movement                       *
 *                                                          *
 ************************************************************
 */

  bool wcastleRights() const;

  bool bcastleRights() const;

  inline static Bitboard oneAround(Bitboard pos);

  Bitboard wKingSafe(Bitboard pos) const;

  Bitboard bKingSafe(Bitboard pos) const;

  Bitboard wKingMoves(Bitboard pos) const;

  Bitboard bKingMoves(Bitboard pos) const;

  /*
 ************************************************************
 *                                                          *
 *                   End of piece methods                   *
 *                                                          *
 ************************************************************
 */

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
    Bitboard _wKing, _bKing;
    Bitboard _wPawns, _bPawns;
    Bitboard _wKnights, _bKnights;
    Bitboard _wBishops, _bBishops;
    Bitboard _wRooks, _bRooks;
    Bitboard _wQueens, _bQueens;

    Bitboard _wCastling, _bCastling;
    Bitboard _enPassant;
    int _onTurn;
  };

  explicit CBoard();

  void initPos(Board board);

  bool loadTextures(const std::string texturePath[12]) const;

  void draw(sf::RenderWindow &window, Bitboard moveFrom);

  static char showPromotionWindow();

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

  int countWMaterial() const;

  int countBMaterial() const;

  int materialEvaluation() const;

  int positionalEvaluation() const;

  int kingSafetyEvaluation() const;

  int pawnStructureEvaluation() const;

  int mobilityEvaluation();

  int evaluate();

  static inline int popcount(Bitboard num);

  std::pair<int, std::pair<Bitboard, Bitboard>> negamax(int depth);
};

#endif //SFML_CHESS_CBOARD_H