#include <iostream>
#include <chess.hpp>
#include <map>
#include <vector>
#include <unordered_map>

using namespace chess;

std::vector<int> mirrorTable(const std::vector<int>& original) {
    if (original.size() != 64) {
        std::cerr << "mirrorTable() error: input vector size != 64\n";
        return std::vector<int>(64, 0);
    }
    std::vector<int> mirrored(64);
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int srcIndex = row * 8 + col;
            int destIndex = (7 - row) * 8 + col;
            mirrored[destIndex] = original[srcIndex];
        }
    }
    return mirrored;
}

struct PieceTable {
    std::unordered_map<std::string, std::vector<int>> tables;

    PieceTable() {
        // Pawn table
        tables["p"] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 10, 10, -40, -40, 10, 10, 5,
            5, -5, -10, 0, 0, -10, -5, 5,
            0, 0, 0, 50, 50, 0, 0, 0,
            5, 5, 10, 25, 25, 10, 5, 5,
            10, 10, 20, 30, 30, 20, 10, 10,
            50, 50, 50, 50, 50, 50, 50, 50,
            0, 0, 0, 0, 0, 0, 0, 0
        };

        // Knight table
        tables["n"] = {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20, 0, 5, 5, 0, -20, -40,
            -30, 5, 10, 15, 15, 10, 5, -30,
            -30, 0, 15, 20, 20, 15, 0, -30,
            -30, 5, 15, 20, 20, 15, 5, -30,
            -30, 0, 10, 15, 15, 10, 0, -30,
            -40, -20, 0, 0, 0, 0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        };

        // Bishop table
        tables["b"] = {
            -20, -10, -40, -10, -10, -40, -10, -20,
            -10, 5, 0, 0, 0, 0, 5, -10,
            -10, 10, 10, 10, 10, 10, 10, -10,
            -10, 0, 20, 10, 10, 20, 0, -10,
            -10, 5, 5, 10, 10, 5, 5, -10,
            -10, 0, 5, 10, 10, 5, 0, -10,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -20, -10, -40, -10, -10, -40, -10, -20
        };

        // Rook table
        tables["r"] = {
            0, 0, 0, 5, 5, 0, 0, 0,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            -5, 0, 0, 0, 0, 0, 0, -5,
            5, 10, 10, 10, 10, 10, 10, 5,
            0, 0, 0, 0, 0, 0, 0, 0
        };

        // Queen table
        tables["q"] = {
            -20, -10, -10, -5, -5, -10, -10, -20,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -10, 5, 5, 5, 5, 5, 0, -10,
            0, 0, 5, 5, 5, 5, 0, -5,
            -5, 0, 5, 5, 5, 5, 0, -5,
            -10, 0, 5, 5, 5, 5, 0, -10,
            -10, 0, 0, 0, 0, 0, 0, -10,
            -20, -10, -10, -5, -5, -10, -10, -20
        };

        // King table
        tables["k"] = {
            20, 30, 10, 0, 0, 10, 30, 20,
            20, 20, -10, -10, -10, -10, 20, 20,
            -10, -20, -20, -20, -20, -20, -20, -10,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30
        };
        tables["P"] = mirrorTable(tables["p"]);
        tables["N"] = mirrorTable(tables["n"]);
        tables["B"] = mirrorTable(tables["b"]);
        tables["R"] = mirrorTable(tables["r"]);
        tables["Q"] = mirrorTable(tables["q"]);
        tables["K"] = mirrorTable(tables["k"]);

    }
};

int pst_score(Board &board)
{
    int score = 0;
    score += board.pieces(PieceType::PAWN,Color::WHITE).count() * 1;
    score += board.pieces(PieceType::KNIGHT,Color::WHITE).count() * 3;
    score += board.pieces(PieceType::BISHOP,Color::WHITE).count() * 3;
    score += board.pieces(PieceType::ROOK,Color::WHITE).count() * 5;
    score += board.pieces(PieceType::QUEEN,Color::WHITE).count() * 9;

    score -= board.pieces(PieceType::PAWN,Color::BLACK).count() * 1;
    score -= board.pieces(PieceType::KNIGHT,Color::BLACK).count() * 3;
    score -= board.pieces(PieceType::BISHOP,Color::BLACK).count() * 3;
    score -= board.pieces(PieceType::ROOK,Color::BLACK).count() * 5;
    score -= board.pieces(PieceType::QUEEN,Color::BLACK).count() * 9;

    return score;
}

int PieceSquares(Bitboard piece, std::string type) {
    int score = 0;
 
    for (int square = 0; square < 64; ++square) {
        if ((piece >> square) & 1ULL) {
            score += PieceTable().tables[type][square];
        }
    }
    std::cout << std::endl;
    return score;
}

int score(Board &board)
{
    int score = 0;
    score += board.pieces(PieceType::PAWN,Color::WHITE).count() * 100;
    score += board.pieces(PieceType::KNIGHT,Color::WHITE).count() * 300;
    score += board.pieces(PieceType::BISHOP,Color::WHITE).count() * 300;
    score += board.pieces(PieceType::ROOK,Color::WHITE).count() * 500;
    score += board.pieces(PieceType::QUEEN,Color::WHITE).count() * 900;
    std::cout << "White score: " << score << std::endl;

    score -= board.pieces(PieceType::PAWN,Color::BLACK).count() * 100;
    score -= board.pieces(PieceType::KNIGHT,Color::BLACK).count() * 300;
    score -= board.pieces(PieceType::BISHOP,Color::BLACK).count() * 300;
    score -= board.pieces(PieceType::ROOK,Color::BLACK).count() * 500;
    score -= board.pieces(PieceType::QUEEN,Color::BLACK).count() * 900;
    std::cout << "Black score: " << score << std::endl;

    score += PieceSquares(board.pieces(PieceType::PAWN, Color::BLACK), "p");
    score += PieceSquares(board.pieces(PieceType::KNIGHT, Color::BLACK), "n");
    score += PieceSquares(board.pieces(PieceType::BISHOP, Color::BLACK), "b");
    score += PieceSquares(board.pieces(PieceType::ROOK, Color::BLACK), "r");
    score += PieceSquares(board.pieces(PieceType::QUEEN, Color::BLACK), "q");
    score += PieceSquares(board.pieces(PieceType::KING, Color::BLACK), "k");
    std::cout << "Black piece square score: " << score << std::endl;

    score -= PieceSquares(board.pieces(PieceType::PAWN, Color::WHITE), "P");
    score -= PieceSquares(board.pieces(PieceType::KNIGHT, Color::WHITE), "N");
    score -= PieceSquares(board.pieces(PieceType::BISHOP, Color::WHITE), "B");
    score -= PieceSquares(board.pieces(PieceType::ROOK, Color::WHITE), "R");
    score -= PieceSquares(board.pieces(PieceType::QUEEN, Color::WHITE), "Q");
    score -= PieceSquares(board.pieces(PieceType::KING, Color::WHITE), "K");
    std::cout << "White piece square score: " << score << std::endl;
    
    std::cout << "Score: " << score << std::endl;

    return score;
}
bool is_checkmate(Board &board) {
    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.size() == 0 && board.inCheck()) {
        return true;
    }
    return false;
}



int negamax(Board &board, int alpha, int beta, int ply)
{
    if (ply ==0){
        return score(board);
    }

    if (is_checkmate(board)) {
        if (board.sideToMove() == Color::WHITE) {
            return -10000 + ply;
        } else {
            return 10000 - ply;
        }
    }
    if (board.isRepetition()) {
        return 0;
    }

    int score = 0;
    int best_value = -1000;

    if (ply>=3 && !board.inCheck()){
        board.makeNullMove();
        score = -negamax(board, -beta, -beta+1, ply - 3);
        board.unmakeNullMove();

        if (score >= beta) {
            return score;
        }
    };

    Movelist moves;
    movegen::legalmoves(moves, board); 
    for (const auto& move : moves) {
        board.makeMove(move);
        score = -negamax(board, -beta, -alpha, ply - 1);
        board.unmakeMove(move);
        if (score >= beta) {
            return score;
        }
        if (score > best_value) {
            best_value = score;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return score;
}



int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int ply = 4;
    int best_value = -1000;
    Move best_move;
    int alpha = -1000;
    int beta = 1000;
    int score = 0;


    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto& move : moves) {
        board.makeMove(move);
        score = -negamax(board, -beta, -alpha, ply - 1);
        board.unmakeMove(move);


        if (score > best_value) {
            best_value = score;
            best_move = move;
        }
    }
    std::cout << "Best move: " << best_move << std::endl;
    std::cout << "Best value: " << best_value << std::endl;

    return 0;
}