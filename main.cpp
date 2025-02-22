// Link to fonts            "/System/Library/Fonts/Supplemental/Arial.ttf"
#include <SFML/Graphics.hpp>
#include "CBoard.h"


int main() {
  sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "CHESS negamax", sf::Style::Close);

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
  if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
    std::cout << "Could not load font" << std::endl;
    return 1;
  }



  sf::Text victoryText;
  victoryText.setFont(font);
  victoryText.setCharacterSize(64);

  victoryText.setFillColor(sf::Color::Red);
  victoryText.setStyle(sf::Text::Bold);



  // Main loop handling window

  while (window.isOpen()) {
    int mouseX = sf::Mouse::getPosition(window).x;
    int mouseY = sf::Mouse::getPosition(window).y;


    sf::Event event = sf::Event();
    while (window.pollEvent(event)) {
      switch (event.type) {

        case sf::Event::Closed:
          window.close();
          break;

        case sf::Event::KeyPressed:
          if (event.key.code == sf::Keyboard::Escape)
            window.close();
          break;

        case sf::Event::MouseButtonPressed:
          if (event.mouseButton.button == sf::Mouse::Right)
            brd.unmakeMove();


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
              if (brd.isMoveLegal(moveFrom, moveTo))
                brd.makeMove(moveFrom, moveTo);


              moveFrom = 0;
            }
          }
          break;

        default:
          break;
      }
    }


    std::cout << brd.evaluate() << std::endl;


    if (brd.isWPromotion() | brd.isBPromotion())
      brd.handlePromotion(CBoard::showPromotionWindow());

    window.clear(sf::Color::Green);

    brd.draw(window, moveFrom);


    if (!brd.legalMoves(brd.onMovePositions())) {
      // If white is on turn, and has no moves, black won, negation is otherwise
      window.clear(brd.whiteToMove() ? sf::Color::Black : sf::Color::White);


      // set the victory string based on the whiteToMove
      victoryText.setString(brd.whiteToMove() ? "Black won" : "White won");

      sf::FloatRect textRect = victoryText.getLocalBounds();
      victoryText.setPosition(sf::Vector2f((WIDTH - textRect.width) / 2., (HEIGHT - 100 - textRect.height) / 2.));

      window.draw(victoryText); // draw the victory text
    }

    window.display();
  }

  return 0;
}