#include <iostream>
#include <string>
#include <cstdint>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include "constants.h"
#include "Reader.h"
#include "Writer.h"

using namespace std;

// chess desk class
class ChessDesk {
private:
    struct Cage {
        int position;
        int piece;
        int type;
        int color;
        int isFirstMove;
    };
    struct RowColumn
    {
        int row;
        int column;
    };
    struct Piece {
        int piece;
        int position;
    };
    struct Turn {
        Piece from;
        Piece to;
    };
    struct State {
        int stepColor;
        vector<Turn> turns;
    };
    
public:
    vector<State> history;
    int stepColor;
    int desk[64];
    unordered_map<int, vector<int>> piecePositions;
    ChessDesk() {
    };
    ChessDesk(int* desk, int stepColor) {
        memcpy(this->desk, desk, 64 * sizeof(int));
        this->stepColor = stepColor;
        this->updatePiecePositions();
    }
    ChessDesk(const ChessDesk& chessDesk) {
        memcpy(this->desk, chessDesk.desk, 64 * sizeof(int));
        this->history = chessDesk.history;
        stepColor = chessDesk.stepColor;
        this->updatePiecePositions();
    }
    //deconstructor
    // ~ChessDesk() {
    //     delete[] desk;
    // }
    ChessDesk(istringstream& data) {
        string str;
        int position = 0;
        for (int i = 0; i < 64; i++) {
            data >> str;
            const int piece = static_cast<int>(std::stoi(str));
            this->desk[i] = piece;
        }
        data >> str;
        this->stepColor = static_cast<int>(std::stoi(str));
        this->updatePiecePositions();
    }
    double status() const {
        double status = 0;
        for (int i = 0; i < 64; i++) {
            int piece = this->desk[i];
            int pieceType = piece & TYPE_SHIFT;
            int color = piece & COLOR_SHIFT;
            if (piece == EMPTY) {
                continue;
            }
            status += DEFAULT_COST.at(piece & TYPE_SHIFT)*(color == WHITE ? 1. : -1.) + SQUARE_COST.at(pieceType | color)[i];
            // cout << SYMBOLS.at(pieceType | color) << " " << DEFAULT_COST.at(piece & TYPE_SHIFT)*(color == WHITE ? 1 : -1) + SQUARE_COST.at(pieceType | color)[i] << endl;
        }
        
        return status;
    }
    void printBoard() {
        // Reader reader(desk);
        for (int i = 0; i < 64; i++) {
            int piece = this->desk[i];
            cout << SYMBOLS.at((piece & TYPE_SHIFT) | (piece & COLOR_SHIFT)) << " ";
            if ((i + 1) % 8 == 0) {
                cout << endl;
            }
        }
        cout << endl;
    }
    void saveState(Turn turn) {
        State state;
        state.turns.push_back(turn);
        state.stepColor = this->stepColor;
        history.push_back(state);
    }
    void appendMoveToLastState(Turn turn) {
        State& state = history.back();
        state.turns.push_back(turn);
    }
    void undo() {
        if (!history.empty()) {

            State& state = history.back();
            for (Turn& turn : state.turns) {
                this->desk[turn.from.position] = turn.from.piece;
                this->desk[turn.to.position] = turn.to.piece;
                int toPiece = turn.to.piece;
                int fromPiece = turn.from.piece;
                this->piecePositions[(toPiece & TYPE_SHIFT) | (toPiece & COLOR_SHIFT)].push_back(turn.to.position);

                auto& piecePositions = this->piecePositions[(fromPiece & TYPE_SHIFT) | (fromPiece & COLOR_SHIFT)];

                auto index = std::find(piecePositions.begin(), piecePositions.end(), turn.to.position);
                piecePositions.erase(index);
                piecePositions.push_back(turn.from.position);
            }
            this->stepColor = state.stepColor;
            history.pop_back();


        }
    }
    bool move(int from, int to, bool hightPriority = false, bool newSave = true) {
        bool isWasKilled = false;
        
        Cage fromCage = getCage(from);
        Cage toCage = getCage(to);
        isWasKilled = toCage.color != stepColor && toCage.piece != EMPTY;

        if (newSave) {
            this->saveState({{fromCage.piece, fromCage.position}, {toCage.piece, toCage.position}});
        } else {
            this->appendMoveToLastState({{fromCage.piece, fromCage.position}, {toCage.piece, toCage.position}});
        }

        if (!hightPriority && (fromCage.color != stepColor)) {
            return isWasKilled;
        }


        this->desk[toCage.position] = fromCage.color | fromCage.piece | MOVED;
        this->desk[fromCage.position] = EMPTY;

        auto& fromPiecePositions = this->piecePositions[fromCage.type | fromCage.color];
        fromPiecePositions.erase(std::find(fromPiecePositions.begin(), fromPiecePositions.end(), fromCage.position));
        fromPiecePositions.push_back(toCage.position);

        auto& toPiecePositions = this->piecePositions[toCage.type | toCage.color];
        toPiecePositions.erase(std::find(toPiecePositions.begin(), toPiecePositions.end(), toCage.position));


        if (fromCage.type == KING && abs(fromCage.position - toCage.position) == 2) {
            if (toCage.position-fromCage.position < 0)
                this->move(fromCage.position - 4, fromCage.position -1, true, false);
            else
                this->move(fromCage.position + 3, fromCage.position + 1, true, false);
        }

        if (!hightPriority) {
            this->stepColor = (this->stepColor == WHITE) ? BLACK : WHITE;
        }
        return isWasKilled;
    }
    Cage getCage(const int& position) {
        int piece = this->desk[position];
        return {
            position,
            piece,
            piece & TYPE_SHIFT,
            piece & COLOR_SHIFT,
            piece & MOVED_SHIFT
        };
    }
    RowColumn getRowColumn(int& position) {
        return {
            static_cast<int>(position / 8),
            static_cast<int>(position % 8)
        };
    }
    int getIndexByRowColumn(const int& row, const int& column) {
        if (column > 7 || row > 7 || row < 0 || column < 0) return -1;
        return row * 8 + column;
    }

    vector<int> getAllowCages(int pos, bool isCheck = true) {
        Cage pieceObj = getCage(pos);
        int type = pieceObj.type;
        int color = pieceObj.color;
        int isFirstMove = pieceObj.isFirstMove;

        RowColumn rowCol = getRowColumn(pos);
        int row = rowCol.row, col = rowCol.column;

        vector<int> result;

        int index, current_peice, current_color;
        int sign = color == BLACK ? 1 : -1;

        switch (pieceObj.type) {
            case PAWN:
                index = this->getIndexByRowColumn(row + sign, col);
                if ((this->desk[index] & TYPE_SHIFT) == EMPTY) {
                    result.push_back(index);
                    index = this->getIndexByRowColumn(row + 2*sign, col);
                    if (isFirstMove == NOT_MOVED && (this->desk[index] & TYPE_SHIFT) == EMPTY) {
                        result.push_back(index);
                    }
                }
                for (int i : {-1, 1}) {
                    index = this->getIndexByRowColumn(row + sign, col + i);
                    current_color = this->desk[index];
                    if ((current_color & TYPE_SHIFT) != EMPTY && (current_color & COLOR_SHIFT) != color) {
                        result.push_back(index);
                    }
                }
                break;
            case KNIGHT:
                this->getKnightAllowCages(row, col, result);
                break;
            case BISHOP:
                this->getBishopAllowCages(row, col, color, result);
                break;
            case ROOK:
                this->getRookAllowCages(row, col, color, result);
                break;
            case QUEEN:
                this->getBishopAllowCages(row, col, color, result);
                this->getRookAllowCages(row, col, color, result);
                break;
            case KING:
                result.push_back(getIndexByRowColumn(row+1, col-1)); 
                result.push_back(getIndexByRowColumn(row+1, col));
                result.push_back(getIndexByRowColumn(row+1, col+1));
                result.push_back(getIndexByRowColumn(row-1, col-1));
                result.push_back(getIndexByRowColumn(row-1, col));
                result.push_back(getIndexByRowColumn(row-1, col+1));
                result.push_back(getIndexByRowColumn(row, col-1));
                result.push_back(getIndexByRowColumn(row, col+1));

                current_peice = this->desk[this->getIndexByRowColumn(row, col+3)];
                if (
                    isFirstMove == NOT_MOVED && current_peice == ROOK|color|NOT_MOVED &&
                    this->desk[this->getIndexByRowColumn(row, col+1)] == EMPTY &&
                    this->desk[this->getIndexByRowColumn(row, col+2)] == EMPTY
                ) {
                    result.push_back(getIndexByRowColumn(row, col+2));
                }
                current_peice = this->desk[this->getIndexByRowColumn(row, col-4)];
                if (
                    isFirstMove == NOT_MOVED && current_peice == ROOK|color|NOT_MOVED &&
                    this->desk[this->getIndexByRowColumn(row, col-3)] == EMPTY &&
                    this->desk[this->getIndexByRowColumn(row, col-1)] == EMPTY
                ) {
                    result.push_back(getIndexByRowColumn(row, col-2));
                }
                break;
        }

        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());

        for (int i = 0; i < result.size(); i++) {
            int piece = this->desk[result[i]];
            if ((result[i] == 255) ||
                (result[i] == -1) ||
                ((piece & TYPE_SHIFT) != EMPTY && (piece & COLOR_SHIFT) == color)
            ) {
                result.erase(result.begin() + i);
                i--;
            }
        }

        if (isCheck) {
            this->checkInFuture(color, pos, result);
        }
        // if (result.size() == 0) {
            // result.push_back(255);
        // }
        return result;
    }
    vector<Move> getAllMoves(int color) {
        vector<Move> result;
        for (int i = 0; i < 64; i++) {
            int piece = this->desk[i];
            if ((piece & TYPE_SHIFT) == EMPTY || (piece & COLOR_SHIFT) != color)
            {
                continue;
            }
            vector<int> moves = this->getAllowCages(i);
            if (!moves.empty() && moves.back() == 255) {
                continue;
            }
            for (auto move : moves) {
                result.push_back({i, move});
            }
        }
        return result;
    }
    void updatePiecePositions() {
        this->piecePositions.clear();
        size_t i = 0;

        while (i < 64)
        {
            int piece = (this->desk[i] & TYPE_SHIFT) | (this->desk[i] & COLOR_SHIFT);
            if (this->piecePositions.find(piece) == this->piecePositions.end())
            {
                this->piecePositions[piece].reserve(8);
            }
            this->piecePositions[piece].push_back(i);
            i++;
        }
    }
    void checkInFuture(int color, int pos, vector<int> &result) {
        for (int i = 0; i < result.size(); i++) {
            int turn = result[i];
            this->move(pos, turn, false);
            bool isCheck = this->isCheck(color);
            this->undo();
            if (isCheck) {
                result.erase(result.begin() + i);
                i--;
            }
        }
    }
    bool isCheck(int color) {
        int kingPos = this->piecePositions[KING | color][0];
        RowColumn rowCol = getRowColumn(kingPos);
        int row = rowCol.row, col = rowCol.column;
        int otherColor = color == WHITE ? BLACK : WHITE;
        vector<int> att;
        this->getKnightAllowCages(row, col, att);
        if (
            (this->piecePositions[KNIGHT | otherColor].size() > 0) && 
            any_of(
                this->piecePositions[KNIGHT | otherColor].begin(), 
                this->piecePositions[KNIGHT | otherColor].end(), 
                [&att](int el) 
                    { 
                        return std::find(att.begin(), att.end(), el) != att.end(); 
                    }
                )
            ) 
        {
            return true;
        }
        int sign = color == WHITE ? -1 : 1;
        att = {
            this->getIndexByRowColumn(row + sign, col - 1),
            this->getIndexByRowColumn(row + sign, col + 1),
        };
        vector<int> pieces = this->piecePositions[PAWN | otherColor];
        if (any_of(pieces.begin(), pieces.end(), [&att](int el) { return std::find(att.begin(), att.end(), el)!= att.end(); }))
        {
            return true;
        }
        att.clear();
        
        // Check for rook and queen threats
        vector<int> rookMoves;
        this->getRookAllowCages(row, col, color, rookMoves);
        vector<int> bishopMoves;
        this->getBishopAllowCages(row, col, color, bishopMoves);

        vector<int> queenPos = 
            this->piecePositions[QUEEN | otherColor].empty() ? 
                vector<int>() : this->piecePositions[QUEEN | otherColor];
        
        auto rookPieces = this->piecePositions[ROOK | otherColor];
        if (rookPieces.empty()) {
            rookPieces = vector<int>();
        }
        rookPieces.insert(rookPieces.end(), queenPos.begin(), queenPos.end());
        if (any_of(rookPieces.begin(), rookPieces.end(), [&](int el) {
            return find(rookMoves.begin(), rookMoves.end(), el) != rookMoves.end(); 
        })) {
            return true;
        }

        auto bishopPieces = piecePositions[BISHOP | otherColor];
        if (bishopPieces.empty()) {
            bishopPieces = vector<int>();
        }
        bishopPieces.insert(bishopPieces.end(), queenPos.begin(), queenPos.end());

        if (any_of(bishopPieces.begin(), bishopPieces.end(), [&](int el) {
            return find(bishopMoves.begin(), bishopMoves.end(), el) != bishopMoves.end(); 
        })) {
            return true;
        }
        att.clear();
        att = {
            this->getIndexByRowColumn(row + 1, col),
            this->getIndexByRowColumn(row + 1, col + 1),
            this->getIndexByRowColumn(row + 1, col - 1),
            this->getIndexByRowColumn(row - 1, col),
            this->getIndexByRowColumn(row - 1, col + 1),
            this->getIndexByRowColumn(row - 1, col - 1),
            this->getIndexByRowColumn(row, col + 1),
            this->getIndexByRowColumn(row, col - 1)
        };
        if (any_of(this->piecePositions[KING | otherColor].begin(), this->piecePositions[KING | otherColor].end(), [&](int el) {
            return find(att.begin(), att.end(), el)!= att.end(); 
        })) {
            return true;
        }


        return false;
    }
    void getKnightAllowCages(int row, int col, vector<int>& result)
    {
        for (int i : {-1, 1})
        {
            result.push_back(this->getIndexByRowColumn(row + i, col + 2));
            result.push_back(this->getIndexByRowColumn(row + i, col - 2));
            result.push_back(this->getIndexByRowColumn(row + 2*i, col + i));
            result.push_back(this->getIndexByRowColumn(row + 2*i, col - i));
        }
    }
    void getBishopAllowCages(int row, int col, int color, vector<int>& result)
    {
        for (int i : {-1, 1})
        {
            for (int j : {-1, 1})
            {
                int currentRow = row + i;
                int currentCol = col + j;
                while (currentRow >= 0 && currentRow < 8 && currentCol >= 0 && currentCol < 8)
                {
                    int index = getIndexByRowColumn(currentRow, currentCol);
                    if (index == -1)
                        break;

                    if ((desk[index] & TYPE_SHIFT) == EMPTY)
                    {
                        result.push_back(index);
                    }
                    else
                    {
                        if ((desk[index] & COLOR_SHIFT) != color)
                        {
                            result.push_back(index);
                        }
                        break;
                    }
                    currentRow += i;
                    currentCol += j;
                }
            }
        }
    }
    void getRookAllowCages(int row, int col, int color, vector<int>& result)
    {
        for (int i : {-1, 1})
        {
            int currentRow = row + i;
            int currentCol = col;
            while (currentRow >= 0 && currentRow < 8)
            {
                int index = getIndexByRowColumn(currentRow, currentCol);
                if (index == -1)
                    break;

                if ((desk[index] & TYPE_SHIFT) == EMPTY)
                {
                    result.push_back(index);
                }
                else
                {
                    if ((desk[index] & COLOR_SHIFT) != color)
                    {
                        result.push_back(index);
                    }
                    break;
                }
                currentRow += i;
            }
        }

        for (int j : {-1, 1})
        {
            int currentRow = row;
            int currentCol = col + j;
            while (currentCol >= 0 && currentCol < 8)
            {
                int index = getIndexByRowColumn(currentRow, currentCol);
                if (index == -1)
                    break;

                if ((desk[index] & TYPE_SHIFT) == EMPTY)
                {
                    result.push_back(index);
                }
                else
                {
                    if ((desk[index] & COLOR_SHIFT) != color)
                    {
                        result.push_back(index);
                    }
                    break;
                }
                currentCol += j;
            }
        }
    }
    string toJson() const {
        string result = "{\"desk\":[";
        for (int i = 0; i < 64; i++)
        {
            result += to_string((desk[i]));
            if (i!= 63)
                result += ",";
        }
        result += "],\"stepColor\": ";
        result += to_string(this->stepColor);
        result += "}";
        return result;
    }
};

struct MoveForMinmax {
    Move move;
    double status;
    ChessDesk desk;
    MoveForMinmax(Move m, double s, ChessDesk d) : move(m), status(s), desk(d) {}
    MoveForMinmax() : move(Move()), status(0), desk(ChessDesk()) {}
    MoveForMinmax(Move m, double s) : move(m), status(s), desk(ChessDesk()) {}
};