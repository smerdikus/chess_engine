#ifndef SFML_CHESS_BOARD_RENDERER_H
#define SFML_CHESS_BOARD_RENDERER_H

#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include <string>
#include "engine/Board.h"
#include "ui/UiConstants.h"

namespace chs {

  class BoardRenderer {
  public:
    explicit BoardRenderer(Board &board);

    bool loadTextures(const std::string texturePath[12]);

    void draw(sf::RenderWindow &window, Bitboard moveFrom, sf::Font &font);

    static char showPromotionWindow(sf::Font &font);

  private:
    Board &m_board;

    // Colors for the palette
    sf::Color m_lightSquareColor;
    sf::Color m_darkSquareColor;
    sf::Color m_borderColor;
    sf::Color m_highlightSrcColor;
    sf::Color m_highlightDstColor;

    std::array<std::optional<sf::Sprite>, 12> m_sprites;
    std::array<sf::Texture, 12> m_textures;
    sf::RectangleShape m_rectangle;
  };

} // namespace chs

#endif //SFML_CHESS_BOARD_RENDERER_H
