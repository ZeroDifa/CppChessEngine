#pragma once;

#include <cstdint>
#include <unordered_map>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

const uint8_t EMPTY = 0;
const uint8_t PAWN = 1;
const uint8_t BISHOP = 3;
const uint8_t KNIGHT = 2;
const uint8_t ROOK = 4;
const uint8_t QUEEN = 5;
const uint8_t KING = 6;
const uint8_t WHITE = 8;
const uint8_t BLACK = 16;

const uint8_t MOVED = 32;
const uint8_t NOT_MOVED = 64;

const uint8_t TYPE_SHIFT = 7;
const uint8_t COLOR_SHIFT = 24;
const uint8_t MOVED_SHIFT = 96;

const int MAX_DEPTH = 6;

const unordered_map<uint8_t, string> SYMBOLS = {
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
// const uint8_t INITIAL_SETUP[] = {
//     ROOK | BLACK | NOT_MOVED, KNIGHT | BLACK | NOT_MOVED, BISHOP | BLACK | NOT_MOVED, QUEEN | BLACK | NOT_MOVED, KING | BLACK | NOT_MOVED, BISHOP | BLACK | NOT_MOVED, KNIGHT | BLACK | NOT_MOVED, ROOK | BLACK | NOT_MOVED,
//     PAWN | BLACK | NOT_MOVED, PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,  PAWN | BLACK | NOT_MOVED, PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,   PAWN | BLACK | NOT_MOVED,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     PAWN | WHITE | NOT_MOVED, PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,  PAWN | WHITE | NOT_MOVED, PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,   PAWN | WHITE | NOT_MOVED,
//     ROOK | WHITE | NOT_MOVED, KNIGHT | WHITE | NOT_MOVED, BISHOP | WHITE | NOT_MOVED, QUEEN | WHITE | NOT_MOVED, KING | WHITE | NOT_MOVED, BISHOP | WHITE | NOT_MOVED, KNIGHT | WHITE | NOT_MOVED, ROOK | WHITE | NOT_MOVED
// };
const uint8_t INITIAL_SETUP[] = {
    84,82,83,85,86,83,0,84,81,81,81,81,0,81,81,81,0,0,0,0,0,0,0,105,0,0,0,0,0,0,0,114,0,0,0,0,0,0,0,105,0,105,0,0,0,0,0,0,0,0,106,73,73,73,73,110,113,0,0,76,74,75,77,0
};
const unordered_map<uint8_t, string> PEICE_WORDS = {
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
    uint8_t from;
    uint8_t to;
    Move(uint8_t f, uint8_t t) : from(f), to(t) {}
    Move() : from(0), to(0) {}
};


const unordered_map<uint8_t, double> DEFAULT_COST = {
    {KING, 900},
    {QUEEN, 90},
    {ROOK, 50},
    {BISHOP, 30},
    {KNIGHT, 30},
    {PAWN, 10},
    {EMPTY, 0}
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
unordered_map<uint8_t, vector<double>> SQUARE_COST = {
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
