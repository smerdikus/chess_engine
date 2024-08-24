// Link to fonts            "/System/Library/Fonts/Supplemental/Arial.ttf"
#include <SFML/Graphics.hpp>
#include "CBoard.h"


int main() {
  sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "CHESS negamax", sf::Style::Close);

  window.setFramerateLimit(60);

  // Board initialization

  // Create a map to store textures
  sf::Texture textures[12];

  // Load textures of the pieces
  if (!textures[0].loadFromFile("../assets/white-pawn.png") ||
      !textures[1].loadFromFile("../assets/white-king.png") ||
      !textures[2].loadFromFile("../assets/white-knight.png") ||
      !textures[3].loadFromFile("../assets/white-bishop.png") ||
      !textures[4].loadFromFile("../assets/white-queen.png") ||
      !textures[5].loadFromFile("../assets/white-rook.png") ||
      !textures[6].loadFromFile("../assets/black-pawn.png") ||
      !textures[7].loadFromFile("../assets/black-king.png") ||
      !textures[8].loadFromFile("../assets/black-knight.png") ||
      !textures[9].loadFromFile("../assets/black-bishop.png") ||
      !textures[10].loadFromFile("../assets/black-queen.png") ||
      !textures[11].loadFromFile("../assets/black-rook.png"))
    return -1;


  // Initialize the board
  CBoard brd(textures);


  Bitboard moveFrom = 0;


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

        case sf::Event::MouseButtonPressed:
          if (event.mouseButton.button == sf::Mouse::Right)
            brd.unmakeMove();


          if (event.mouseButton.button == sf::Mouse::Left) {
            // Here we need to get the index of the piece we clicked
            int index = (mouseX / TILE) + ((HEIGHT - mouseY) / TILE) * 8;
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
              if (brd.canMakeMove(moveFrom, moveTo))
                brd.makeMove(moveFrom, moveTo);



              moveFrom = 0;
            }
          }
          break;

        default:
          break;
      }
    }



    window.clear(sf::Color::Black);

    brd.draw(window, moveFrom);

    window.display();
  }

  return 0;
}