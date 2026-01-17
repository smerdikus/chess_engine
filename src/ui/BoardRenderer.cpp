//
// Board renderer and promotion dialog.
//

#include "ui/BoardRenderer.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace chs {

  BoardRenderer::BoardRenderer(Board &board) : m_board(board) {}

  bool BoardRenderer::loadTextures(const std::string texturePath[12]) {
    for (int i = 0; i < 12; ++i) {
      if (!m_textures[i].loadFromFile(texturePath[i])) {
        std::cerr << "Failed to load texture: " << texturePath[i] << std::endl;
        return false;
      }

      m_sprites[i].emplace(m_textures[i]);
      sf::Vector2u textureSize = m_textures[i].getSize();

      float scaleFactor = static_cast<float>(TILE) / static_cast<float>(std::max(textureSize.x, textureSize.y));
      m_sprites[i]->setScale(sf::Vector2f(scaleFactor, scaleFactor));
    }

    m_lightSquareColor = sf::Color(240, 248, 255);  // Alice blue
    m_darkSquareColor = sf::Color(70, 130, 180);    // Steel blue
    m_borderColor = sf::Color(60, 100, 150);        // Deep blue
    m_highlightSrcColor = sf::Color(0, 191, 255);   // Deep sky blue
    m_highlightDstColor = sf::Color(30, 144, 255);  // Dodger blue

    m_rectangle = sf::RectangleShape(sf::Vector2f(TILE - BORDER * 2, TILE - BORDER * 2));
    m_rectangle.setOutlineColor(m_borderColor);
    m_rectangle.setOutlineThickness(BORDER);

    return true;
  }


  void BoardRenderer::draw(sf::RenderWindow &window, Bitboard moveFrom, sf::Font &font) {
    window.setSize(sf::Vector2u(WIDTH, HEIGHT));

    // Drawing the board and squares
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        m_rectangle.setPosition(sf::Vector2f(static_cast<float>(x * TILE + BORDER),
                                             static_cast<float>(y * TILE + BORDER)));
        m_rectangle.setFillColor((x + y) % 2 == 0 ? m_lightSquareColor : m_darkSquareColor);
        window.draw(m_rectangle);
      }
    }

    if (moveFrom) {
      int pos = __builtin_ctzll(moveFrom);

      int x = pos % 8;
      int y = 7 - (pos / 8);

      m_rectangle.setPosition(sf::Vector2f(static_cast<float>(x * TILE + BORDER),
                                           static_cast<float>(y * TILE + BORDER)));
      m_rectangle.setFillColor(m_highlightSrcColor);
      window.draw(m_rectangle);

      Bitboard possibleMoves = m_board.legalMoves(moveFrom);
      for (int square = 0; square < 64; ++square) {
        if (possibleMoves & (1ULL << square)) {
          int x = square % 8;
          int y = 7 - (square / 8);

          m_rectangle.setPosition(sf::Vector2f(static_cast<float>(x * TILE + BORDER),
                                               static_cast<float>(y * TILE + BORDER)));
          m_rectangle.setFillColor(m_highlightDstColor);
          window.draw(m_rectangle);
        }
      }
    }

    auto posFromBitboard = [&](sf::Sprite &sprite, Bitboard bitboard) {
      for (int square = 0; square < 64; ++square) {
        if (bitboard & (1ULL << square)) {
          int rank = square / 8;
          int file = square % 8;

          sprite.setPosition(sf::Vector2f(static_cast<float>(file) * TILE,
                                          static_cast<float>(7 - rank) * TILE));

          window.draw(sprite);
        }
      }
    };

    const BoardState &state = m_board.state();
    Bitboard pieces[12] = {
            state.wPawns, state.wKing, state.wKnights, state.wBishops, state.wQueens, state.wRooks,
            state.bPawns, state.bKing, state.bKnights, state.bBishops, state.bQueens, state.bRooks
    };

    for (size_t i = 0; i < 12; ++i) {
      if (!m_sprites[i]) continue;
      posFromBitboard(*m_sprites[i], pieces[i]);
    }

    sf::RectangleShape winningRect(sf::Vector2f(4 * TILE + m_board.evaluate() * state.onTurn / 3, 30));
    winningRect.setPosition(sf::Vector2f(0.f, static_cast<float>(8 * TILE)));
    winningRect.setFillColor(sf::Color::White);
    window.draw(winningRect);

#if SFML_VERSION_MAJOR >= 3
    sf::Text text(font, "", 24);
#else
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
#endif
    text.setString(std::to_string(m_board.evaluate() * state.onTurn));
    text.setFillColor(sf::Color(130, 130, 160, 255));

#if SFML_VERSION_MAJOR >= 3
    const auto textBounds = text.getLocalBounds();
    text.setPosition(sf::Vector2f((WIDTH - textBounds.size.x) / 2.f,
                                  static_cast<float>(TILE * 8)));
#else
    text.setPosition(sf::Vector2f((WIDTH - text.getLocalBounds().width) / 2.f,
                                  static_cast<float>(TILE * 8)));
#endif

    window.draw(text);
  }

  char BoardRenderer::showPromotionWindow(sf::Font &font) {
#if SFML_VERSION_MAJOR >= 3
    sf::RenderWindow promotionWindow(sf::VideoMode(sf::Vector2u(150, 100)), "Pawn Promotion");
#else
    sf::RenderWindow promotionWindow(sf::VideoMode(150, 100), "Pawn Promotion");
#endif

    std::string texts[4] = {"Q - Queen", "R - Rook", "B - Bishop", "N - Knight"};

#if SFML_VERSION_MAJOR >= 3
    sf::Text text(font, "", 20);
#else
    sf::Text text("", font, 20);
#endif

    text.setPosition(sf::Vector2f(20.f, 5.f));
    text.setFillColor(sf::Color::Black);

    char chosenPiece = '\0';

    while (promotionWindow.isOpen()) {
#if SFML_VERSION_MAJOR >= 3
      while (const auto event = promotionWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          promotionWindow.close();
          continue;
        }

        if (const auto *keyEvent = event->getIf<sf::Event::KeyPressed>()) {
          switch (keyEvent->code) {
            case sf::Keyboard::Key::Q:
              chosenPiece = 'Q';
              promotionWindow.close();
              break;
            case sf::Keyboard::Key::R:
              chosenPiece = 'R';
              promotionWindow.close();
              break;
            case sf::Keyboard::Key::B:
              chosenPiece = 'B';
              promotionWindow.close();
              break;
            case sf::Keyboard::Key::N:
              chosenPiece = 'N';
              promotionWindow.close();
              break;
            default:
              break;
          }
        }
      }
#else
      sf::Event event;
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
#endif

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

} // namespace chs
