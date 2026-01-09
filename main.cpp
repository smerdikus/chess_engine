// Link to fonts            "/System/Library/Fonts/Supplemental/Arial.ttf"
#include <SFML/Graphics.hpp>

#include "CBoard.h"

using namespace chs;

namespace {
  int countPieces(const Board &brd) {
    return popcount(brd.white() | brd.black());
  }

  int chooseSearchDepth(const Board &brd) {
    int pieces = countPieces(brd);
    if (pieces >= 28) return 3;
    if (pieces >= 20) return 4;
    if (pieces >= 14) return 5;
    if (pieces >= 10) return 6;
    return 7;
  }

  std::string squareName(Bitboard bb) {
    if (!bb) return "--";
    int index = __builtin_ctzll(bb);
    char file = static_cast<char>('a' + (index % 8));
    char rank = static_cast<char>('1' + (index / 8));
    return std::string{file, rank};
  }
}

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
  int lastSearchDepth = chooseSearchDepth(brd.m_brd);
  int lastSearchPieces = countPieces(brd.m_brd);


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

#if SFML_VERSION_MAJOR >= 3
  sf::Text depthText(font, "", 16);
#else
  sf::Text depthText;
  depthText.setFont(font);
  depthText.setCharacterSize(16);
#endif
  depthText.setFillColor(sf::Color(210, 210, 230, 255));



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

              lastSearchPieces = countPieces(brd.m_brd);
              lastSearchDepth = chooseSearchDepth(brd.m_brd);
              std::cout << "// ai search depth=" << lastSearchDepth
                        << " pieces=" << lastSearchPieces << std::endl;

              auto result = brd.negamax(lastSearchDepth);
              std::cout << "// ai move " << squareName(result.second.first)
                        << " -> " << squareName(result.second.second)
                        << " eval=" << result.first << std::endl;

              if (result.second.first != 0 && result.second.second != 0)
                brd.makeMove(result.second.first, result.second.second);
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

                lastSearchPieces = countPieces(brd.m_brd);
                lastSearchDepth = chooseSearchDepth(brd.m_brd);
                std::cout << "// ai search depth=" << lastSearchDepth
                          << " pieces=" << lastSearchPieces << std::endl;

                auto result = brd.negamax(lastSearchDepth);
                std::cout << "// ai move " << squareName(result.second.first)
                          << " -> " << squareName(result.second.second)
                          << " eval=" << result.first << std::endl;

                if (result.second.first != 0 && result.second.second != 0)
                  brd.makeMove(result.second.first, result.second.second);
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
      depthText.setString("AI depth: " + std::to_string(lastSearchDepth) +
                          "  Pieces: " + std::to_string(lastSearchPieces));
      depthText.setPosition(sf::Vector2f(10.f, static_cast<float>(TILE * 8 + 6)));
      window.draw(depthText);
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
