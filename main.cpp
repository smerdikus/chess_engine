// Link to fonts            "/System/Library/Fonts/Supplemental/Arial.ttf"
#include <SFML/Graphics.hpp>

#include "CBoard.h"

using namespace chs;

int main() {
#if SFML_VERSION_MAJOR >= 3
  sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WIDTH, HEIGHT)), "CHESS negamax", sf::Style::Close);
#else
  sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "CHESS negamax", sf::Style::Close);
#endif

  window.setFramerateLimit(60);



  // Initialize the board
  CBoard brd;

  std::string textures[] = {
          "../assets/white-pawn.png",
          "../assets/white-king.png",
          "../assets/white-knight.png",
          "../assets/white-bishop.png",
          "../assets/white-queen.png",
          "../assets/white-rook.png",
          "../assets/black-pawn.png",
          "../assets/black-king.png",
          "../assets/black-knight.png",
          "../assets/black-bishop.png",
          "../assets/black-queen.png",
          "../assets/black-rook.png"
  };



  // Load textures of the pieces
  if (!brd.loadTextures(textures))
    return -1;


  Bitboard moveFrom = 0;


  sf::Font font;
#if SFML_VERSION_MAJOR >= 3
  if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
#else
  if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
#endif
    std::cout << "Could not load font" << std::endl;
    return 1;
  }


#if SFML_VERSION_MAJOR >= 3
  sf::Text victoryText(font, "", 64);
#else
  sf::Text victoryText;
  victoryText.setFont(font);
  victoryText.setCharacterSize(64);
#endif

  victoryText.setFillColor(sf::Color::Red);
  victoryText.setStyle(sf::Text::Bold);



  // Main loop handling window

  while (window.isOpen()) {
    int mouseX = sf::Mouse::getPosition(window).x;
    int mouseY = sf::Mouse::getPosition(window).y;


#if SFML_VERSION_MAJOR >= 3
    while (const auto event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
        continue;
      }

      if (const auto *keyEvent = event->getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape)
          window.close();
        if (keyEvent->code == sf::Keyboard::Key::B)
          brd.unmakeMove();
        continue;
      }

      if (const auto *mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
          // Here we need to get the index of the piece we clicked
          int index = (mouseX / TILE) + ((HEIGHT - 30 - mouseY) / TILE) * 8;
          std::cout << index << std::endl;
          Bitboard currentPos = 1ULL << index;

          if (moveFrom == 0) {
            if (currentPos & brd.onMovePositions())
              moveFrom = currentPos;

          } else {
            Bitboard moveTo = currentPos;

            if (moveFrom == currentPos || (currentPos & brd.onMovePositions())) {
              moveFrom = currentPos;
              continue;
            }

            // If the move is in legal moves, provide it, if not just continue
            if (brd.isMoveLegal(moveFrom, moveTo)) {
              brd.makeMove(moveFrom, moveTo);

              std::pair<Bitboard, Bitboard> move;
              if (isEndgame(brd.m_brd))
                move = brd.negamax(6).second;
              else
                move = brd.negamax(3).second;
              brd.makeMove(move.first, move.second);
            }

            moveFrom = 0;
          }
        }
      }
    }
#else
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {

        case sf::Event::Closed:
          window.close();
          break;

        case sf::Event::KeyPressed:
          if (event.key.code == sf::Keyboard::Escape)
            window.close();
          if (event.key.code == sf::Keyboard::B)
            brd.unmakeMove();
          break;

        case sf::Event::MouseButtonPressed:
          if (event.mouseButton.button == sf::Mouse::Left) {
            // Here we need to get the index of the piece we clicked
            int index = (mouseX / TILE) + ((HEIGHT - 30 - mouseY) / TILE) * 8;
            std::cout << index << std::endl;
            Bitboard currentPos = 1ULL << index;

            if (moveFrom == 0) {
              if (currentPos & brd.onMovePositions())
                moveFrom = currentPos;

            } else {
              Bitboard moveTo = currentPos;

              if (moveFrom == currentPos || (currentPos & brd.onMovePositions())) {
                moveFrom = currentPos;
                break;
              }

              // If the move is in legal moves, provide it, if not just continue
              if (brd.isMoveLegal(moveFrom, moveTo)) {
                brd.makeMove(moveFrom, moveTo);

                std::pair<Bitboard, Bitboard> move;
                if (isEndgame(brd.m_brd))
                  move = brd.negamax(6).second;
                else
                  move = brd.negamax(3).second;
                brd.makeMove(move.first, move.second);
              }

              moveFrom = 0;
            }
          }
          break;

        default:
          break;
      }
    }
#endif


    if (brd.isWPromotion() | brd.isBPromotion())
      brd.handlePromotion(CBoard::showPromotionWindow(font));


    window.clear(sf::Color::Black);

    // Test if there are any more moves
    if (brd.legalMoves(brd.onMovePositions())) {
      brd.draw(window, moveFrom, font);
    } else {
      // If white is on turn, and has no moves, black won, negation is otherwise
      window.clear(brd.m_brd.whiteToMove() ? sf::Color::Black : sf::Color::White);

      // set the victory string based on the whiteToMove
      victoryText.setString(brd.m_brd.whiteToMove() ? "Black won" : "White won");

#if SFML_VERSION_MAJOR >= 3
      const auto textRect = victoryText.getLocalBounds();
      victoryText.setPosition(sf::Vector2f((WIDTH - textRect.size.x) / 2.f,
                                           (HEIGHT - 100 - textRect.size.y) / 2.f));
#else
      sf::FloatRect textRect = victoryText.getLocalBounds();
      victoryText.setPosition(sf::Vector2f((WIDTH - textRect.width) / 2.f,
                                           (HEIGHT - 100 - textRect.height) / 2.f));
#endif

      window.draw(victoryText); // draw the victory text
    }

    window.display();
  }

  return 0;
}
