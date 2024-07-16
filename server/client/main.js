let a = Math.min(window.innerWidth, window.innerHeight) * 0.8;
console.log(window.innerWidth, window.innerHeight, a);
let px = a / 8;
//CANVAS
let canvas = document.getElementById('canvas');
window.ctx = canvas.getContext('2d');
canvas.width = a;
canvas.height = a;
window.onresize = (e) => {
    a = Math.min(window.innerWidth, window.innerHeight) * 0.8
    px = a / 8;
    canvas.width = a;
    canvas.height = a;
}

// fetch to localhost

let l = console.log;


let PlayerColor = [WHITE, BLACK][rnd([0, 2])];
let BotColor = (PlayerColor === WHITE) ? BLACK : WHITE
l(PlayerColor)

// let MainDesk = new CheckersDesk();
let MainDesk = new Desk();
MainDesk.fetchCurrent();

function render(ds) {
    ctx.clearRect(0, 0, canvas.width, canvas.height)
    let ActiveX = PeiceOnFocus.position%8 * px, ActiveY = Math.floor(PeiceOnFocus.position / 8) * px;
    if (i_want_pos)
    {
        drawRect('blue', i_want_pos[0] % 8 * px, Math.floor(i_want_pos[0] / 8) * px, px/2, ctx)
        drawRect('blue', i_want_pos[1] % 8 * px, Math.floor(i_want_pos[1] / 8) * px, px/2, ctx)
    }

    if (!positionCount) {
        drawRect('green', ActiveX, ActiveY, px/2, ctx)
    } else {
        PeiceOnFocus = -1;
    }

    if (PeiceOnFocus.piece !== EMPTY) {
        if (GreenArcsArray.length === 0) drawRect('red', ActiveX, ActiveY, px/2, ctx)
        else drawRect('green', ActiveX, ActiveY, px/2, ctx)
        if (PeiceOnFocus.color !== MainDesk.stepColor) {
            drawRect('red', ActiveX, ActiveY, px/2, ctx);
            // ds.draw()
            ctx.drawImage(Images['stop'], ActiveX, ActiveY, px, px)
            GreenArcsArray = [];
            // return
        }
    }


    ds.draw()
    for (let arc of GreenArcsArray) {
        let y = Math.floor(arc / 8) * px, x = arc%8 * px;
        try {
            draw_circle('#00991e', x + px / 2, y + px / 2, 18 * px / 108, ctx)
            // ctx.drawImage(Images['circle'], x - px/1.35, y - px/1.35, px/2, px/2)
        } catch (e) {}
    }

    if (PeiceOnFocus.piece !== EMPTY) {
		ctx.globalAlpha = 0.9
		if (GreenArcsArray.length === 0) ctx.drawImage(Images['stop'], ActiveX, ActiveY, px, px)
		ctx.globalAlpha = 1
	}
}

let DEPTH = parseInt(document.getElementsByTagName('select')[0].value);
let MAX_DEPTH = parseInt(document.getElementsByTagName('select')[1].value)
function selectUpdate()
{
    DEPTH = parseInt(document.getElementsByTagName('select')[0].value);
    MAX_DEPTH = parseInt(document.getElementsByTagName('select')[1].value)
}

function getPosition(x, y) {
    return (Math.floor(x / px) + a / px * Math.floor(y / px));
}

let PeiceOnFocus = -1, GreenArcsArray = [];
let mousedown = async function (event) {
    let positionFrom = PeiceOnFocus = getPosition(event.offsetX, event.offsetY) ;
    PeiceOnFocus = MainDesk.getPiece(PeiceOnFocus);
    GreenArcsArray = MainDesk.get_all_moves(MainDesk.stepColor)[getPosition(event.offsetX, event.offsetY)] ?? [];
    // GreenArcsArray = await MainDesk.fetchGetAllMoves(PeiceOnFocus.position);
    console.log(GreenArcsArray)
    console.log(getPosition(event.offsetX, event.offsetY), PeiceOnFocus, GreenArcsArray, event.offsetX, event.offsetY)

    canvas.onmousedown = (event2) => {
        if (!GreenArcsArray.includes(getPosition(event2.offsetX, event2.offsetY))) {
            GreenArcsArray = []
            mousedown(event2);
            return
        }
        // MainDesk.move(positionFrom, getPosition(event2.offsetX, event2.offsetY), false, true)
        MainDesk.fetchMove(positionFrom, getPosition(event2.offsetX, event2.offsetY), false, true)
        GreenArcsArray = []
        PeiceOnFocus = -1;


        GreenArcsArray = []
        // makeTurn(MainDesk, MainDesk.stepColor, DEPTH);

        canvas.onmousedown = mousedown
    }
}
canvas.onmousedown = mousedown
// COMPUTER

let positionCount = 0;
let AlreadySolved = {};
let AlreadySolvedCount = 0;
let i_want_pos = null;
var minimax = async function (depth, game, alpha, beta, color, prolongationDepth = false) {
    positionCount++;
    if ((depth < 0 && !prolongationDepth) || depth < MAX_DEPTH*-1) {
        return game.status;
    }
    var available_moves = game.get_all_moves(color);
    let moves = [];
    for (let peice in available_moves) {
        for (let i = 0; i < available_moves[peice].length; i++) {
            let newGameMove = available_moves[peice][i];
            peice = parseInt(peice);
            game.move(peice, newGameMove);
            moves.push({
                move: [peice, newGameMove],
                status: game.status
            });
            game.undo()
        }
    }
    let sort_sign = color === WHITE ? -1 : 1;
    moves.sort((a, b) => {
        return sort_sign*(a.status-b.status)
    })

    if (color === WHITE) {
        var bestMove = -Infinity;
        for (let i = 0; i < moves.length; i++) {
            const move = moves[i];
            let isKilled = game.move(move.move[0], move.move[1])
            let minmaxResult = await minimax(depth - 1, game, alpha, beta,color === WHITE ? BLACK : WHITE, isKilled);
            bestMove = Math.max(bestMove, minmaxResult);
            game.undo();

            alpha = Math.max(alpha, bestMove);
            if (beta <= alpha) {
                return bestMove;
            }
        }
    } else {
        var bestMove = Infinity;
        for (let i = 0; i < moves.length; i++) {
            const move = moves[i];
            let isKilled = game.move(move.move[0], move.move[1])
            let minmaxResult = await minimax(depth - 1, game, alpha, beta,color === WHITE ? BLACK : WHITE, isKilled);
            bestMove = Math.min(bestMove, minmaxResult);
            game.undo();

            beta = Math.min(beta, bestMove);
            if (beta <= alpha) {
                return bestMove;
            }
        }
    }



    return bestMove;
};
let minimaxMain = async function(game, color, depth) {

    var available_moves = game.get_all_moves(color);

    var bestMove = color === WHITE ? -Infinity : Infinity;
    var bestMoveFound;

    let moves = [];
    for (let peice in available_moves) {
        for (let i = 0; i < available_moves[peice].length; i++) {
            var newGameMove = available_moves[peice][i]
            peice = parseInt(peice);
            let newDesk = new Desk(game.copy());
            newDesk.stepColor = color;

            newDesk.move(peice, newGameMove);
            moves.push({
                move: [peice, newGameMove],
                status: newDesk.status,
                desk: newDesk
            })
        }
    }
    let sort_sign = color === WHITE ? -1 : 1;
    moves.sort((a, b) => {
        return sort_sign*(a.status-b.status)
    })
    let moveStatusesSet = {};
    for (let i = 0; i < moves.length; i++) {
        const move = moves[i];
        let hash = await crc32()
        var value = await minimax(depth, move.desk, -10000, 10000, color === WHITE ? BLACK : WHITE);

        if (color === WHITE && value >= bestMove) {
            bestMove = value;
            bestMoveFound = move.move;
        }
        if (color === BLACK && value <= bestMove) {
            bestMove = value;
            bestMoveFound = move.move;
        }
        i_want_pos = bestMoveFound ? (bestMoveFound) : null;
        updateProgressBar(((i)/moves.length))
    }
    return bestMoveFound;
};
let timeToFunc = 0;
async function makeTurn(desk, color, depth, cycle = false) {
    console.log('Depth', depth, 'Max Depth', MAX_DEPTH)
    let t = performance.now();
    let bestMove = await minimaxMain(desk, color, depth);
    desk.move(bestMove[0], bestMove[1])
    let time = ((performance.now()-t)/1000).toFixed(3);
    writeToLog(time, positionCount)
    timeToFunc = 0
    positionCount = 0;
    i_want_pos = null;
    AlreadySolvedCount = 0;

    // if (cycle) makeTurn(MainDesk, MainDesk.stepColor, depth, cycle)
}
function writeToLog(time, positions)
{
    document.getElementById("AdminInfo").innerHTML = `
        <br>Speed: ${(positions/time).toFixed(0)}/s
        <br>Total ${positions} (${time} с)
        <br>------------------
    ` + document.getElementById("AdminInfo").innerHTML;
}
setInterval(() => {
    render(MainDesk)
}, 1000/60)
// OTHER
function rnd(arr) {
    arr[0] = Math.ceil(arr[0]);
    arr[1] = Math.floor(arr[1]);
    return Math.floor(Math.random() * (arr[1] - arr[0])) + arr[0];
}
function draw_circle(fillCircle, x, y, mass, ctx) {
    ctx.beginPath();
    ctx.shadowColor = fillCircle
    ctx.shadowBlur = 40
    ctx.fillStyle = fillCircle;
    ctx.strokeStyle = fillCircle;
    ctx.arc(x, y, mass, 0, Math.PI * 2);
    if (fillCircle) {
        ctx.fill();
    } else {

    }
}

function drawRect(fillCircle, x, y, mass, ctx) {
    ctx.beginPath();
    ctx.shadowColor = fillCircle
	ctx.shadowBlur = 40
    ctx.fillStyle = fillCircle;
    ctx.strokeStyle = fillCircle;
    ctx.rect(x, y, px, px);
    if (fillCircle) {
        ctx.fill();
    } else {

    }
}


async function crc32(message) {
    return await new Promise((r) => {
        setTimeout(() => r(true), 0)
    });
}
function updateProgressBar(p){
    document.getElementById('progressbar').style.height = (a*p) + 'px'
}
