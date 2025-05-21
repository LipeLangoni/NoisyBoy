#include <iostream>
#include <chess.hpp>
#include <map>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <array>
#include <string>

using namespace chess;
const int MAX_DEPTH = 20;
const int MATE_VALUE = 10000;

inline std::vector<int> mirrorTable(const std::vector<int>& original) {
    if (original.size() != 64) {
        throw std::invalid_argument("mirrorTable() error: input vector size != 64");
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

int pieceSquaresVal(Bitboard piece, const std::string &type) {
    int score = 0;
    static const PieceTable pieceTable;

    auto it = pieceTable.tables.find(type);
    if (it == pieceTable.tables.end()) {
        throw std::invalid_argument("Invalid piece type: " + type);
    }
    
    const std::vector<int>& table = it->second;

    for (Bitboard bb = piece; bb; bb &= (bb.getBits() - 1)) {
        int square = __builtin_ctzll(bb.getBits());
        score += table[square];
    }
    return score;
}

struct PieceInfo {
    chess::PieceType type;
    int materialValue;
    const char* whiteKey;  
    const char* blackKey;
};

struct SearchInfo {
    std::vector<Move> pv;
    long long nodes;
    std::chrono::time_point<std::chrono::high_resolution_clock> max_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

static constexpr std::array<PieceInfo, 5> pieceInfos{{
    { chess::PieceType::PAWN,   100, "p", "P" },
    { chess::PieceType::KNIGHT, 300, "n", "N" },
    { chess::PieceType::BISHOP, 300, "b", "B" },
    { chess::PieceType::ROOK,   500, "r", "R" },
    { chess::PieceType::QUEEN,  900, "q", "Q" }
}};

inline const char* getKey(const PieceInfo &info, Color side) {
    return (side == Color::WHITE) ? info.whiteKey : info.blackKey;
}

inline bool is_endgame(Board &board) {
    int queens = 0;
    int minors = 0;
    queens += board.pieces(PieceType::QUEEN).count();
    minors += board.pieces(PieceType::KNIGHT).count();
    minors += board.pieces(PieceType::BISHOP).count();

    if (queens == 0 || (queens <= 2 && minors<=1)){
        return true;
    }

    return false;
}


int score(Board &board) {
    int totalScore = 0;

    Color us = board.sideToMove();
    Color them = ~us;

    for (const auto &info : pieceInfos) {
        auto ourPieces = board.pieces(info.type, us);
        auto theirPieces = board.pieces(info.type, them);

        int ourPieceCount = ourPieces.count();
        int theirPieceCount = theirPieces.count();
        totalScore += ourPieceCount * info.materialValue;
        totalScore -= theirPieceCount * info.materialValue;

        totalScore += pieceSquaresVal(ourPieces, getKey(info, us));
        totalScore -= pieceSquaresVal(theirPieces, getKey(info, them));
    }

    auto ourKing = board.pieces(PieceType::KING, us);
    auto theirKing = board.pieces(PieceType::KING, them);
    totalScore += pieceSquaresVal(ourKing, getKey({PieceType::KING, 0, "k", "K"}, us));
    totalScore -= pieceSquaresVal(theirKing, getKey({PieceType::KING, 0, "k", "K"}, them));

    return totalScore;
}

struct SearchTimeoutException : public std::exception {
    const char* what() const noexcept override {
        return "Search timeout";
    }
};

bool is_null_move_allowed(const Board &board) {
    Color sideToMove = board.sideToMove();

    Bitboard kingBitboard = board.pieces(PieceType::KING, sideToMove);
    Bitboard pawnBitboard = board.pieces(PieceType::PAWN, sideToMove);
    Bitboard allPiecesBitboard = board.us(sideToMove);

    if ((kingBitboard | pawnBitboard) == allPiecesBitboard) {
        return false;
    }

    int totalPieces = board.all().count();

    return totalPieces > 6;
}

bool should_stop(SearchInfo &info) {
    auto current_time = std::chrono::high_resolution_clock::now();
    return current_time > info.max_time;
}

std::chrono::milliseconds get_duration(std::chrono::time_point<std::chrono::high_resolution_clock> start) {
    auto current_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start);
}

int quisce(
    Board &board, 
    int alpha, 
    int beta, 
    int ply,
    SearchInfo &info
)
{
    info.nodes++;

    if (should_stop(info)) {
        return 0;
    }

    if (board.isRepetition()) {
        return 0;
    }

    int best = score(board);
    if (best >= beta) {
        return best;
    }

    if (best > alpha) {
        alpha = best;
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto& move : moves) {
        if (!board.isCapture(move)) {
            continue;
        }
    
        board.makeMove(move);
        int score = -quisce(board, -beta, -alpha, ply + 1, info);
        board.unmakeMove(move);
        
        if (should_stop(info)) {
            return 0;
        }
        if (score >= beta) {
            return score;
        }
        if (score > best) {
            best = score;
            if (score > alpha) {
                alpha = score;
                info.pv[ply] = move;
            }
        }        
    }

    return best;
}

int negamax(
    Board &board, 
    int alpha, 
    int beta,
    int depth, 
    int ply, 
    SearchInfo &info
)
{
    if (should_stop(info)) {
        return 0;
    }

    if (depth <= 0) {
        return quisce(board, alpha, beta, ply, info);
    }

    info.nodes++;

    if (board.isRepetition() && ply > 0) {
        return 0;
    }

    Movelist moves;
    movegen::legalmoves(moves, board); 

    if (moves.empty()) {
        return board.inCheck() ? -MATE_VALUE + ply : 0;
    }

    if (depth > 3 
        && !board.inCheck() 
        && is_null_move_allowed(board)
        && (beta - score(board) > 200)
    ){
        board.makeNullMove();
        int score = -negamax(board, -beta, -beta + 1, depth - 3, ply + 1, info);
        board.unmakeNullMove();

        if (score >= beta) {
            return score;
        }
    };

    int best_value = -MATE_VALUE;

    for (const auto& move : moves) {
        board.makeMove(move);
        int score = -negamax(board, -beta, -alpha, depth - 1, ply + 1, info);
        board.unmakeMove(move);

        if (should_stop(info)) {
            return 0;
        }
        if (score >= beta) {
            return score;
        }
        if (score > best_value) {
            best_value = score;
            if (score > alpha) {
                alpha = score;
                info.pv[ply] = move;
            }
        }
    }

    return best_value;
}

chess::Move noisy_boy(Board &board, int wtime = 0, int btime = 0, int winc = 0, int binc = 0) {
    Move best_move;
    SearchInfo info = SearchInfo();
    info.nodes = 0;
    info.pv.resize(MAX_DEPTH);

    auto time_remaining = (board.sideToMove() == Color::WHITE) ? wtime : btime;
    auto increment = (board.sideToMove() == Color::WHITE) ? winc : binc;

    auto start = std::chrono::high_resolution_clock::now();

    info.max_time = start +
        std::chrono::milliseconds(time_remaining / 40) + std::chrono::milliseconds(increment / 2);

    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        int alpha = -MATE_VALUE;
        int beta = MATE_VALUE;
        int best_value = -MATE_VALUE;

        int score = negamax(board, alpha, beta, depth, 0, info);
        auto duration = get_duration(start);
        auto current_time = std::chrono::high_resolution_clock::now();
        if (current_time > info.max_time) {
            break;
        }  

        auto nps = info.nodes / (get_duration(start).count() + 1);
        std::string pvLine = uci::moveToUci(best_move);

        std::cout << "info depth " << depth << " score cp " << score << " time " << duration.count() 
        << " nodes" << info.nodes << " nps " <<  nps << " pv " << pvLine << std::endl;

        if (score > best_value) {
            best_move = info.pv[0];
            best_value = score;   
        }
    }

    return best_move;
}

void uci_commands(Board &board, const std::string &message) {
    std::string msg = message;

    std::istringstream iss(msg);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (msg == "quit") {
        std::exit(0);  
    }

    if (msg == "uci") {
        std::cout << "id name NoisyBoy 0.1.1" << std::endl;
        std::cout << "id author Felipe Langoni Ramos" << std::endl;
        std::cout << "uciok" << std::endl;
        return;
    }

    if (msg == "isready") {
        std::cout << "readyok" << std::endl;
        return;
    }

    if (msg == "ucinewgame") {
        return;
    }

    if (msg == "setoption") {
        return;
    }

    if (tokens[0] == "position") {
        if (tokens.size() < 2) {
            return;  
        }

        size_t moves_start = 0;

        if (tokens[1] == "startpos") {
   
            board.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            moves_start = 2;
        } else if (tokens[1] == "fen") {
            std::string fen = "";
            for (size_t i = 2; i < 8; ++i) {
                fen += tokens[i] + " ";
            }
        
            board.setFen(fen);
            moves_start = 8;
        } else {
            return;  
        }

        if (tokens.size() <= moves_start || tokens[moves_start] != "moves") {
            return;  
        }

        for (size_t i = moves_start + 1; i < tokens.size(); ++i) {
            std::string move_str = tokens[i];
            Move move = uci::uciToMove(board,move_str);  
            board.makeMove(move);  
        }
    }

    if (msg == "d") {
        std::cout << board << std::endl;
        std::cout << board.getFen() << std::endl;
    }

    if (msg.substr(0, 2) == "go") {

        std::unordered_map<std::string, int> params = {
            {"wtime", 300000},
            {"btime", 300000},
            {"winc", 0},
            {"binc", 0}
        };
        
        std::istringstream iss(msg); 
        std::string token;
        
        for (size_t i = 0; i < 4; ++i) {
            iss >> token;
            if (token == "wtime") {
                iss >> params["wtime"];
            } else if (token == "btime") {
                iss >> params["btime"];
            } else if (token == "winc") {
                iss >> params["winc"];
            } else if (token == "binc") {
                iss >> params["binc"];
            }
        }
        auto start = std::chrono::high_resolution_clock::now();
        Move best_move = noisy_boy(board, params["wtime"], params["btime"],
                                   params["winc"], params["binc"]);  
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        
        std::cout << "bestmove " << uci::moveToUci(best_move)  << std::endl;
        std::cout << " (calc time " << duration << "s)" << std::endl;
    }
    if (msg.substr(0, 4) == "eval") {
        auto start = std::chrono::high_resolution_clock::now();
        int s = score(board);  
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        
        std::cout << "score: " << s
                  << " (calc time " << duration << "s)" << std::endl;
    }
    if (msg.substr(0, 4) == "side") {
        auto start = std::chrono::high_resolution_clock::now();
        int s = board.sideToMove();  
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        
        std::cout << "side: " << s
                  << " (calc time " << duration << "s)" << std::endl;
    }
}


int main() {
    std::string input;
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    while (true) {
        std::getline(std::cin, input);
        uci_commands(board,input);
    }
    
    return 0;
}

