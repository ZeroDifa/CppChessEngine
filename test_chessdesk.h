#include <cassert>
#include "chessdesk.h"


void testStatus() {
    ChessDesk chessDesk;

    // Test case 1: Empty chessboard
    double result1 = chessDesk.status();
    assert(result1 == 0.0);

    // Test case 2: Chessboard with pieces
    chessDesk.desk[0] = WHITE | PAWN;
    chessDesk.desk[8] = BLACK | ROOK;
    chessDesk.desk[16] = WHITE | KNIGHT;
    chessDesk.desk[24] = BLACK | BISHOP;
    chessDesk.desk[32] = WHITE | QUEEN;
    chessDesk.desk[40] = BLACK | KING;
    chessDesk.desk[48] = WHITE | PAWN;
    chessDesk.desk[56] = BLACK | ROOK;

    double result2 = chessDesk.status();
    assert(result2 == 0.0); // The sum of the piece values is 0

    // Test case 3: Chessboard with different piece values
    chessDesk.desk[0] = WHITE | PAWN;
    chessDesk.desk[8] = BLACK | ROOK;
    chessDesk.desk[16] = WHITE | KNIGHT;
    chessDesk.desk[24] = BLACK | BISHOP;
    chessDesk.desk[32] = WHITE | QUEEN;
    chessDesk.desk[40] = BLACK | KING;
    chessDesk.desk[48] = WHITE | PAWN;
    chessDesk.desk[56] = BLACK | ROOK;

    double result3 = chessDesk.status();
    assert(result3 == 0.0); // The sum of the piece values is 0

    // Test case 4: Chessboard with positive and negative piece values
    chessDesk.desk[0] = WHITE | PAWN;
    chessDesk.desk[8] = BLACK | ROOK;
    chessDesk.desk[16] = WHITE | KNIGHT;
    chessDesk.desk[24] = BLACK | BISHOP;
    chessDesk.desk[32] = WHITE | QUEEN;
    chessDesk.desk[40] = BLACK | KING;
    chessDesk.desk[48] = WHITE | PAWN;
    chessDesk.desk[56] = BLACK | ROOK;
    chessDesk.desk[1] = WHITE | PAWN;
    chessDesk.desk[9] = BLACK | ROOK;
    chessDesk.desk[17] = WHITE | KNIGHT;
    chessDesk.desk[25] = BLACK | BISHOP;
    chessDesk.desk[33] = WHITE | QUEEN;
    chessDesk.desk[41] = BLACK | KING;
    chessDesk.desk[49] = WHITE | PAWN;
    chessDesk.desk[57] = BLACK | ROOK;

    double result4 = chessDesk.status();
    assert(result4 == 0.0); // The sum of the piece values is 0

    // Test case 5: Chessboard with non-zero piece values
    chessDesk.desk[0] = WHITE | PAWN;
    chessDesk.desk[8] = BLACK | ROOK;
    chessDesk.desk[16] = WHITE | KNIGHT;
    chessDesk.desk[24] = BLACK | BISHOP;
    chessDesk.desk[32] = WHITE | QUEEN;
    chessDesk.desk[40] = BLACK | KING;
    chessDesk.desk[48] = WHITE | PAWN;
    chessDesk.desk[56] = BLACK | ROOK;
    chessDesk.desk[1] = WHITE | PAWN;
    chessDesk.desk[9] = BLACK | ROOK;
    chessDesk.desk[17] = WHITE | KNIGHT;
    chessDesk.desk[25] = BLACK | BISHOP;
    chessDesk.desk[33] = WHITE | QUEEN;
    chessDesk.desk[41] = BLACK | KING;
    chessDesk.desk[49] = WHITE | PAWN;
    chessDesk.desk[57] = BLACK | ROOK;
    chessDesk.desk[2] = WHITE | PAWN;
    chessDesk.desk[10] = BLACK | ROOK;
    chessDesk.desk[18] = WHITE | KNIGHT;
    chessDesk.desk[26] = BLACK | BISHOP;
    chessDesk.desk[34] = WHITE | QUEEN;
    chessDesk.desk[42] = BLACK | KING;
    chessDesk.desk[50] = WHITE | PAWN;
    chessDesk.desk[58] = BLACK | ROOK;

    double result5 = chessDesk.status();
    assert(result5 == 0.0); // The sum of the piece values is 0

    // Test case 6: Chessboard with positive and negative piece values
    chessDesk.desk[0] = WHITE | PAWN;
    chessDesk.desk[8] = BLACK | ROOK;
    chessDesk.desk[16] = WHITE | KNIGHT;
    chessDesk.desk[24] = BLACK | BISHOP;
    chessDesk.desk[32] = WHITE | QUEEN;
    chessDesk.desk[40] = BLACK | KING;
    chessDesk.desk[48] = WHITE | PAWN;
    chessDesk.desk[56] = BLACK | ROOK;
    chessDesk.desk[1] = WHITE | PAWN;
    chessDesk.desk[9] = BLACK | ROOK;
    chessDesk.desk[17] = WHITE | KNIGHT;
    chessDesk.desk[25] = BLACK | BISHOP;
    chessDesk.desk[33] = WHITE | QUEEN;
    chessDesk.desk[41] = BLACK | KING;
    chessDesk.desk[49] = WHITE | PAWN;
    chessDesk.desk[57] = BLACK | ROOK;
    chessDesk.desk[2] = WHITE | PAWN;
    chessDesk.desk[10] = BLACK | ROOK;
    chessDesk.desk[18] = WHITE | KNIGHT;
    chessDesk.desk[26] = BLACK | BISHOP;
    chessDesk.desk[34] = WHITE | QUEEN;
    chessDesk.desk[42] = BLACK | KING;
    chessDesk.desk[50] = WHITE | PAWN;
    chessDesk.desk[58] = BLACK | ROOK;
    chessDesk.desk[3] = WHITE | PAWN;
    chessDesk.desk[11] = BLACK | ROOK;
    chessDesk.desk[19] = WHITE | KNIGHT;
    chessDesk.desk[27] = BLACK | BISHOP;
    chessDesk.desk[35] = WHITE | QUEEN;
    chessDesk.desk[43] = BLACK | KING;
    chessDesk.desk[51] = WHITE | PAWN;
    chessDesk.desk[59] = BLACK | ROOK;

    double result6 = chessDesk.status();
    assert(result6 == 0.0); // The sum of the piece values is 0
}

int main() {
    testStatus();

    return 0;
}