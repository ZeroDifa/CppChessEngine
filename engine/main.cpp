#include <iostream>
#include <thread>
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <list>
#include <unordered_set>
#include <algorithm>
#include <atomic>

#include "SimpleJSON.h"
#include "utils.h"
#include "constants.h"

#include "chessdesk.h"
#include "thearpool.h"
#include "configflagmanager.h"

#include "robin_hood.h"

using namespace std;

ZobristHash zobristHash;
std::mutex mtx;


ConfigFlagManager CONFIG = ConfigFlagManager();


TranspositionTable transpositionTable(TRANSPOSITION_TABLE_MAX_SIZE);

uint64_t computeHash(const ChessDesk& chessDesk) {
    return zobristHash.hash(chessDesk.desk);
}

std::atomic<bool> checkmateFound(false);

double minmax(int depth, ChessDesk& chessDesk, double alpha, double beta, int color, bool prolongation, double calculatedStatus) {
    positionsCount++;
    const int MAX_R = 3;
    const int MIN_R = 2;
    int R = 2;
    if (depth >= R && !prolongation && !chessDesk.isCheck(color)) {
        ChessDesk tempchessDesk = ChessDesk(chessDesk);
        double nullMoveValue = -minmax(depth - 1 - R, tempchessDesk, -beta, -beta + 1, color, false, calculatedStatus);
        if (nullMoveValue >= beta) 
            return nullMoveValue;
    }
    
    if ((depth < 0 && !prolongation) || depth < MAX_DEPTH * -1 || (checkmateFound.load())) {
        return calculatedStatus;
    }
    
    uint64_t hash = computeHash(chessDesk);
    TranspositionTableEntry entry;
    if (transpositionTable.lookup(hash, entry)) {
        if (entry.depth <= depth) {
            if (entry.flag == 0) return entry.value;
            if (entry.flag == -1 && entry.value <= alpha) return alpha;
            if (entry.flag == 1 && entry.value >= beta) return beta;
        }
    }

    int sign = color == WHITE ? 1 : -1;
    
    std::vector<Move> available_moves = chessDesk.getAllMoves(color);
    Move bestMoveFound;

    if (available_moves.size() == 0) {
        return color == WHITE ? -10000 : 10000;
    }

    std::vector<MoveForMinmax> moves;
    for (auto& move : available_moves) {
        bool isKill = chessDesk.move(move.from, move.to);
        double status = chessDesk.status();
        moves.push_back(MoveForMinmax{move, status, false, isKill});
        chessDesk.undo();
    }

    if (CONFIG.get("--no_sort"))
    {
        std::sort(moves.begin(), moves.end(), [color](const MoveForMinmax& a, const MoveForMinmax& b) {
            return color == WHITE ? a.status > b.status : a.status < b.status;
        });
        
    }
    else
    {
        std::sort(moves.begin(), moves.end(), [color](const MoveForMinmax& a, const MoveForMinmax& b) {
            if (a.isKill != b.isKill) {
                return a.isKill > b.isKill;
            }
            if (a.status != b.status) {
                return color == WHITE ? a.status > b.status : a.status < b.status;
            }
            return false;
        });
    }

    double bestMove = color == WHITE ? INT_MIN : INT_MAX;
    
    
    int i = 0;
    bool isFirstMove = true;
    bool firstMove = true;
    for (MoveForMinmax& move : moves) {
        bool isKilled = chessDesk.move(move.move.from, move.move.to);
        double minmaxResult;
        if (firstMove) {
            minmaxResult = minmax(depth - 1, chessDesk, alpha, beta, color == WHITE ? BLACK : WHITE, isKilled, move.status);
            firstMove = false;
        } else {
            if (depth >= 3 && !isKilled) {
                minmaxResult = minmax(depth - 2, chessDesk, alpha, beta, color == WHITE ? BLACK : WHITE, isKilled, move.status);
            } else {
                minmaxResult = minmax(depth - 1, chessDesk, alpha, beta, color == WHITE ? BLACK : WHITE, isKilled, move.status);
            }
        }
        if (color == WHITE) {
            bestMove = std::max(bestMove, minmaxResult);
            alpha = std::max(alpha, bestMove);
        } else {
            bestMove = std::min(bestMove, minmaxResult);
            beta = std::min(beta, bestMove);
        }
        chessDesk.undo();

        if (beta <= alpha) break;
    }

    entry.value = bestMove;
    entry.depth = depth;
    if (bestMove <= alpha) {
        entry.flag = -1;
    } else if (bestMove >= beta) {
        entry.flag = 1;
    } else {
        entry.flag = 0;
    }
    transpositionTable.store(hash, entry);
    

    return bestMove;
}
struct MinmaxMainResult {
    double bestValue;
    Move bestMove;
};

MinmaxMainResult minmaxMain(ChessDesk& chessDesk, int color, int depth, double alpha, double beta) {
    std::vector<Move> allMoves = chessDesk.getAllMoves(color);

    double bestMove = color == WHITE ? INT_MIN : INT_MAX;
    Move bestMoveFound;

    std::vector<MoveForMinmax> moves(allMoves.size());
    size_t i = 0;
    int sign = color == WHITE ? 1 : -1;
    int tempDepth = MAX_DEPTH;
    MAX_DEPTH = 3;
    for (auto& move : allMoves) {
        ChessDesk tempChessDesk = ChessDesk(chessDesk);
        bool isKill = tempChessDesk.move(move.from, move.to);
        double status = chessDesk.status();
        bool isCheck = tempChessDesk.isCheck(color == WHITE ? BLACK : WHITE);
        moves[i] = {move, status, tempChessDesk, isCheck, isKill};
        i++;
    }
    MAX_DEPTH = tempDepth;

    if (CONFIG.get("--no_sort"))
    {
        std::sort(moves.begin(), moves.end(), [color](const MoveForMinmax& a, const MoveForMinmax& b) {
                return color == WHITE ? a.status > b.status : a.status < b.status;
        });
        
    }
    else
    {
        std::sort(moves.begin(), moves.end(), [color](const MoveForMinmax& a, const MoveForMinmax& b) {
            if (a.isKill != b.isKill) {
                return a.isKill > b.isKill;
            }
            if (a.status != b.status) {
                return color == WHITE ? a.status > b.status : a.status < b.status;
            }
            return false;
        });
    }
    

    size_t numThreads = std::thread::hardware_concurrency();
    if (CONFIG.get("--no_threads"))
        numThreads = 2;
    stats["threadsCount"] = JSONValue((int)numThreads/2);
    ThreadPool pool(numThreads);
    std::vector<std::future<void>> futures;
    i = (size_t)0;
    for (MoveForMinmax& move : moves) {
        if (i == 0) {
            i++;
            continue;
        }
        futures.emplace_back(pool.enqueue([&] {
            if (checkmateFound.load()) {
                return; // Если мат найден, выходим раньше
            }
            double value = minmax(depth, move.desk, alpha, beta, color == WHITE ? BLACK : WHITE, false, move.status);
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (checkmateFound.load()) {
                    return;
                }
                if ((color == WHITE && value >= bestMove) || (color == BLACK && value <= bestMove)) {
                    if (abs(value) == 10000 && color == (value > 0 ? WHITE : BLACK))
                        checkmateFound.store(true);

                    bestMove = value;
                    bestMoveFound = move.move;
                    JSONValue::JSONObject iWant;
                    JSONValue::JSONArray pos;
                    pos.push_back(JSONValue(move.move.from));
                    pos.push_back(JSONValue(move.move.to));
                    iWant["move"] = JSONValue(pos);
                    iWant["type"] = JSONValue("wantPos");
                    iWant["value"] = JSONValue(value);
                    cout << JSONParser::stringify(iWant) << endl;
                }
            }
        }));
    }

    for (auto& future : futures) {
        future.get();
    }
    
    if (bestMoveFound.from == 0 && bestMoveFound.to == 0) {
        throw std::runtime_error("Best move not found");
    }
    return {bestMove, bestMoveFound};
}

Move iterativeDeepening(ChessDesk& chessDesk, int color, int maxDepth, int timeLimit) {
    auto startTime = std::chrono::steady_clock::now();
    Move bestMove;
    MinmaxMainResult res;
    double alpha;
    double beta;
    // return minmaxMain(chessDesk, color, maxDepth, alpha, beta);
    double aspirationWindow = 40; // Initial aspiration window
    double bestValue = 0;
    int maxInterDepth = 0;
    for (int depth = 1; depth <= maxDepth; ++depth) {
        checkmateFound = false;
        alpha = bestValue - aspirationWindow;
        beta = bestValue + aspirationWindow;
        res = minmaxMain(chessDesk, color, depth, alpha, beta);
        if (res.bestValue <= alpha || res.bestValue >= beta) {
            alpha = -100000;
            beta = 100000;
            res = minmaxMain(chessDesk, color, depth, alpha, beta);
        }
        bestMove = res.bestMove;
        bestValue = res.bestValue;
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        maxInterDepth = depth;
        if (elapsedTime > timeLimit) break;
        
    }
    stats["checkmateFound"] = JSONValue((int)checkmateFound.load());
    stats["maxDepth"] = JSONValue(maxInterDepth);
    return bestMove;
}
int main(int argc, char* argv[]) {
    ChessDesk mainChessDesk = ChessDesk(INITIAL_SETUP, INIT_COLOR);
    
    for (int i = 0; i < argc; i++) {
        CONFIG.set(string(argv[i]));
    }

    string input;
    while (true) {
        std::getline(std::cin, input); // Считать данные из stdin
        std::istringstream iss(input);
        string command, buffer;
        iss >> command;
        if (command == "exit") {
            break;
        } else if (command == "pretty-piece-positions")
        {
            const robin_hood::unordered_map<int, vector<int>>& piecePositions = mainChessDesk.piecePositions;
            for (auto& piece : piecePositions) {
                if (piece.second.size() == 0 || piece.first == EMPTY) continue;
                cout << SYMBOLS.at(static_cast<int>(piece.first)) << ": ";
                for (auto& position : piece.second) {
                    cout << static_cast<int>(position) << " ";
                }
                cout << std::endl;
            }
        } else if (command == "get-allow-moves") {
            iss >> buffer;
            if ((mainChessDesk.desk[atoi(buffer.c_str())] & COLOR_SHIFT) != mainChessDesk.stepColor) {
                JSONValue::JSONObject result;
                JSONValue::JSONArray movesJson;
                result["moves"] = JSONValue(movesJson);
                result["type"] = JSONValue("get-allow-moves");
                cout << JSONParser::stringify(result);
                continue;
            }
            vector<int> moves = mainChessDesk.getAllowCages(atoi(buffer.c_str()));
            JSONValue::JSONObject result;
            JSONValue::JSONArray movesJson;
            for (auto& move : moves) {
                movesJson.push_back(JSONValue(move));
            }
            result["moves"] = JSONValue(movesJson);
            result["type"] = JSONValue("get-allow-moves");
            cout << JSONParser::stringify(result);

        } else if (command == "print") {
            mainChessDesk.printBoard();
        } else if (command == "get-current") {
            cout << mainChessDesk.toJson();
        } else if (command == "move") {
            string from, to;
            iss >> from >> to;
            int fromInt = atoi(from.c_str());
            int toInt = atoi(to.c_str());
            vector<int> moves = mainChessDesk.getAllowCages(fromInt);
            if (find(moves.begin(), moves.end(), toInt) == moves.end()) {
                cout << "Invalid move" << endl;
                continue;
            }
            mainChessDesk.move(fromInt, toInt, false, true);
            JSONValue::JSONObject result;
            JSONValue::JSONArray desk;
            for (int i = 0; i < 64; i++)
            {
                desk.push_back(JSONValue(mainChessDesk.desk[i]));
            }
            result["status"] = JSONValue(mainChessDesk.status());
            result["type"] = JSONValue("get-current");
            result["desk"] = JSONValue(desk);
            result["stepColor"] = JSONValue(mainChessDesk.stepColor);
            JSONValue::JSONObject move;
            move["from"] = JSONValue(atoi(from.c_str()));
            move["to"] = JSONValue(atoi(to.c_str()));
            result["move"] = JSONValue(move);
            cout << JSONParser::stringify(result);
        } else if (command == "check") {
            string colorConstsntName;
            iss >> colorConstsntName;
            int color = colorConstsntName == "w" ? WHITE : BLACK;
            cout << mainChessDesk.isCheck(color);
        } else if (command == "undo") {
            mainChessDesk.undo();
            cout << mainChessDesk.toJson();
        } else if (command == "status") {
            cout << "Main desk status: " << mainChessDesk.status() << endl;
        } else if (command == "do") {
            positionsCount = 0;
            auto start_time = std::chrono::steady_clock::now();
            string depth, prolongation;
            iss >> depth >> prolongation;
            MAX_DEPTH = atoi(prolongation.c_str());
            // Move bestMove = minmaxMain(mainChessDesk, mainChessDesk.stepColor, atoi(depth.c_str()));
            Move bestMove = iterativeDeepening(mainChessDesk, mainChessDesk.stepColor, atoi(depth.c_str()), 5000);
            auto end_time = std::chrono::steady_clock::now();

            mainChessDesk.move(bestMove.from, bestMove.to);

            auto elapsed_ns = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            JSONValue::JSONObject result;
            JSONValue::JSONArray desk;
            for (int i = 0; i < 64; i++)
            {
                desk.push_back(JSONValue(mainChessDesk.desk[i]));
            }
            result["type"] = JSONValue("do");
            result["desk"] = JSONValue(desk);
            result["stepColor"] = JSONValue(mainChessDesk.stepColor);
            result["time"] = JSONValue(elapsed_ns.count() / 1000.);
            result["funcTime"] = JSONValue(total_duration.count() / 1000);
            result["status"] = JSONValue(mainChessDesk.status());
            JSONValue::JSONObject bestMoveJson;
            bestMoveJson["from"] = JSONValue(bestMove.from);
            bestMoveJson["to"] = JSONValue(bestMove.to);
            result["move"] = JSONValue(bestMoveJson);
            stats["hashSize"] =  JSONValue((int)transpositionTable.table.size());
            stats["positionsCount"] = JSONValue(positionsCount);
            stats["speed"] = JSONValue(positionsCount / (elapsed_ns.count() / 1000.));
            result["stats"] = stats;
            JSONValue::JSONObject temp(result);
            cout << JSONParser::stringify(temp);
            stats.clear();
            if (transpositionTable.table.size() > TRANSPOSITION_TABLE_MAX_SIZE) {
                transpositionTable.clear();
            }
        } else if (command == "piece-p") {
            // const unordered_map<int, vector<int>>& piecePositions = mainChessDesk.piecePositions;
            // for (auto& piece : piecePositions) {
            //     cout << SYMBOLS.at(static_cast<int>(piece.first)) << ": ";
            //     for (auto& position : piece.second) {
            //         cout << static_cast<int>(position) << " ";
            //     }
            //     cout << std::endl;
            // }
        }
        else 
        {
            std::cout << "Unknown command: " << command << std::endl;
        }
        std::cout.flush();
    }
    return 0;
}


