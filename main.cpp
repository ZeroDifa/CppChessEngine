#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include "constants.h"
#include "chessdesk.h"
#include <chrono>

using namespace std;


double minmax(int depth, ChessDesk& chessDesk, double alpha, double beta, int color, bool prolongation = false, double calculatedStatus = 0) {
    positionsCount++;
    if ((depth < 0 && !prolongation) || depth < MAX_DEPTH*-1) {
        return calculatedStatus;
    }



    vector<Move> available_moves = chessDesk.getAllMoves(color);
    Move bestMoveFound;

    vector<MoveForMinmax> moves;
    for (auto& move : available_moves) {

        chessDesk.move(move.from, move.to);
        moves.push_back({move, chessDesk.status()});
        chessDesk.undo();
        

    }

    if (color == WHITE) {
        sort(moves.begin(), moves.end(), [](const MoveForMinmax& a, const MoveForMinmax& b) {
            return a.status > b.status;
        });
    } 
    else 
    {
        sort(moves.begin(), moves.end(), [](const MoveForMinmax& a, const MoveForMinmax& b) {
            return a.status < b.status;
        });
    }

    double bestMove = color == WHITE ? INT_MIN : INT_MAX;

    if (color == WHITE) {
        for (MoveForMinmax& move : moves) {
            bool isKilled = chessDesk.move(move.move.from, move.move.to);
            double minmaxResult = minmax(depth - 1, chessDesk, alpha, beta, color == WHITE ? BLACK : WHITE, isKilled, move.status);
            bestMove = max(bestMove, minmaxResult);
            chessDesk.undo();

            alpha = max(alpha, bestMove);
            if (beta <= alpha) 
                return bestMove;
        }
    } else {
        for (MoveForMinmax& move : moves) {
            bool isKilled = chessDesk.move(move.move.from, move.move.to);
            double minmaxResult = minmax(depth - 1, chessDesk, alpha, beta, color == WHITE ? BLACK : WHITE, isKilled, move.status);
            bestMove = min(bestMove, minmaxResult);
            chessDesk.undo();

            beta = min(beta, bestMove);
            if (beta <= alpha) 
                return bestMove;
        }
    }
    return bestMove;
}
void threadFunction ()
{

}
Move minmaxMain(ChessDesk& chessDesk, int color, int depth) {
    vector<Move> allMoves = chessDesk.getAllMoves(color);

    double bestMove = color == WHITE ? INT_MIN : INT_MAX;

    Move bestMoveFound;

    vector<MoveForMinmax> moves(allMoves.size());
    size_t i = 0;
    for (auto& move : allMoves) {
        ChessDesk tempChessDesk = ChessDesk(chessDesk);
        tempChessDesk.move(move.from, move.to);
        moves[i] = {move, tempChessDesk.status(), tempChessDesk};
        i++;
    }
    
    sort(moves.begin(), moves.end(), [color](const MoveForMinmax& a, const MoveForMinmax& b) {
        return color == WHITE ? a.status > b.status : a.status < b.status;
    });
    
    std::vector<std::thread> threads;
    
    for (MoveForMinmax& move : moves) {
        threads.emplace_back([&]() {
            double value = minmax(depth, move.desk, -100000, 100000, color == WHITE ? BLACK : WHITE, false, move.status);
            if ((color == WHITE && value >= bestMove) || (color == BLACK && value <= bestMove)) {
                bestMove = value;
                bestMoveFound = move.move;
            }
            // cout << "mived" << std::endl;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return bestMoveFound;
}



int main(int argc, char* argv[]) {
    
    // for (int i = 0; i < 1; ++i) {
    //     threads.push_back(std::thread(threadFunction, i, std::ref(chessDesk)));
    // }


    // for (auto& thread : threads) {
    //     thread.join();
    // }
    ChessDesk mainChessDesk = ChessDesk(INITIAL_SETUP, WHITE);
    string input;
    while (true) {
        std::getline(std::cin, input); // Считать данные из stdin
        std::istringstream iss(input);
        string command, buffer;
        iss >> command;
        if (command == "exit") {
            break;
        } else if (command == "get-allow-moves") {
            ChessDesk chessDesk = ChessDesk(iss);
            iss >> buffer;
            vector<int> moves = chessDesk.getAllowCages(atoi(buffer.c_str()));
            for (int i = 0; i < moves.size(); ++i) {
                std::cout << moves[i] << " ";
            }
            std::cout << std::endl;
        } else if (command == "print") {
            mainChessDesk.printBoard();
        } else if (command == "get-current") {
            cout << mainChessDesk.toJson();
        } else if (command == "move") {
            string from, to;
            iss >> from >> to;
            mainChessDesk.move(atoi(from.c_str()), atoi(to.c_str()), false, true);
            cout << mainChessDesk.toJson();
        } else if (command == "check") {
            cout << mainChessDesk.isCheck(WHITE);
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
            Move bestMove = minmaxMain(mainChessDesk, mainChessDesk.stepColor, atoi(depth.c_str()));
            auto end_time = std::chrono::steady_clock::now();

            mainChessDesk.move(bestMove.from, bestMove.to);

            auto elapsed_ns = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            // cout << "Time: " << elapsed_ns.count() / 1000. << endl;
            // cout << "Pos cnt:" << positionsCount << endl;
            // cout << "Total time function: " << total_duration.count() / 1000 << endl;
            // cout << "Speed: " << (double)positionsCount / (elapsed_ns.count() / 1000.) << "/s" << endl;
            
            string result = "{\"desk\":[";
            for (int i = 0; i < 64; i++)
            {
                result += to_string((mainChessDesk.desk[i]));
                if (i!= 63)
                    result += ",";
            }
            result += "],\"stepColor\": ";
            result += to_string(mainChessDesk.stepColor);
            result += ", \"time\": ";
            result += to_string(elapsed_ns.count() / 1000.);
            result += ", \"positionsCount\": ";
            result += to_string(positionsCount);
            result += ", \"funcTime\": ";
            result += to_string(total_duration.count() / 1000);
            result += "}";
            cout << result;
            
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

