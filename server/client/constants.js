
const EMPTY = 0;
const PAWN = 1;
const BISHOP = 3;
const KNIGHT = 2;
const ROOK = 4;
const QUEEN = 5;
const KING = 6;
const WHITE = 8;
const BLACK = 16;

const MOVED = 32;
const NOT_MOVED = 64;


const TYPE_SHIFT = 7;
const COLOR_SHIFT = 24;
const MOVED_SHIFT = 96;

const PIECE_BYTE_SIZE = 3;


const SYMBOLS = {
    [EMPTY]: '  ',
    [PAWN | WHITE]: '♙',
    [KNIGHT | WHITE]: '♘',
    [BISHOP | WHITE]: '♗',
    [ROOK | WHITE]: '♖',
    [QUEEN | WHITE]: '♕',
    [KING | WHITE]: '♔',
    [PAWN | BLACK]: '♟',
    [KNIGHT | BLACK]: '♞',
    [BISHOP | BLACK]: '♝',
    [ROOK | BLACK]: '♜',
    [QUEEN | BLACK]: '♛',
    [KING | BLACK]: '♚'
}
const INITIAL_SETUP = [
    ROOK | BLACK, KNIGHT | BLACK, BISHOP | BLACK, QUEEN | BLACK, KING | BLACK, BISHOP | BLACK, KNIGHT | BLACK, ROOK | BLACK,
    PAWN | BLACK, PAWN | BLACK,   PAWN | BLACK,   PAWN | BLACK,  PAWN | BLACK, PAWN | BLACK,   PAWN | BLACK,   PAWN | BLACK,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    PAWN | WHITE, PAWN | WHITE,   PAWN | WHITE,   PAWN | WHITE,  PAWN | WHITE, PAWN | WHITE,   PAWN | WHITE,   PAWN | WHITE,
    ROOK | WHITE, KNIGHT | WHITE, BISHOP | WHITE, QUEEN | WHITE, KING | WHITE, BISHOP | WHITE, KNIGHT | WHITE, ROOK | WHITE
]
// const INITIAL_SETUP = [
//     ROOK | BLACK, KNIGHT | BLACK, BISHOP | BLACK, QUEEN | BLACK, KING | BLACK, BISHOP | BLACK, KNIGHT | BLACK, ROOK | BLACK,
//     PAWN | BLACK, PAWN | BLACK,   PAWN | BLACK,   PAWN | BLACK,  PAWN | BLACK, PAWN | BLACK,   PAWN | BLACK,   PAWN | BLACK,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
//     PAWN | WHITE, PAWN | WHITE,   PAWN | WHITE,   PAWN | WHITE,  PAWN | WHITE, PAWN | WHITE,   PAWN | WHITE,   PAWN | WHITE,
//     ROOK | WHITE, EMPTY,          EMPTY,          EMPTY,         KING | WHITE, BISHOP | WHITE, KNIGHT | WHITE, ROOK | WHITE
// ]

let Images = {
    [BISHOP + '-' + BLACK]: 'img/eb.png',  // 1 = ПЕШКА 2 = КОНЬ. 3 = СЛОН. 4 = ЛАДЬЯ. 5 = ФЕРЗЬ. 6 = КОРОЛЬ
    [BISHOP + '-' + WHITE]: 'img/ew.png',
    [PAWN + '-' + BLACK]: 'img/pb.png',
    [PAWN + '-' + WHITE]: 'img/pw.png',
    [KNIGHT + '-' + BLACK]: 'img/hb.png',
    [KNIGHT + '-' + WHITE]: 'img/hw.png',
    [KING + '-' + BLACK]: 'img/kb.png',
    [KING + '-' + WHITE]: 'img/kw.png',
    [QUEEN + '-' + BLACK]: 'img/qb.png',
    [QUEEN + '-' + WHITE]: 'img/qw.png',
    [ROOK + '-' + BLACK]: 'img/rb.png',
    [ROOK + '-' + WHITE]: 'img/rw.png',
    'stop': 'img/stop.png',
    'checkblack': 'img/checkblack.png',
    'checkwhite': 'img/checkwhite.png'
}
let total_load = Object.keys(Images).length;
let load = () => {
    total_load--
    if (!total_load) {
        // render(MainDesk)
    }
}
let src;
for (let id in Images) {
    src = Images[id];
    Images[id] = new Image();
    Images[id].src = src;
    Images[id].onload = load
}


class Writer {
    constructor(buffer) {
        this.buffer = buffer;
        this.view = new DataView(buffer);
        this.position = 0;
    }
    writeUint8(value) {
        this.view.setUint8(this.position, value);
        this.position += Uint8Array.BYTES_PER_ELEMENT;
        return this;
    }
    seek(position) {
        this.position = position;
        return this;
    }
}
class Reader {
    constructor(buffer) {
        this.buffer = buffer;
        this.view = new DataView(buffer);
        this.position = 0;
    }
    readUint8(index = this.position) {
        if (index < 0) return 0;
        const value = this.view.getUint8(index);
        if (index === this.position) this.position += Uint8Array.BYTES_PER_ELEMENT;
        return value;
    }
    seek(position) {
        this.position = position;
        return this;
    }
}





function reverseArray(array) {
    return array.slice().reverse();
}
function negativeArray(array) {
    return array.slice().map((el) => {
        return -el;
    })
}
function unpackArray(array) {
    let res = [];
    for (const arrayElement of array.slice()) {
        res = res.concat(arrayElement);
    }
    return res;
}
const whitePawnPositionValues =
[
    [0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0],
    [5.0,  5.0,  5.0,  5.0,  5.0,  5.0,  5.0,  5.0],
    [1.0,  1.0,  2.0,  3.0,  3.0,  2.0,  1.0,  1.0],
    [0.5,  0.5,  1.0,  2.5,  2.5,  1.0,  0.5,  0.5],
    [0.0,  0.0,  0.0,  2.0,  2.0,  0.0,  0.0,  0.0],
    [0.5, -0.5, -1.0,  0.0,  0.0, -1.0, -0.5,  0.5],
    [0.5,  1.0, 1.0,  -2.0, -2.0,  1.0,  1.0,  0.5],
    [0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0]
];

const blackPawnPositionValues = negativeArray(unpackArray(reverseArray(whitePawnPositionValues)));

const whiteKnightPositionValues=
[
    [-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0],
    [-4.0, -2.0,  0.0,  0.0,  0.0,  0.0, -2.0, -4.0],
    [-3.0,  0.0,  1.0,  1.5,  1.5,  1.0,  0.0, -3.0],
    [-3.0,  0.5,  1.5,  2.0,  2.0,  1.5,  0.5, -3.0],
    [-3.0,  0.0,  1.5,  2.0,  2.0,  1.5,  0.0, -3.0],
    [-3.0,  0.5,  1.0,  1.5,  1.5,  1.0,  0.5, -3.0],
    [-4.0, -2.0,  0.0,  0.5,  0.5,  0.0, -2.0, -4.0],
    [-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0]
];
const blackKnightPositionValues = negativeArray(unpackArray(reverseArray(whiteKnightPositionValues)));

const whiteBishopPositionValues = [
    [-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0],
    [-1.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0],
    [-1.0,  0.0,  0.5,  1.0,  1.0,  0.5,  0.0, -1.0],
    [-1.0,  0.5,  0.5,  1.0,  1.0,  0.5,  0.5, -1.0],
    [-1.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -1.0],
    [-1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, -1.0],
    [-1.0,  0.5,  0.0,  0.0,  0.0,  0.0,  0.5, -1.0],
    [-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0]
];

const blackBishopPositionValues = negativeArray(unpackArray(reverseArray(whiteBishopPositionValues)));

const whiteRookPositionValues = [
    [ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0],
    [ 0.5,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.5],
    [-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5],
    [-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5],
    [-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5],
    [-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5],
    [-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5],
    [ 0.0,   0.0, 0.0,  0.5,  0.5,  0.0,  0.0,  0.0]
];

const blackRookPositionValues = negativeArray(unpackArray(reverseArray(whiteRookPositionValues)));

const whiteQueenPositionValues =
[
    [-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0],
    [-1.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0],
    [-1.0,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -1.0],
    [-0.5,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -0.5],
    [ 0.0,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -0.5],
    [-1.0,  0.5,  0.5,  0.5,  0.5,  0.5,  0.0, -1.0],
    [-1.0,  0.0,  0.5,  0.0,  0.0,  0.0,  0.0, -1.0],
    [-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0]
];
const blackQueenPositionValues = negativeArray(unpackArray(reverseArray(whiteQueenPositionValues)));

const whiteKingPositionValues = [
    [-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0],
    [-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0],
    [-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0],
    [-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0],
    [-2.0, -3.0, -3.0, -4.0, -4.0, -3.0, -3.0, -2.0],
    [-1.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0],
    [ 2.0,  2.0,  0.0,  0.0,  0.0,  0.0,  2.0,  2.0],
    [ 2.0,  3.0,  1.0,  0.0,  0.0,  1.0,  3.0,  2.0]
];
const blackKingPositionValues = negativeArray(unpackArray(reverseArray(whiteKingPositionValues)));

let SQUARE_COST = {
    [PAWN | WHITE]: unpackArray(whitePawnPositionValues),
    [KNIGHT | WHITE]: unpackArray(whiteKnightPositionValues),
    [BISHOP | WHITE]: unpackArray(whiteBishopPositionValues),
    [ROOK | WHITE]: unpackArray(whiteRookPositionValues),
    [QUEEN | WHITE]: unpackArray(whiteQueenPositionValues),
    [KING | WHITE]: unpackArray(whiteKingPositionValues),
    [PAWN | BLACK]: blackPawnPositionValues,
    [KNIGHT | BLACK]: blackKnightPositionValues,
    [BISHOP | BLACK]: blackBishopPositionValues,
    [ROOK | BLACK]: blackRookPositionValues,
    [QUEEN | BLACK]: blackQueenPositionValues,
    [KING | BLACK]: blackKingPositionValues
};

const DEFAULT_COST = {
    [KING]: 900,
    [QUEEN]: 90,
    [ROOK]: 50,
    [KNIGHT]: 30,
    [BISHOP]: 30,
    [PAWN]: 10,
}

let PIECE_WORDS = {
    0: 'EMPTY',
    [PAWN | WHITE]: 'WHITE PAWN',
    [KNIGHT | WHITE]: 'WHITE KNIGHT',
    [BISHOP | WHITE]: 'WHITE BISHOP',
    [ROOK | WHITE]: 'WHITE ROOK',
    [QUEEN | WHITE]: 'WHITE QUEEN',
    [KING | WHITE]: 'WHITE KING',
    [PAWN | BLACK]: 'BLACK PAWN',
    [KNIGHT | BLACK]: 'BLACK KNIGHT',
    [BISHOP | BLACK]: 'BLACK BISHOP',
    [ROOK | BLACK]: 'BLACK ROOK',
    [QUEEN | BLACK]: 'BLACK QUEEN',
    [KING | BLACK]: 'BLACK KING'
};

function printSquareArray(arr) {
    const size = Math.sqrt(arr.length); // Вычисляем размер квадратного массива

    // Преобразуем одномерный массив в двумерный
    const squareArray = [];
    for (let i = 0; i < size; i++) {
        squareArray.push(arr.slice(i * size, (i + 1) * size));
    }

    // Выводим двумерный массив
    for (let i = 0; i < squareArray.length; i++) {
        let row = '';
        for (let j = 0; j < squareArray[i].length; j++) {
            row += '  ' + (squareArray[i][j] >= 0 ? ' ' : '') + squareArray[i][j].toFixed(1);
        }
        console.log(row);
    }
}


const CHECKERS_INITIAL_SETUP = [
    EMPTY, PAWN | BLACK, 0, PAWN | BLACK, 0, PAWN | BLACK, 0, PAWN | BLACK,
    PAWN | BLACK, EMPTY,   PAWN | BLACK,   EMPTY,  PAWN | BLACK, EMPTY,   PAWN | BLACK,   EMPTY,
    EMPTY, PAWN | BLACK, 0, PAWN | BLACK, 0, PAWN | BLACK, 0, PAWN | BLACK,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    EMPTY,        EMPTY,          EMPTY,          EMPTY,         EMPTY,        EMPTY,          EMPTY,          EMPTY,
    PAWN | WHITE, EMPTY,   PAWN | WHITE,   EMPTY,  PAWN | WHITE, EMPTY,   PAWN | WHITE,   EMPTY,
    EMPTY, PAWN | WHITE, 0, PAWN | WHITE, 0, PAWN | WHITE, 0, PAWN | WHITE,
    PAWN | WHITE, EMPTY,   PAWN | WHITE,   EMPTY,  PAWN | WHITE, EMPTY,   PAWN | WHITE,   EMPTY,
]