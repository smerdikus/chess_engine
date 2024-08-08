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


#define TILE    70
#define WIDTH   (8 * TILE) // 8 because We have 8 rectangles here
#define HEIGHT  WIDTH
#define BORDER  1

typedef uint64_t Bitboard;


class CBoard {
private:

  enum class GameStatus {
    InProgress, Draw, WhiteWon, BlackWon
  };


  struct MoveInfo {
    Bitboard moveFrom;
    Bitboard moveTo;
    Bitboard capturedPiece;
    Bitboard previousEnPassant;
    Bitboard previousCastlingRights;
    int previousOnTurn;
    bool wasEnPassant;
    char capturedPieceType; // Store type of captured piece ('P', 'N', 'B', 'R', 'Q', 'K')
  };


/*
 ************************************************************
 *                                                          *
 *                       Board positions                    *
 *                       Board positions                    *
 *                                                          *
 ************************************************************
 */

  static constexpr uint64_t RANK_1 = 0x00000000000000FF;
  static constexpr uint64_t RANK_2 = 0x000000000000FF00;
  static constexpr uint64_t RANK_3 = 0x0000000000FF0000;
  static constexpr uint64_t RANK_4 = 0x00000000FF000000;
  static constexpr uint64_t RANK_5 = 0x000000FF00000000;
  static constexpr uint64_t RANK_6 = 0x0000FF0000000000;
  static constexpr uint64_t RANK_7 = 0x00FF000000000000;
  static constexpr uint64_t RANK_8 = 0xFF00000000000000;

  static constexpr uint64_t FILE_A = 0x0101010101010101;
  static constexpr uint64_t FILE_B = 0x0202020202020202;
  static constexpr uint64_t FILE_C = 0x0404040404040404;
  static constexpr uint64_t FILE_D = 0x0808080808080808;
  static constexpr uint64_t FILE_E = 0x1010101010101010;
  static constexpr uint64_t FILE_F = 0x2020202020202020;
  static constexpr uint64_t FILE_G = 0x4040404040404040;
  static constexpr uint64_t FILE_H = 0x8080808080808080;

  static constexpr uint64_t NOT_FILE_A = ~0x0101010101010101;
  static constexpr uint64_t NOT_FILE_B = ~0x0202020202020202;
  static constexpr uint64_t NOT_FILE_C = ~0x0404040404040404;
  static constexpr uint64_t NOT_FILE_D = ~0x0808080808080808;
  static constexpr uint64_t NOT_FILE_E = ~0x1010101010101010;
  static constexpr uint64_t NOT_FILE_F = ~0x2020202020202020;
  static constexpr uint64_t NOT_FILE_G = ~0x4040404040404040;
  static constexpr uint64_t NOT_FILE_H = ~0x8080808080808080;


/*
 ************************************************************
 *                                                          *
 *                     Evaluation values                    *
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


  // Create piece bitboards
  Bitboard wPawns, wKnights, wBishops, wRooks, wQueens, wKing;
  Bitboard bPawns, bKnights, bBishops, bRooks, bQueens, bKing;

  Bitboard wCastling;
  Bitboard bCastling;

  Bitboard enPassant;

  int onTurn;

  std::stack<MoveInfo> m_moveList;

  // Colors for the palette
  sf::Color lightSquareColor;
  sf::Color darkSquareColor;
  sf::Color borderColor;
  sf::Color highlightSrcColor;
  sf::Color highlightDstColor;

  // Create an array to store sprites
  mutable sf::Sprite m_sprites[12];     // Just for drawing -> mutable
  mutable sf::RectangleShape m_rectangle;


/*
 ************************************************************
 *                                                          *
 *                    Bitwise Logic                         *
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
 *                      King movement                       *
 *                                                          *
 ************************************************************
 */

  inline static Bitboard oneAround(Bitboard pos);

  Bitboard wKingSafe(Bitboard pos) const;

  Bitboard bKingSafe(Bitboard pos) const;

  Bitboard wKingMoves(Bitboard pos) const;

  Bitboard bKingMoves(Bitboard pos) const;

  /*
 ************************************************************
 *                                                          *
 *                   End of piece methods                   *
 *                   End of piece methods                   *
 *                                                          *
 ************************************************************
 */

  static void movePiece(Bitboard &pieces, Bitboard moveFrom, Bitboard moveTo);

  template<bool isWhite>
  constexpr Bitboard enemyOrEmpty() const {
    if constexpr (isWhite)
      return ~white();
    return ~black();
  }


public:
  explicit CBoard(sf::Texture textures[12]);

  void draw(sf::RenderWindow &window, Bitboard moveFrom);

  bool whiteToMove() const;

  bool blackToMove() const;

  inline Bitboard white() const;

  inline Bitboard black() const;

  inline Bitboard empty() const;

  int pieceCount() const;

  bool makeMove(Bitboard moveFrom, Bitboard moveTo);

  bool canMakeMove(Bitboard moveFrom, Bitboard moveTo);

  bool unmakeMove();

  Bitboard pseudoLegalMoves(Bitboard pos) const;

  Bitboard legalMoves(Bitboard pos);

  bool isMoveLegal(Bitboard from, Bitboard to);

  Bitboard onMovePositions() const;

  std::vector<std::pair<Bitboard, Bitboard>> generateMoves(Bitboard moveFrom);

  GameStatus isCheckmate();

  int evaluate();

  static int pieceSquareValue(Bitboard pieces, const int table[64]);

  static int evaluatePawnStructure(Bitboard pawns, Bitboard opponentPawns);

  int evaluateMobility(Bitboard onMove);

  static int popcount(Bitboard bb);

  int negamax(int depth);
};

#endif //SFML_CHESS_CBOARD_H