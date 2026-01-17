// Link to fonts            "/System/Library/Fonts/Supplemental/Arial.ttf"
#include <SFML/Graphics.hpp>

#include "engine/Board.h"
#include "engine/Search.h"
#include "ui/BoardRenderer.h"
#include "ui/UiConstants.h"

using namespace chs;

namespace {
	int countPieces(const BoardState &brd) { return popcount(brd.white() | brd.black()); }

	int chooseSearchDepth(const BoardState &brd) {
		int pieces = countPieces(brd);
		if (pieces >= 28) return 4;
		if (pieces >= 20) return 5;
		if (pieces >= 14) return 6;
		if (pieces >= 8) return 8;
		return 10;
	}

  std::string squareName(Bitboard bb) {
    if (!bb) return "--";
    int index = __builtin_ctzll(bb);
    char file = static_cast<char>('a' + (index % 8));
    char rank = static_cast<char>('1' + (index / 8));
    return std::string{file, rank};
  }

  bool sideInCheck(const BoardState &brd) {
    if (brd.whiteToMove())
      return Board::wKingSafe(brd, brd.wKing) == 0;
    return Board::bKingSafe(brd, brd.bKing) == 0;
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
	Board brd;
	Search search;
	BoardRenderer renderer(brd);

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
	if (!renderer.loadTextures(textures)) return -1;


	Bitboard moveFrom = 0;
	int lastSearchDepth = chooseSearchDepth(brd.state());
	int lastSearchPieces = countPieces(brd.state());


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

	const float barTop = static_cast<float>(TILE * 8);
	const float barHeight = 30.f;
	const float buttonSize = 24.f;
	const float buttonPadding = 6.f;
	const float buttonGap = 6.f;
	const float buttonTop = barTop + (barHeight - buttonSize) / 2.f;
	const float undoLeft = buttonPadding;
	const float redoLeft = undoLeft + buttonSize + buttonGap;

	auto pointInRect = [](int x, int y, float left, float top, float width, float height) {
		return x >= left && x <= (left + width) && y >= top && y <= (top + height);
	};

	sf::RectangleShape undoButton(sf::Vector2f(buttonSize, buttonSize));
	undoButton.setPosition(sf::Vector2f(undoLeft, buttonTop));
	undoButton.setFillColor(sf::Color(20, 20, 30));
	undoButton.setOutlineColor(sf::Color(120, 120, 140));
	undoButton.setOutlineThickness(1.f);

	sf::RectangleShape redoButton(sf::Vector2f(buttonSize, buttonSize));
	redoButton.setPosition(sf::Vector2f(redoLeft, buttonTop));
	redoButton.setFillColor(sf::Color(20, 20, 30));
	redoButton.setOutlineColor(sf::Color(120, 120, 140));
	redoButton.setOutlineThickness(1.f);

#if SFML_VERSION_MAJOR >= 3
	sf::Text undoLabel(font, "<", 18);
	sf::Text redoLabel(font, ">", 18);
#else
	sf::Text undoLabel;
	undoLabel.setFont(font);
	undoLabel.setCharacterSize(18);
	undoLabel.setString("<");

	sf::Text redoLabel;
	redoLabel.setFont(font);
	redoLabel.setCharacterSize(18);
	redoLabel.setString(">");
#endif

	undoLabel.setFillColor(sf::Color(220, 220, 230, 255));
	redoLabel.setFillColor(sf::Color(220, 220, 230, 255));
	undoLabel.setPosition(sf::Vector2f(undoLeft + 7.f, buttonTop + 2.f));
	redoLabel.setPosition(sf::Vector2f(redoLeft + 7.f, buttonTop + 2.f));

	auto refreshSearchInfo = [&]() {
		lastSearchPieces = countPieces(brd.state());
		lastSearchDepth = chooseSearchDepth(brd.state());
	};



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
					brd.unmakeMove(true);
				if (keyEvent->code == sf::Keyboard::Key::R)
					brd.redoMove();
				refreshSearchInfo();
				continue;
			}

			if (const auto *mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mouseEvent->button == sf::Mouse::Button::Left) {
					if (mouseY >= static_cast<int>(barTop)) {
						if (pointInRect(mouseX, mouseY, undoLeft, buttonTop, buttonSize, buttonSize))
							brd.unmakeMove(true);
						else if (pointInRect(mouseX, mouseY, redoLeft, buttonTop, buttonSize, buttonSize))
							brd.redoMove();
						refreshSearchInfo();
						continue;
					}
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
              brd.makeMove(moveFrom, moveTo, true);
              if (brd.isWPromotion() | brd.isBPromotion())
                brd.handlePromotion(BoardRenderer::showPromotionWindow(font));

              if (!brd.legalMoves(brd.onMovePositions())) {
                moveFrom = 0;
                continue;
              }

              lastSearchPieces = countPieces(brd.state());
              lastSearchDepth = chooseSearchDepth(brd.state());
              std::cout << "// ai search depth=" << lastSearchDepth
                        << " pieces=" << lastSearchPieces << std::endl;

							auto result = search.iterativeDeepening(brd, lastSearchDepth);
							std::cout << "// ai move " << squareName(result.second.first)
							          << " -> " << squareName(result.second.second)
							          << " eval=" << result.first << std::endl;

              if (result.second.first != 0 && result.second.second != 0) {
                brd.makeMove(result.second.first, result.second.second, true);
                if (brd.isWPromotion() | brd.isBPromotion()) {
                  char promo = search.lastPromotion();
                  if (promo == '\0') promo = 'Q';
                  brd.handlePromotion(promo);
                }
              }
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
						brd.unmakeMove(true);
					if (event.key.code == sf::Keyboard::R)
						brd.redoMove();
					refreshSearchInfo();
					break;

				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (mouseY >= static_cast<int>(barTop)) {
							if (pointInRect(mouseX, mouseY, undoLeft, buttonTop, buttonSize, buttonSize))
								brd.unmakeMove(true);
							else if (pointInRect(mouseX, mouseY, redoLeft, buttonTop, buttonSize, buttonSize))
								brd.redoMove();
							refreshSearchInfo();
							break;
						}
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
              brd.makeMove(moveFrom, moveTo, true);
              if (brd.isWPromotion() | brd.isBPromotion())
                brd.handlePromotion(BoardRenderer::showPromotionWindow(font));

              if (!brd.legalMoves(brd.onMovePositions())) {
                moveFrom = 0;
                break;
              }

              lastSearchPieces = countPieces(brd.state());
              lastSearchDepth = chooseSearchDepth(brd.state());
              std::cout << "// ai search depth=" << lastSearchDepth
                        << " pieces=" << lastSearchPieces << std::endl;

								auto result = search.iterativeDeepening(brd, lastSearchDepth);
								std::cout << "// ai move " << squareName(result.second.first)
													<< " -> " << squareName(result.second.second)
													<< " eval=" << result.first << std::endl;

                if (result.second.first != 0 && result.second.second != 0) {
                  brd.makeMove(result.second.first, result.second.second, true);
                  if (brd.isWPromotion() | brd.isBPromotion()) {
                    char promo = search.lastPromotion();
                    if (promo == '\0') promo = 'Q';
                    brd.handlePromotion(promo);
                  }
                }
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
			brd.handlePromotion(BoardRenderer::showPromotionWindow(font));


		window.clear(sf::Color::Black);

    // Test if there are any more moves
    const bool hasMoves = brd.legalMoves(brd.onMovePositions());
    if (hasMoves) {
      renderer.draw(window, moveFrom, font);
    } else {
      const bool inCheck = sideInCheck(brd.state());
      // If no legal moves, it is checkmate or stalemate.
      window.clear(brd.state().whiteToMove() ? sf::Color::Black : sf::Color::White);

      if (inCheck)
        victoryText.setString(brd.state().whiteToMove() ? "Black won" : "White won");
      else
        victoryText.setString("Draw");

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

		depthText.setString("AI depth: " + std::to_string(lastSearchDepth) +
		                    "  Pieces: " + std::to_string(lastSearchPieces));
		depthText.setPosition(sf::Vector2f(10.f, static_cast<float>(TILE * 8 + 6)));
		window.draw(depthText);
		window.draw(undoButton);
		window.draw(redoButton);
		window.draw(undoLabel);
		window.draw(redoLabel);

		window.display();
	}

	return 0;
}
