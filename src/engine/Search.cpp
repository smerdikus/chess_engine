//
// Alpha-beta search with move ordering and transposition table.
//

#include "engine/Search.h"
#include "engine/Zobrist.h"

#include <algorithm>

namespace {
  constexpr uint8_t TT_EXACT = 0;
  constexpr uint8_t TT_LOWER = 1;
  constexpr uint8_t TT_UPPER = 2;
  constexpr int SEARCH_INF = 1'000'000'000;
  constexpr int MATE_SCORE = 1'000'000;
  constexpr int TT_MOVE_BONUS = 1'000'000;
  constexpr int KILLER1_BONUS = 900'000;
  constexpr int KILLER2_BONUS = 800'000;

  inline bool sideInCheck(const chs::BoardState &state) {
    return state.whiteToMove()
                   ? (chs::Board::wKingSafe(state, state.wKing) == 0)
                   : (chs::Board::bKingSafe(state, state.bKing) == 0);
  }
}

namespace chs {

	Search::Search(size_t ttSize, int noPrunePieces) : m_tt(ttSize), m_noPrunePieces(noPrunePieces) {}

	uint64_t Search::positionKey(const BoardState &state) const {
		return Zobrist::instance().hashBoard(state);
	}

	Search::TTEntry *Search::probeTT(uint64_t key) {
		TTEntry &entry = m_tt[key & (m_tt.size() - 1)];

		if (entry.key == key) return &entry;

		return nullptr;
	}

	void Search::storeTT(uint64_t key, int depth, int score, uint8_t flag, Bitboard bestFrom, Bitboard bestTo) {
		TTEntry &entry = m_tt[key & (m_tt.size() - 1)];
		if (entry.key != key || depth >= entry.depth) {
			entry.key = key;
			entry.depth = depth;
			entry.score = score;
			entry.flag = flag;
			entry.bestFrom = bestFrom;
			entry.bestTo = bestTo;
		}
	}

  int Search::pieceValueAt(const BoardState &state, Bitboard square, bool isWhite) const {
    const Bitboard pawns = isWhite ? state.wPawns : state.bPawns;
    const Bitboard knights = isWhite ? state.wKnights : state.bKnights;
    const Bitboard bishops = isWhite ? state.wBishops : state.bBishops;
    const Bitboard rooks = isWhite ? state.wRooks : state.bRooks;
    const Bitboard queens = isWhite ? state.wQueens : state.bQueens;

    int value = 0;
    value += static_cast<int>((square & pawns) != 0) * PAWN_VALUE;
    value += static_cast<int>((square & knights) != 0) * KNIGHT_VALUE;
    value += static_cast<int>((square & bishops) != 0) * BISHOP_VALUE;
    value += static_cast<int>((square & rooks) != 0) * ROOK_VALUE;
    value += static_cast<int>((square & queens) != 0) * QUEEN_VALUE;
    return value;
  }

	std::vector<Search::ScoredMove> Search::orderedMoves(Board &board, const TTEntry *ttEntry, int ply) {
		const BoardState &state = board.state();
		auto moves = board.generateMoves(board.onMovePositions());
		std::vector<ScoredMove> scored;
		scored.reserve(moves.size());

		const bool isWhite = state.whiteToMove();
		const Bitboard pawns = isWhite ? state.wPawns : state.bPawns;
		const Bitboard opponent = isWhite ? state.black() : state.white();
		const Bitboard king = isWhite ? state.wKing : state.bKing;
		const Bitboard promotionRank = isWhite ? RANK_8 : RANK_1;
		const Bitboard castlingTargets = isWhite ? (C1 | G1) : (C8 | G8);
		const Bitboard enPassant = state.enPassant;
		const int color = isWhite ? 0 : 1;
		const KillerMove *killer1 = (ply < MAX_PLY) ? &m_killers[ply][0] : nullptr;
		const KillerMove *killer2 = (ply < MAX_PLY) ? &m_killers[ply][1] : nullptr;

		for (const auto &move: moves) {
			int score = 0;
			if (ttEntry && ttEntry->bestFrom == move.first && ttEntry->bestTo == move.second)
				score += TT_MOVE_BONUS;

			const bool isPromotion = (move.first & pawns) && (move.second & promotionRank);
			const bool isCastling = (move.first & king) && (move.second & castlingTargets);
			const bool isEnPassant = (move.first & pawns) && (move.second & enPassant);
			const bool isCapture = isEnPassant || (move.second & opponent);

			if (isCapture) {
				const int movingValue = isEnPassant ? PAWN_VALUE : pieceValueAt(state, move.first, isWhite);
				const int capturedValue = isEnPassant ? PAWN_VALUE : pieceValueAt(state, move.second, !isWhite);
				score += 10 * capturedValue - movingValue;
			} else {
				if (killer1 && killer1->from == move.first && killer1->to == move.second)
					score += KILLER1_BONUS;
				else if (killer2 && killer2->from == move.first && killer2->to == move.second)
					score += KILLER2_BONUS;
				score += m_history[color][squareIndex(move.first)][squareIndex(move.second)];
			}

			if (isPromotion)
				score += QUEEN_VALUE;
			if (isCastling)
				score += 50;

			scored.push_back({move.first, move.second, score});
		}

		std::sort(scored.begin(), scored.end(), [](const ScoredMove &a, const ScoredMove &b) { return a.score > b.score; });
		return scored;
	}

	void Search::recordHistory(bool isWhite, Bitboard from, Bitboard to, int depth) {
		const int color = isWhite ? 0 : 1;
		m_history[color][squareIndex(from)][squareIndex(to)] += depth * depth;
	}

	void Search::recordKiller(Bitboard from, Bitboard to, int ply) {
		if (ply >= MAX_PLY)
			return;
		KillerMove &slot0 = m_killers[ply][0];
		if (slot0.from == from && slot0.to == to)
			return;
		m_killers[ply][1] = slot0;
		slot0 = {from, to};
	}

	int Search::negamaxSearch(Board &board, int depth, int alpha, int beta, Bitboard &bestFrom, Bitboard &bestTo,
	                          char &bestPromotion, int ply) {
		bestPromotion = '\0';
		const BoardState &state = board.state();
		if (depth <= 0) {
			auto moves = board.generateMoves(board.onMovePositions());
			if (moves.empty()) {
				if (sideInCheck(state))
					return -MATE_SCORE + ply;
				return 0;
			}
			return board.evaluate();
		}

		const int pieceCount = popcount(state.white() | state.black());
		const bool noPrune = pieceCount <= m_noPrunePieces;

		uint64_t key = positionKey(state);
		TTEntry *ttEntry = probeTT(key);
		int alphaOrig = alpha;

		if (!noPrune && ttEntry && ttEntry->depth >= depth) {
			if (ttEntry->flag == TT_EXACT) return ttEntry->score;
			if (ttEntry->flag == TT_LOWER) alpha = std::max(alpha, ttEntry->score);
			else if (ttEntry->flag == TT_UPPER) beta = std::min(beta, ttEntry->score);

			if (alpha >= beta) return ttEntry->score;
		}

		auto moves = orderedMoves(board, ttEntry, ply);
		if (moves.empty()) {
			if (sideInCheck(state))
				return -MATE_SCORE + ply;
			return 0;
		}

		const bool inCheck = sideInCheck(state);
		const bool isPV = (beta - alpha > 1);
		const Bitboard minorMajor = state.wKnights | state.wBishops | state.wRooks | state.wQueens |
		                            state.bKnights | state.bBishops | state.bRooks | state.bQueens;
		const bool zugzwangRisk = (minorMajor == 0) || isEndgame(state, 10);

		// Null-move pruning: depth>=3, not in check, not PV, and avoid zugzwang-prone endgames.
		if (depth >= 3 && !inCheck && !isPV && !zugzwangRisk) {
			const int R = (depth >= 6) ? 3 : 2;
			if (board.makeNullMove()) {
				Bitboard nullFrom = 0;
				Bitboard nullTo = 0;
				char nullPromotion = '\0';
				int score = -negamaxSearch(board, depth - 1 - R, -beta, -beta + 1, nullFrom, nullTo, nullPromotion, ply + 1);
				board.unmakeNullMove();
				if (score >= beta)
					return score;
			}
		}

		int bestScore = -SEARCH_INF;
		const bool isWhite = state.whiteToMove();
		const Bitboard pawns = isWhite ? state.wPawns : state.bPawns;
		const Bitboard opponent = isWhite ? state.black() : state.white();
		const Bitboard king = isWhite ? state.wKing : state.bKing;
		const Bitboard promotionRank = isWhite ? RANK_8 : RANK_1;
		const Bitboard castlingTargets = isWhite ? (C1 | G1) : (C8 | G8);
		const Bitboard enPassant = state.enPassant;

		int moveIndex = 0;
		for (const auto &move: moves) {
			const bool isEnPassant = (move.from & pawns) && (move.to & enPassant);
			const bool isCapture = isEnPassant || (move.to & opponent);
			const bool isPromotion = (move.from & pawns) && (move.to & promotionRank);
			const bool isCastling = (move.from & king) && (move.to & castlingTargets);

			Bitboard childBestFrom = 0;
			Bitboard childBestTo = 0;
			char childPromotion = '\0';
			int eval = 0;
			char movePromotion = '\0';
			bool doLmr = (depth >= 3) && !inCheck && !isPV && (moveIndex >= 4) && !isCapture && !isPromotion;
			if (doLmr) {
				// LMR: reduce only late, quiet moves that do not give check.
				board.makeMove(move.from, move.to);
				if (sideInCheck(board.state()))
					doLmr = false;
				board.unmakeMove();
			}

			if (isPromotion) {
				const char promoPieces[] = {'Q', 'R', 'B', 'N'};
				int bestPromoScore = -SEARCH_INF;
				for (char promo: promoPieces) {
					board.makeMove(move.from, move.to);
					board.handlePromotion(promo);
					int score = -negamaxSearch(board, depth - 1, -beta, -alpha, childBestFrom, childBestTo,
					                           childPromotion, ply + 1);
					board.unmakeMove();
					if (score > bestPromoScore) {
						bestPromoScore = score;
						movePromotion = promo;
					}
				}
				eval = bestPromoScore;
			} else {
				board.makeMove(move.from, move.to);
				if (doLmr) {
					const int r = (depth >= 6 && moveIndex >= 8) ? 2 : 1;
					int reduced = -negamaxSearch(board, depth - 1 - r, -alpha - 1, -alpha, childBestFrom, childBestTo,
					                             childPromotion, ply + 1);
					if (reduced > alpha)
						eval = -negamaxSearch(board, depth - 1, -beta, -alpha, childBestFrom, childBestTo,
						                      childPromotion, ply + 1);
					else
						eval = reduced;
				} else {
					eval = -negamaxSearch(board, depth - 1, -beta, -alpha, childBestFrom, childBestTo,
					                      childPromotion, ply + 1);
				}
				board.unmakeMove();
			}

			if (eval > bestScore) {
				bestScore = eval;
				bestFrom = move.from;
				bestTo = move.to;
				bestPromotion = movePromotion;
			}

			if (eval > alpha) alpha = eval;
			if (!noPrune && alpha >= beta) {
				if (!(isCapture || isPromotion || isCastling)) {
					recordKiller(move.from, move.to, ply);
					recordHistory(isWhite, move.from, move.to, depth);
				}
				break;
			}

			moveIndex++;
		}

		uint8_t flag = TT_EXACT;
		if (!noPrune) {
			if (bestScore <= alphaOrig) flag = TT_UPPER;
			else if (bestScore >= beta) flag = TT_LOWER;
		}

		storeTT(key, depth, bestScore, flag, bestFrom, bestTo);

		return bestScore;
	}

	std::pair<int, std::pair<Bitboard, Bitboard>> Search::negamax(Board &board, int depth) {
		Bitboard bestFrom = 0;
		Bitboard bestTo = 0;
		char bestPromotion = '\0';
		int score = negamaxSearch(board, depth, -SEARCH_INF, SEARCH_INF, bestFrom, bestTo, bestPromotion, 0);
		m_lastPromotion = bestPromotion;
		return {score, {bestFrom, bestTo}};
	}

	std::pair<int, std::pair<Bitboard, Bitboard>> Search::iterativeDeepening(Board &board, int maxDepth) {
		std::pair<int, std::pair<Bitboard, Bitboard>> best = {0, {0, 0}};
		char bestPromotion = '\0';
		for (int depth = 1; depth <= maxDepth; ++depth) {
			Bitboard bestFrom = 0;
			Bitboard bestTo = 0;
			char depthPromotion = '\0';
			int score = negamaxSearch(board, depth, -SEARCH_INF, SEARCH_INF, bestFrom, bestTo, depthPromotion, 0);
			if (bestFrom != 0 && bestTo != 0) {
				best = {score, {bestFrom, bestTo}};
				bestPromotion = depthPromotion;
			} else if (depth == 1) {
				best = {score, {bestFrom, bestTo}};
				bestPromotion = depthPromotion;
			}
		}
		m_lastPromotion = bestPromotion;
		return best;
	}

} // namespace chs
