#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <numeric>

#include "configflagmanager.h"
#include "robin_hood.h"

using namespace std;

JSONValue::JSONObject stats = JSONValue::JSONObject();

static std::chrono::duration<double, std::milli> total_duration(0);
static int positionsCount = 0;


const int EMPTY = 0;
const int PAWN = 1;
const int BISHOP = 3;
const int KNIGHT = 2;
const int ROOK = 4;
const int QUEEN = 5;
const int KING = 6;
const int WHITE = 8;
const int BLACK = 16;

const int MOVED = 32;
const int NOT_MOVED = 64;

const int TYPE_SHIFT = 7;
const int COLOR_SHIFT = 24;
const int MOVED_SHIFT = 96;

const int TRANSPOSITION_TABLE_MAX_SIZE = 30000000;

int MAX_DEPTH = 4;

const robin_hood::unordered_map<int, string> SYMBOLS = {
    {EMPTY, " "},
    {PAWN | WHITE, "♙"},
    {KNIGHT | WHITE, "♘"},
    {BISHOP | WHITE, "♗"},
    {ROOK | WHITE, "♖"},
    {QUEEN | WHITE, "♕"},
    {KING | WHITE, "♔"},
    {PAWN | BLACK, "♟"},
    {KNIGHT | BLACK, "♞"},
    {BISHOP | BLACK, "♝"},
    {ROOK | BLACK, "♜"},
    {QUEEN | BLACK, "♛"},
    {KING | BLACK, "♚"}
};
std::array<int, 64> INITIAL_SETUP = {
    ROOK | BLACK | NOT_MOVED, KNIGHT | BLACK | NOT_MOVED, BISHOP | BLACK | NOT_MOVED, QUEEN | BLACK | NOT_MOVED, KING | BLACK | NOT_MOVED, BISHOP | BLACK | NOT_MOVED, KNIGHT | BLACK | NOT_MOVED, ROOK | BLACK | NOT_MOVED,
    PAWN | BLACK | NOT_MOVED, PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,  PAWN | BLACK | NOT_MOVED, PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    PAWN | WHITE | NOT_MOVED, PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,  PAWN | WHITE | NOT_MOVED, PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,
    ROOK | WHITE | NOT_MOVED, KNIGHT | WHITE | NOT_MOVED, BISHOP | WHITE | NOT_MOVED, QUEEN | WHITE | NOT_MOVED, KING | WHITE | NOT_MOVED, BISHOP | WHITE | NOT_MOVED, KNIGHT | WHITE | NOT_MOVED, ROOK | WHITE | NOT_MOVED
};

// std::array<int, 64> INITIAL_SETUP = {
//     0,0,0,0,108,0,0,0,0,0,0,0,0,81,0,118,0,0,0,0,0,0,0,113,113,0,0,0,0,0,0,0,0,0,113,113,0,0,0,116,105,0,114,0,0,0,0,117,0,0,73,0,0,73,0,0,0,0,0,0,0,0,110,0

// };
// std::array<int, 64> INITIAL_SETUP = {
// 0,0,0,0,0,0,118,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,105,0,0,113,0,0,0,105,0,0,0,0,113,0,0,0,0,113,109,0,0,115,0,0,117,110,0,0,0,0,0,0,0,108,0,0,0,0,0
// };
const robin_hood::unordered_map<int, string> PEICE_WORDS = {
    {EMPTY, "empty"},
    {PAWN | WHITE, "white pawn"},
    {KNIGHT | WHITE, "white knight"},
    {BISHOP | WHITE, "white bishop"},
    {ROOK | WHITE, "white rook"},
    {QUEEN | WHITE, "white queen"},
    {KING | WHITE, "white king"},
    {PAWN | BLACK, "black pawn"},
    {KNIGHT | BLACK, "black knight"},
    {BISHOP | BLACK, "black bishop"},
    {ROOK | BLACK, "black rook"},
    {QUEEN | BLACK, "black queen"},
    {KING | BLACK, "black king"}
};


struct Move {
    int from;
    int to;
    int pieceChoice;
    Move(int f, int t) : from(f), to(t) {}
    Move() : from(0), to(0) {}
};


const robin_hood::unordered_map<int, double> DEFAULT_COST = {
    {KING, 900.},
    {QUEEN, 90.},
    {ROOK, 50.},
    {BISHOP, 30.},
    {KNIGHT, 30.},
    {PAWN, 10.},
    {EMPTY, 0.}
};

vector<vector<double>> reverseArray(const vector<vector<double>> array) {
    vector<vector<double>> reversedArray = array;
    reverse(reversedArray.begin(), reversedArray.end());
    return reversedArray;
}

vector<double> negativeArray(vector<double> array) {
    vector<double> negatedArray = array;
    transform(negatedArray.begin(), negatedArray.end(), negatedArray.begin(), [](double el) {
        return -el;
    });
    return negatedArray;
}

vector<double> unpackArray(const vector<vector<double>> array) {
    vector<double> res;
    for (const auto& arrayElement : array) {
        res.insert(res.end(), arrayElement.begin(), arrayElement.end());
    }
    return res;
}

const vector<vector<double>> whitePawnPositionValues = {
    {0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0},
    {5.0,  5.0,  5.0,  5.0,  5.0,  5.0,  5.0,  5.0},
    {1.0,  1.0,  2.0,  3.0,  3.0,  2.0,  1.0,  1.0},
    {0.5,  0.5,  1.0,  2.5,  2.5,  1.0,  0.5,  0.5},
    {0.0,  0.0,  0.0,  2.0,  2.0,  0.0,  0.0,  0.0},
    {0.5, -0.5, -1.0,  0.0,  0.0, -1.0, -0.5,  0.5},
    {0.5,  1.0,  1.0, -2.0, -2.0,  1.0,  1.0,  0.5},
    {0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0}
};

const vector<double> blackPawnPositionValues = negativeArray(unpackArray(reverseArray(whitePawnPositionValues)));


const vector<vector<double>> whiteKnightPositionValues = {
    {-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0},
    {-4.0, -2.0,  0.0,  0.0,  0.0,  0.0, -2.0, -4.0},
    {-3.0,  0.0,  1.0,  1.5,  1.5,  1.0,  0.0, -3.0},
    {-3.0,  0.5,  1.5,  2.0,  2.0,  1.5,  0.5, -3.0},
    {-3.0,  0.0,  1.5,  2.0,  2.0,  1.5,  0.0, -3.0},
    {-3.0,  0.5,  1.0,  1.5,  1.5,  1.0,  0.5, -3.0},
    {-4.0, -2.0,  0.0,  0.5,  0.5,  0.0, -2.0, -4.0},
    {-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0}
};

const vector<double> blackKnightPositionValues = negativeArray(unpackArray(reverseArray(whiteKnightPositionValues)));

const vector<vector<double>> whiteBishopPositionValues = {
    {-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0},
    {-1.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0},
    {-1.0,  0.0,  0.5,  1.0,  1.0,  0.5,  0.0, -1.0},
    {-1.0,  0.5,  0.5,  1.0,  1.0,  0.5,  0.5, -1.0},
    {-1.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -1.0},
    {-1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, -1.0},
    {-1.0,  0.5,  0.0,  0.0,  0.0,  0.0,  0.5, -1.0},
    {-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0}
};

const vector<double> blackBishopPositionValues = negativeArray(unpackArray(reverseArray(whiteBishopPositionValues)));

const vector<vector<double>> whiteRookPositionValues = {
    { 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0},
    { 0.5,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.5},
    {-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5},
    {-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5},
    {-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5},
    {-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5},
    {-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5},
    { 0.0,  0.0,  0.0,  0.5,  0.5,  0.0,  0.0,  0.0}
};

const vector<double> blackRookPositionValues = negativeArray(unpackArray(reverseArray(whiteRookPositionValues)));

const vector<vector<double>> whiteQueenPositionValues = {
    {-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0},
    {-1.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0},
    {-1.0,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -1.0},
    {-0.5,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -0.5},
    { 0.0,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -0.5},
    {-1.0,  0.5,  0.5,  0.5,  0.5,  0.5,  0.0, -1.0},
    {-1.0,  0.0,  0.5,  0.0,  0.0,  0.0,  0.0, -1.0},
    {-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0}
};

const vector<double> blackQueenPositionValues = negativeArray(unpackArray(reverseArray(whiteQueenPositionValues)));

const vector<vector<double>> whiteKingPositionValues = {
    {-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0},
    {-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0},
    {-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0},
    {-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0},
    {-2.0, -3.0, -3.0, -4.0, -4.0, -3.0, -3.0, -2.0},
    {-1.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0},
    { 2.0,  2.0,  0.0,  0.0,  0.0,  0.0,  2.0,  2.0},
    { 2.0,  3.0,  1.0,  0.0,  0.0,  1.0,  3.0,  2.0}
};

const vector<double> blackKingPositionValues = negativeArray(unpackArray(reverseArray(whiteKingPositionValues)));
const vector<double> emptyPositionValues = {
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0,
    0., 0, 0, 0, 0, 0, 0, 0
};
robin_hood::unordered_map<int, vector<double>> SQUARE_COST = {
    {PAWN | WHITE, unpackArray(whitePawnPositionValues)},
    {KNIGHT | WHITE, unpackArray(whiteKnightPositionValues)},
    {BISHOP | WHITE, unpackArray(whiteBishopPositionValues)},
    {ROOK | WHITE, unpackArray(whiteRookPositionValues)},
    {QUEEN | WHITE, unpackArray(whiteQueenPositionValues)},
    {KING | WHITE, unpackArray(whiteKingPositionValues)},
    {PAWN | BLACK, blackPawnPositionValues},
    {KNIGHT | BLACK, blackKnightPositionValues},
    {BISHOP | BLACK, blackBishopPositionValues},
    {ROOK | BLACK, blackRookPositionValues},
    {QUEEN | BLACK, blackQueenPositionValues},
    {KING | BLACK, blackKingPositionValues},
    {EMPTY, emptyPositionValues}
};

const int ROOK_DIRS[4][2] = {
    {0, 1}, 
    {0, -1}, 
    {-1, 0}, 
    {1, 0}
};
const int BISHOP_DIRS[4][2] = {
    {1, 1}, 
    {1, -1}, 
    {-1, 1}, 
    {-1, -1}
};