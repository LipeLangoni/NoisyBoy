#include <iostream>
#include <chess.hpp>
#include <map>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <array>
#include <string>

using namespace chess;
const int MAX_DEPTH = 10;
inline std::vector<int> mirrorTable(const std::vector<int>& original) {
    assert(original.size() == 64 && "mirrorTable() error: input vector size != 64");
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


int PieceSquares(Bitboard piece, const std::string &type) {
    int score = 0;
    static const PieceTable pieceTable;
    auto it = pieceTable.tables.find(type);
    if (it == pieceTable.tables.end() || it->second.size() != 64) {
        std::cerr << "Invalid piece type: " << type << "\n";
        return score;
    }
    
    const std::vector<int>& table = it->second;
    for (int square = 0; square < 64; ++square) {
        if ((piece >> square) & 1ULL) {
            score += table[square];
        }
    }
    return score;
}

struct PieceInfo {
    chess::PieceType type;
    int materialValue;
    const char* whiteKey;  
    const char* blackKey;
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

int score(Board &board) {
    int score = 0;
    Color us = board.sideToMove();
    Color them = ~us;
    for (const auto &info : pieceInfos) {
        int sideCount = board.pieces(info.type, us).count();
        int opponentCount = board.pieces(info.type, them).count();
        score += sideCount * info.materialValue;
        score -= opponentCount * info.materialValue;
        score += PieceSquares(board.pieces(info.type, us), getKey(info, us));
        score -= PieceSquares(board.pieces(info.type, them), getKey(info, them));
    }

    score += PieceSquares(board.pieces(PieceType::KING, us), us == Color::WHITE ? "k" : "K");
    score -= PieceSquares(board.pieces(PieceType::KING, them), them == Color::WHITE ? "k" : "K");

    return score;
}

int quisce(Board &board, int alpha, int beta, 
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time,
    std::chrono::milliseconds max_time)
{
    auto current_time = std::chrono::high_resolution_clock::now();
    if (current_time - start_time > max_time) {
        return 0;
    }

    if (board.isRepetition()) {
        return 0;
    }

    int stand_pat = score(board);
    int best_value = stand_pat;

    if (stand_pat >= beta) {
        return stand_pat;
    }
    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto& move : moves) {
        if (board.isCapture(move)) {
            board.makeMove(move);
            int score = -quisce(board, -beta, -alpha, start_time, max_time);
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
    }

    return best_value;  
}




int negamax(Board &board, int alpha, int beta, int ply,std::chrono::time_point<std::chrono::high_resolution_clock>start_time,std::chrono::milliseconds max_time)
{
    
    auto now = std::chrono::high_resolution_clock::now();
    if (now - start_time > max_time) {
        return 0;
    }
    if (ply == 0){
        return quisce(board, alpha, beta,start_time,max_time);
    }

    // if (ply == 0){
    //     return score(board);
    // }

    if (board.isRepetition()) {
        return 0;
    }
    Movelist moves;
    movegen::legalmoves(moves, board); 
    
    if (moves.size() == 0) {
        if (board.inCheck()) {
            return -10000 + ply;
        } else {
            return 0;
        }
    }
    

    int score = 0;
    int best_value = -100000;

    if (ply>=3 && !board.inCheck()){
        board.makeNullMove();
        score = -negamax(board, -beta, -beta+1, ply - 3,start_time,max_time);
        board.unmakeNullMove();

        if (score >= beta) {
            return score;
        }
    };

    
    for (const auto& move : moves) {
        board.makeMove(move);
        score = -negamax(board, -beta, -alpha, ply - 1,start_time,max_time);
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

    return best_value;
}
chess::Move noisy_boy(Board &board,int wtime = 0, int btime = 0, int winc = 0, int binc = 0) {
    
    Move best_move;
    
    Movelist moves;
    movegen::legalmoves(moves, board);
    std::unordered_map<int, std::vector<std::tuple<chess::Move, int>>> pv_table;

    auto tms = (board.sideToMove()==Color::WHITE ? wtime : btime);
    auto inc = (board.sideToMove()==Color::WHITE ? winc  : binc);
    std::chrono::milliseconds max_time = 
    std::chrono::milliseconds(tms / 40) + std::chrono::milliseconds(inc / 2);
    int lastDepth = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (int ply = 1; ply <= MAX_DEPTH; ++ply) {
        int alpha = -100000;
        int beta = 100000;
        int score = 0;
        int best_value = -100000;

        lastDepth = ply;
        auto current_time = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start).count() > max_time.count()) {
            return best_move;
        }
        std::vector<std::tuple<Move, int>> sorted_moves;
        if (ply == 1) {
            for (const auto& move : moves) {
                board.makeMove(move);
                score = -negamax(board, -beta, -alpha, ply - 1,start,max_time);
                board.unmakeMove(move);

                if (score > best_value) {
                    best_value = score;
                    best_move = move;
                }
                sorted_moves.emplace_back(move, score);
            }
            std::sort(sorted_moves.begin(), sorted_moves.end(),
            [](const std::tuple<Move, int>& a, const std::tuple<Move, int>& b) {
                return std::get<1>(a) > std::get<1>(b);
            });
            pv_table[ply] = sorted_moves;
        } else {
            for (auto& move_tuple : pv_table[ply-1]) {
                board.makeMove(std::get<0>(move_tuple));
                score = -negamax(board, -beta, -alpha, ply - 1,start,max_time);
                board.unmakeMove(std::get<0>(move_tuple));

                if (score > best_value) {
                    best_value = score;
                    best_move = std::get<0>(move_tuple);
                }
                sorted_moves.emplace_back(std::get<0>(move_tuple), score);
            }
            std::sort(sorted_moves.begin(), sorted_moves.end(),
            [](const std::tuple<Move, int>& a, const std::tuple<Move, int>& b) {
                return std::get<1>(a) > std::get<1>(b);
            });
            pv_table[ply] = sorted_moves;
            
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
        std::cout << "id name NoisyBoy" << std::endl;
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
        
        std::cout << "bestmove " << uci::moveToUci(best_move)
                  << " (calc time " << duration << "s)" << std::endl;
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

