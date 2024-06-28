class CheckersDesk extends Desk {
    constructor(buffer = null) {
        super(buffer);
    }
    fillBoard(SETUP = CHECKERS_INITIAL_SETUP)
    {
        super.fillBoard(SETUP);
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
        ctx.font = "48px serif";
        for (let i = 0; i < 8; i++) {
            for (let j = 0; j < 8; j++) {
                let piece = reader.readUint8();
                let color = reader.readUint8();
                ctx.fillText(j + i*8, (j) * px, (i+1) * px)
                reader.position++;
                if (!piece) continue
                ctx.shadowBlur = 30
                ctx.shadowColor = color === BLACK ? 'black' : '#555'
                ctx.drawImage(Images[color === BLACK ? 'checkblack' : 'checkwhite'], j * px + 5, i * px + 5, px-10, px-10)
            }
        }
    }
    getCycleMoves(row, col, result, piece, eaten) {
        let itMustKill = false;
        for (const i of [1, -1]) {
            for (const j of [1, -1]) {
                let index = this.getIndex(row + i, col + j);
                if (this.array[index].type !== EMPTY && this.array[index].color !== piece.color && this.array[this.getIndex(row + 2*i, col + 2*j)].type === EMPTY) {
                    index = this.getIndex(row + 2*i, col+2*j);
                    eaten[index] ??= [];
                    if (result.includes(index) || eaten[index].includes(this.getIndex(row + i, col + j)) || index < 0) continue;
                    result.push(index);
                    eaten[index].push(this.getIndex(row + i, col + j))
                    itMustKill = true;
                    this.getCycleMoves(row + 2*i, col+2*j, result, piece, eaten)
                }
            }
        }
        return itMustKill;
    }
    getAllowCages(pos, result) {
        let reader = new Reader(this.board);
        reader.seek(pos*PIECE_BYTE_SIZE);
        let [piece, color, isFirstMove] = [reader.readUint8(), reader.readUint8(), reader.readUint8()];
        let pieceObj = this.array[pos];
        let {row, col} = this.getRowCol(pos);
        if (pieceObj.type === 0) return []
        let index, isMustKill = false;
        switch (pieceObj.type) {
            case PAWN:
                let eaten = [];
                isMustKill = this.getCycleMoves(row, col, result[pos], pieceObj, eaten);
                console.log(eaten);
                if (isMustKill) break;
                let sign = color === BLACK ? 1 : -1;
                for (index of [this.getIndex(row + sign, col+1), this.getIndex(row + sign, col-1)]) {
                    if (this.array[index].type === EMPTY) {
                        result[pos].push(index);
                    }
                }
                break;
            case QUEEN:
                break
            default:
                break;
        }
        result[pos] = Array.from(new Set(result[pos]));
        for (let i = 0; i < result[pos].length; i++) {
            let p = this.array[result[pos][i]]
            if (p.type !== EMPTY && p.color === color) result[pos].splice(i--, 1)
        }
        if (result[pos].includes(-1)) result[pos].splice(result[pos].indexOf(-1), 1)

        return isMustKill;
    }
    get_all_moves(color) {
        let reader = new Reader(this.board);
        let m = {};
        let isMustKill = false;
        for (let i = 0; i < 64; i++) {
            let piece = reader.readUint8(), clr = reader.readUint8();
            reader.position++;
            if (piece === EMPTY || color !== clr) continue
            m[i] = [];
            let newValKill = this.getAllowCages(i, m);
            if (newValKill === true) isMustKill = true;
            m[i].push(newValKill);
        }
        for (const mKey in m) {
            if (m[mKey].pop() === false && isMustKill) m[mKey] = [];
        }
        return m
    }
    findDiagonalCells(cell1, cell2) {
        if (cell2 < cell1)
        {
            let t = cell2;
            cell2 = cell1;
            cell1 = t;
        }
        let diffHorizontal = cell1 % 8 - cell2 % 8;
        let diffVertical = Math.floor(cell1 / 8) - Math.floor(cell2 / 8);
        if (Math.abs(diffHorizontal) !== Math.abs(diffVertical)) {
            return [];
        }
        let step, currentCell;
        if (diffHorizontal > 0) {
            step = 7
            currentCell = cell1 + step;
        } else {
            step = 9
            currentCell = cell1 + step;
        }
        let diagonalCells = [];
        while (currentCell < cell2) {
            diagonalCells.push(currentCell);
            currentCell += step;
        }

        return diagonalCells;
    }
    move(from, to, highPriority = false, newSave = true) {
        let moves = {
            [from]: []
        };

        this.getAllowCages(from, moves);

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

        }
        if (!highPriority) this.stepColor = this.stepColor === WHITE ? BLACK : WHITE
        return isWasKilled
    }
}