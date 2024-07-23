class Desk {
    constructor(buffer = null, createSocket = false) {
        if (buffer !== null) this.board = buffer;
        else {
            this.board = new ArrayBuffer(64*PIECE_BYTE_SIZE);
            this.fillBoard();
        }
        this.array = {}
        this.stepColor = WHITE;
        this.lastTurn = []
        this.saves = [];
        this.piecePositions = {};
        this.movesToAnimate = [];
        this.serverDeskStatus = null;
        this.updatePeopleThinkingDesk();
        if (createSocket) {
            this.ws = new WebSocket('ws://localhost:1337');
            this.ws.onmessage = this.onmessage.bind(this);
        }
        this.uuid = null;
    }
    onmessage(event) {

        let data;
        try {
            data = JSON.parse(event.data);
        } catch (e) {
            new PushNotify(event.data, 10000, 'red')
            return
        }
        if (data.type === 'new') {
            this.loadDeskFromJson(data);
        }
        console.log(data)
        switch (data.type) {
            case 'partyInit':
                this.uuid = data.uuid;
                this.fetchCurrent();
                updatePartySelect(data.partyList);
                break;
            case 'get-current':
                this.addMoveToAnimate(data.move);
                this.loadDeskFromJson(data);
                this.serverDeskStatus = data.status;
                break;
            case 'do':
                let counter = data.counter;
                writeToLog({
                    TotalTim: data.time,
                    stats: data.stats
                });
                last_move = [data.move.from, data.move.to]
                i_want_pos = null;
                this.addMoveToAnimate(data.move);
                this.loadDeskFromJson(data);
                this.serverDeskStatus = data.status;
                break;
            case 'partyList':
                updatePartySelect(data.data);
                break;
            case 'wantPos':
                i_want_pos = data.move;
                break;
            case 'get-allow-moves':
                GreenArcsArray = data.moves;
                break
        }
    }
    addMoveToAnimate(move) {
        if (move === undefined) return;
        this.movesToAnimate.push({
            from: move.from,
            to: move.to,
            x: (move.from % 8) * px,
            y: Math.floor(move.from / 8) * px,
            xTo: (move.to % 8) * px,
            yTo: Math.floor(move.to / 8) * px,
            pieceFrom: Object.assign(this.array[move.from], {}),
            pieceTo: Object.assign(this.array[move.to], {})
        })
        frameRate = 120;
    }
    fillBoard(SETUP = INITIAL_SETUP)
    {
        let writer = new Writer(this.board);
        for (let i = 0; i < 64; i++) {
            if (SETUP[i] !== 0) {
                writer.writeUint8(SETUP[i] & TYPE_SHIFT).writeUint8(SETUP[i] & COLOR_SHIFT).writeUint8((SETUP[i] & MOVED_SHIFT) === 0 ? NOT_MOVED : (SETUP[i] & MOVED_SHIFT))
            } else {
                writer.writeUint8(0).writeUint8(0).writeUint8(0)
            }
        }
    }
    toString()
    {
        let reader = new Reader(MainDesk.board)
        let output = '';
        for (let i = 1; i <= 64; i++) {
            const symbol = reader.readUint8() | reader.readUint8() | reader.readUint8();
            output += symbol.toString() + ' ';
        }

        return output;
    }
    print(){
        let reader = new Reader(this.board);
        let output = '';
        for (let i = 1; i <= 64; i++) {
            let [type, color] = [reader.readUint8(), reader.readUint8()];
            const symbol = SYMBOLS[type | color];
            output += symbol;
            reader.position++;
            if (i%8 === 0) output += '\n';
        }
        console.log(output);
    }

    reset(array = null) {
        if (array == null) {
            this.fillBoard();
            return;
        }
        this.board = array;
    }
    save(lastTurn) {
        this.saves.push(
            {
                turns: [
                    lastTurn
                ],
                stepColor: this.stepColor
            }
        )
    }
    appendMoveToLastSave(turn) {
        this.saves[this.saves.length-1].turns.push(Object.assign(turn, {}));
    }
    undo() {
        if(!this.saves.length) return
        let save = this.saves.pop()
        for (const turn of save.turns) {
            this.array[turn.from.position] = turn.from;
            this.array[turn.to.position] = turn.to;
            let writer = new Writer(this.board);

            writer.seek((turn.to.position)*PIECE_BYTE_SIZE).writeUint8(turn.to.type).writeUint8(turn.to.color).writeUint8(turn.to.isFirstMove)
            writer.seek((turn.from.position)*PIECE_BYTE_SIZE).writeUint8(turn.from.type).writeUint8(turn.from.color).writeUint8(turn.from.isFirstMove);

            this.piecePositions[turn.to.type|turn.to.color].push(turn.to.position);
            this.piecePositions[turn.from.type|turn.from.color][this.piecePositions[turn.from.type|turn.from.color].indexOf(turn.to.position)] = turn.from.position;
        }
        this.stepColor = save.stepColor;
    }
    draw() {
        let x,y;
        let reader = new Reader(this.board);
        ctx.globalAlpha = 0.2
        for (let i = 0; i < this.lastTurn.length; i++) {
            const cage = this.lastTurn[i];
            x = cage%8 * px; y = Math.floor(cage / 8) * px;
            drawRect('orange', x, y, px, ctx)
        }
        ctx.globalAlpha = 1
        let skipPositions = []
        for (let move of this.movesToAnimate) {
            skipPositions.push(move.to)
        }
        for (let i = 0; i < 8; i++) {
            for (let j = 0; j < 8; j++) {
                let piece = reader.readUint8();
                let color = reader.readUint8();
                reader.position++;
                if (!piece || skipPositions.includes(this.getIndex(i, j))) continue
                ctx.drawImage(Images[piece + '-' + color], j * px, i * px, px, px)
                if (document.getElementById('show-indexes').checked)
                {
                    ctx.fillStyle = 'black'
                    ctx.font = '15px Arial'
                    ctx.fillText(i*8+j, j*px, i*px+15)
                }
            }
        }
        for (let move of this.movesToAnimate) {
            move.x += (move.xTo - move.x) * 0.16;
            move.y += (move.yTo - move.y) * 0.16;
            let piece = move.pieceFrom
            ctx.drawImage(Images[piece.type + '-' + piece.color], move.x, move.y, px, px)
            let distance = Math.sqrt((move.x - move.xTo) ** 2 + (move.y - move.yTo) ** 2)
            if (distance < 1) {
                this.movesToAnimate.splice(this.movesToAnimate.indexOf(move), 1)
                frameRate = 1;
            }
        }
        if (this.serverDeskStatus !== null)
        {
            // ctx.fillStyle = 'rgba(0, 0, 0, 0.5)'
            // ctx.fillRect(0, 0, 8*px, 8*px)
            ctx.fillStyle = 'white'
            ctx.font = '20px Arial'
            ctx.fillText(this.serverDeskStatus, 8*px-50, 8*px/2)

        }
    }
    getIndex(row, col) {
        if (row < 0 || col < 0 || row > 7 || col > 7) return -1
        return 8 * row + col
    }
    get dump() {
        return this.board.toString()
    }
    get status() {
        let status = 0;
        let reader = new Reader(this.board);
        for (let i = 0; i < 64; i++) {
            const piece = reader.readUint8(), color = reader.readUint8();
            reader.position++;
            if (piece === 0) continue;
            status += DEFAULT_COST[piece]*(color === WHITE ? 1 : -1) + SQUARE_COST[piece|color][i];
        }
        return status;
    }
    saveDeskToLocalStorage()
    {
        let reader = new Reader(this.board);
        let savedDesk = [];
        for (let index = 0; index < 64; index++) {
            savedDesk.push(reader.readUint8()|reader.readUint8()|reader.readUint8())
        }
        localStorage.setItem("save", JSON.stringify({desk: savedDesk, stepColor:this.stepColor}));
    }
    loadDeskFromLocalStorage()
    {
        let data = JSON.parse(localStorage.getItem("save"));
        this.fillBoard(data.desk)
        this.updatePeopleThinkingDesk();
        this.stepColor = data.stepColor;
    }
    loadDeskFromJson(data)
    {
        this.fillBoard(data.desk)
        this.updatePeopleThinkingDesk();
        this.stepColor = data.stepColor;
    }
    check(color)
    {
        let kingPos = this.piecePositions[KING|color][0];
        let {row, col} = this.getRowCol(kingPos);
        let otherColor = color === WHITE ? BLACK : WHITE;
        let att = [];
        this.getKnightMoves(row, col, att);

        if ((this.piecePositions[KNIGHT | otherColor] ?? []).some(el => att.includes(el))) {
            return true;
        }
        let sign = color === WHITE ? -1 : 1;
        att = [
            this.getIndex(row + sign, col+1),
            this.getIndex(row + sign, col-1),
        ]
        let pieces = (this.piecePositions[PAWN|otherColor] ?? []);
        if ((att).some(el => pieces.includes(el))) {
            return true;
        }

        let rookMoves = [];
        this.getRookMoves(row, col, color, rookMoves);

        let bishopMoves = [];
        this.getBishopMoves(row, col, color, bishopMoves);
        let queenPos = this.piecePositions[QUEEN|otherColor] ?? [];
        if ((this.piecePositions[ROOK|otherColor] ?? []).concat(queenPos).some(el => rookMoves.includes(el))) {
            return true;
        }
        if ((this.piecePositions[BISHOP|otherColor] ?? []).concat(queenPos).some(el => bishopMoves.includes(el))) {
            return true;
        }
        att = [
            this.getIndex(row+1, col),
            this.getIndex(row+1, col-1),
            this.getIndex(row+1, col+1),
            this.getIndex(row, col-1),
            this.getIndex(row, col+1),
            this.getIndex(row-1, col),
            this.getIndex(row-1, col-1),
            this.getIndex(row-1, col+1),
        ]
        if (att.includes(this.piecePositions[KING|otherColor][0]))
        {
            return true;
        }
        return false;
    }
    getPiece(pos) {
        let reader = new Reader(this.board);
        reader.seek(pos*PIECE_BYTE_SIZE)

        return {position: pos, piece: reader.readUint8(), color: reader.readUint8(), isFirstMove: reader.readUint8()}
    }

    updatePeopleThinkingDesk()
    {
        let i = 0, reader = new Reader(this.board);
        this.array[-1] = {
            type: 0,
            color: 0,
            isFirstMove: 0
        }
        this.piecePositions = {
            // [EMPTY|WHITE]: [],
            // [EMPTY|BLACK]: []
        };
        while (i < 64)
        {
            let el = {
                type: reader.readUint8(),
                color: reader.readUint8(),
                isFirstMove: reader.readUint8()
            }
            this.array[i++] = el;
            // if (el.type === EMPTY) continue;
            this.piecePositions[el.type | el.color] = this.piecePositions[el.type | el.color]?.concat(i-1) ?? [i-1]
        }
    }
    getKnightMoves(row, col, array)
    {
        for (let i of [1, -1]) {
            array.push(this.getIndex(row + i, col + 2))
            array.push(this.getIndex(row + i, col - 2))
            array.push(this.getIndex(row + 2 * i, col + 1))
            array.push(this.getIndex(row + 2 * i, col - 1))
        }
    }
    getRookMoves(row, col, color, array) {
        const directions = [
            { dr: 0, dc: 1 }, 
            { dr: 0, dc: -1 },
            { dr: -1, dc: 0 },
            { dr: 1, dc: 0 }  
        ];
    
        directions.forEach(({ dr, dc }) => {
            for (let i = 1; i < 8; i++) {
                const index = this.getIndex(row + i * dr, col + i * dc);
                if (index === -1) break;
                const cur_p = this.array[index];
                if (cur_p.type !== EMPTY) {
                    if (cur_p.color === color) break;
                    array.push(index);
                    break;
                }
                array.push(index);
            }
        });
    }
    getBishopMoves(row, col, color, array)
    {
        const directions = [
            { dr: 1, dc: 1 },
            { dr: 1, dc: -1 },
            { dr: -1, dc: 1 },
            { dr: -1, dc: -1 }
        ];
        
        directions.forEach(({ dr, dc }) => {
            for (let i = 1; i < 8; i++) {
                const index = this.getIndex(row + i * dr, col + i * dc);
                if (index === -1) break;
                const cur_p = this.array[index];
                if (cur_p.type !== EMPTY) {
                    if (cur_p.color === color) break;
                    array.push(index);
                    break;
                }
                array.push(index);
            }
        });
    }
    getRowCol(pos)
    {
        return {row: Math.floor(pos / 8), col:pos%8}
    }
    getPlayerAllMoves(color)
    {
        let result = [];
        let pieces = this.piecePositions[PAWN|color] ?? [];
        let sign = color === WHITE ? -1 : 1;
        for (const piece of pieces) {
            result.push(piece+8*sign);
            if (this.array[piece].isFirstMove === NOT_MOVED)
            {
                result.push(piece+16*sign)
            }
        }
        return result;
    }
    getAllowCages(pos, check = true) {
        let reader = new Reader(this.board);
        reader.seek(pos*PIECE_BYTE_SIZE);
        let [piece, color, isFirstMove] = [reader.readUint8(), reader.readUint8(), reader.readUint8()];
        let pieceObj = this.array[pos];
        let {row, col} = this.getRowCol(pos);
        let result = [];
        if (pieceObj.type === 0) return []
        let index, current_piece, current_color;
        switch (pieceObj.type) {
            case PAWN: // pawn
                let sign = color === BLACK ? 1 : -1;
                index = this.getIndex(row + 1*sign, col)

                if (this.array[index].type === EMPTY) {
                    result.push(index);
                    index = this.getIndex(row + 2*sign, col);
                    if (pieceObj.isFirstMove === NOT_MOVED && this.array[index].type === EMPTY)
                    {
                        result.push(index)
                    }
                }
                index = this.getIndex(row + 1*sign, col+1);
                current_piece = this.array[index]
                if (current_piece.type !== EMPTY && current_piece.color !== pieceObj.color)
                {
                    result.push(index)
                }
                index = this.getIndex(row + 1*sign, col-1);
                current_piece = this.array[index]
                if (current_piece.type !== EMPTY && current_piece.color !== pieceObj.color)
                {
                    result.push(index)
                }
                break;
            case KNIGHT:
                this.getKnightMoves(row, col, result);
                break;
            case BISHOP: // Bishop
                this.getBishopMoves(row, col, color, result);
                break;
            case ROOK: // Rook
                this.getRookMoves(row, col, color, result);
                break;
            case QUEEN: // queen
                this.getRookMoves(row, col, color, result);
                this.getBishopMoves(row, col, color, result);
                break;
            case KING: // king
                result.push(this.getIndex(row + 1, col-1))
                result.push(this.getIndex(row + 1, col))
                result.push(this.getIndex(row + 1, col+1))
                result.push(this.getIndex(row - 1, col))
                result.push(this.getIndex(row - 1, col-1))
                result.push(this.getIndex(row - 1, col+1))
                result.push(this.getIndex(row, col+1))
                result.push(this.getIndex(row, col-1))

                reader.seek(this.getIndex(row, col+3)*PIECE_BYTE_SIZE)
                if (isFirstMove === NOT_MOVED && reader.readUint8()+reader.readUint8()+reader.readUint8() === ROOK+color+NOT_MOVED) {
                    result.push(this.getIndex(row, col+2));
                }
                reader.seek(this.getIndex(row, col-4)*PIECE_BYTE_SIZE)
                if (
                    isFirstMove === NOT_MOVED && reader.readUint8()+reader.readUint8()+reader.readUint8() === ROOK+color+NOT_MOVED &&
                    reader.seek(this.getIndex(row, col-1)*PIECE_BYTE_SIZE).readUint8() === EMPTY &&
                    reader.seek(this.getIndex(row, col-3)*PIECE_BYTE_SIZE).readUint8() === EMPTY
                ) {
                    result.push(this.getIndex(row, col-2))
                }
                break;
            case undefined:
                break
            default:
                break;
        }
        result = Array.from(new Set(result));
        for (let i = 0; i < result.length; i++) {
            let p = this.array[result[i]]
            if (p.type !== EMPTY && p.color === color) result.splice(i--, 1)
        }
        if (result.includes(-1)) result.splice(result.indexOf(-1), 1)
        if (check) result = this.checkInFuture(pieceObj, color, pos, result)
        return result
    }
    copy() {
        return this.board.slice();
    }
    checkInFuture(p, color, pos, turns) {
        for (let i = 0; i < turns.length; i++) {
            const turn = turns[i];
            this.move(pos, turn, true);
            let dc = this.check(color);
            this.undo()
            if (dc) turns.splice(i--, 1)
        }
        return turns
    }
    move(from, to, highPriority = false, newSave = true)
    {
        let isWasKilled = false;
        let reader = new Reader(this.board);
        let writer = new Writer(this.board);
        reader.seek((from)*PIECE_BYTE_SIZE);
        from = {
            position: from,
            type: reader.readUint8(),
            color: reader.readUint8(),
            isFirstMove: reader.readUint8()
        }
        reader.seek((to) * PIECE_BYTE_SIZE);
        to = {
            position: to,
            type: reader.readUint8(),
            color: reader.readUint8(),
            isFirstMove: reader.readUint8()
        }
        if (newSave) this.save({from: {...from}, to: {...to}});
        else this.appendMoveToLastSave({from: {...from}, to: {...to}});
        isWasKilled = to.type !== EMPTY;

        if (!highPriority && (from.color !== this.stepColor)) {
            return isWasKilled;
        }

        writer.seek((to.position)*PIECE_BYTE_SIZE).writeUint8(from.type).writeUint8(from.color).writeUint8(MOVED)
        writer.seek((from.position)*PIECE_BYTE_SIZE).writeUint8(0).writeUint8(0).writeUint8(0);

        this.array[from.position] = {
            type: 0,
            color: 0,
            isFirstMove: 0
        }
        from.isFirstMove = MOVED;
        this.array[to.position] = from;

        this.piecePositions[from.type|from.color][(this.piecePositions[from.type|from.color] ?? []).indexOf(from.position)] = to.position;
        this.piecePositions[to.type|to.color]?.splice(this.piecePositions[to.type|to.color].indexOf(to.position), 1)

        if (from.type === KING && Math.abs(to.position-from.position) === 2) {
            if (to.position-from.position < 0) this.move(from.position-4, from.position-1, true, false)
            else this.move(from.position+3, from.position+1, true, false)
        }
        if (!highPriority) this.stepColor = this.stepColor === WHITE ? BLACK : WHITE
        return isWasKilled
    }
    get_all_moves(color) {
        let reader = new Reader(this.board);
        let m = {};
        for (let i = 0; i < 64; i++) {
            let piece = reader.readUint8(), clr = reader.readUint8();
            reader.position++;
            if (piece === EMPTY || color !== clr) continue
            m[i] = this.getAllowCages(i);
        }
        return m
    }
    msg(message) {
        this.ws.send(JSON.stringify(message));
    }
    fetchCurrent() {
        this.msg({
            uuid: this.uuid,
            data: 'get-current'
        })
    }
    async fetchUndo()
    {

        this.msg({
            uuid: this.uuid,
            data: 'undo'
        });
    }

    fetchGetAllMoves(pos)
    {
        this.msg({
            uuid: this.uuid,
            data: 'get-allow-moves ' + pos
        })
    }
    fetchMove(from, to) {
        this.msg({
            uuid: this.uuid,
            data: 'move ' + from + ' ' + to
        });
    }
    fetchNextMove() {
        this.msg({
            uuid: this.uuid,
            data: 'do ' + DEPTH + ' ' + MAX_DEPTH
        });
    }
}