class Desk {
    constructor(buffer = null) {
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
        this.updatePeopleThinkingDesk();
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
        for (let i = 0; i < 8; i++) {
            for (let j = 0; j < 8; j++) {
                let piece = reader.readUint8();
                let color = reader.readUint8();
                reader.position++;
                if (!piece) continue
                ctx.shadowBlur = 30
                ctx.shadowColor = color === BLACK ? 'black' : '#555'
                ctx.drawImage(Images[piece + '-' + color], j * px, i * px, px, px)

            }
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
        let cur_p, index;
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row, col + i)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row, col - i)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row - i, col)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row + i, col)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
    }
    getBishopMoves(row, col, color, array)
    {
        let cur_p, index;
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row + i, col + i)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row + i, col - i)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row - i, col + i)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
        for (let i = 1; i < 8; i++) {
            index = this.getIndex(row - i, col - i)
            if (index === -1) break
            cur_p = this.array[index];
            if (cur_p.type !== EMPTY && cur_p.color === color) break
            array.push(index)
            if (cur_p.type !== EMPTY && cur_p.color !== color) break
        }
    }
    getRowCol(pos)
    {
        return {row: Math.floor(pos / 8), col:pos%8}
    }
    getPlayerAllMoves(color)
    {
        let result = [];
        let pieces = this.piecePositions[PAWN|color];
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

    async fetchCurrent() {
        let response = await fetch('http://localhost:8080/command?input=get-current');
        this.loadDeskFromJson(await response.json());
    }
    async fetchUndo()
    {
        let response = await fetch('http://localhost:8080/command?input=undo');
        this.loadDeskFromJson(await response.json());
    }

    async fetchGetAllMoves(pos)
    {
        let response = await fetch(`http://localhost:8080/command?input=get-allow-moves ${MainDesk.toString()}${this.stepColor} ${pos}`);
        return (await response.text()).split(' ').map(Number);
    }
    async fetchMove(from, to) {
        let response = await fetch(`http://localhost:8080/command?input=move ${from} ${to}`);
        this.loadDeskFromJson(await response.json());
    }
    async fetchNextMove() {
        let response = await fetch('http://localhost:8080/command?input=do');
        this.loadDeskFromJson(await response.json());
    }
}