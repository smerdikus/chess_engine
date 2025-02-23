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

    bool handleKingMove(Bitboard &king, Bitboard &rooks, Bitboard moveFrom, Bitboard moveTo,
                        Bitboard &castlingRights) const;

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

    Bitboard wPawns, wKnights, wBishops, wRooks, wQueens, wKing;
    Bitboard bPawns, bKnights, bBishops, bRooks, bQueens, bKing;

    explicit CBoard();

    void initPos(Board board);

    bool loadTextures(const std::string texturePath[12]) const;

    void draw(sf::RenderWindow &window, Bitboard moveFrom, sf::Font &font);

    inline Bitboard isWPromotion() const;

    inline Bitboard isBPromotion() const;

    void handlePromotion(char promotionPiece);

    inline bool whiteToMove() const;

    inline bool blackToMove() const;

    Bitboard onMovePositions() const;

    void handleCapture(Bitboard moveTo, MoveInfo &moveInfo);

    inline Bitboard white() const;

    inline Bitboard black() const;

    inline Bitboard empty() const;

    bool makeMove(Bitboard moveFrom, Bitboard moveTo);

    bool unmakeMove();

    Bitboard pseudoLegalMoves(Bitboard pos) const;

    Bitboard legalMoves(Bitboard pos);

    bool isMoveLegal(Bitboard from, Bitboard to);

    std::vector<std::pair<Bitboard, Bitboard>> generateMoves(Bitboard moveFrom);

    bool isEndgame(int threshold) const;

    int materialEvaluation() const;

    int positionalEvaluation() const;

    int kingSafetyEvaluation() const;

    int pawnStructureEvaluation() const;

    int mobilityEvaluation();

    int evaluate();

    static inline int popcount(Bitboard num);

    std::pair<int, std::pair<Bitboard, Bitboard>> negamax(int depth);




    inline static char showPromotionWindow(sf::Font &font) {
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
  };

}

#endif //SFML_CHESS_CBOARD_H