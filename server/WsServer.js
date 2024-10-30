const { spawn } = require('child_process');
const WebSocket = require('ws');
class WsServer {
    constructor(port) {
        this.port = port;
        this.wss = new WebSocket.Server({port: this.port});
        this.parties = {};
        this.partiesToConnections = {};
        this.wss.on('connection', this.onConnection.bind(this));
    }

    createParty(options = {}) {
        let uuid = options.uuid ?? Math.random().toString(36).substring(7);
        let party = spawn(options.path, options.options);
        party['uuid'] = uuid;
        this.parties[uuid] = party;
        this.partiesToConnections[uuid] = [];
        party.on('close', (code) => {
            console.log(`Calculator process exited with code ${code}`, uuid);
        });
        party.stdout.on('data', (data) => {
            console.log(`stdout: ${data}`);
            this.calculatePartyOutput(uuid, data.toString().trim())
        });
        party.stderr.on('data', (data) => {
            console.error(`stderr: ${data}`);
            this.calculatePartyOutput(uuid, data.toString().trim())
        })
        return uuid;
    }

    joinWsToParty(uuid, ws) {
        ws.partyUuid = uuid;
        this.partiesToConnections[uuid].push(ws);
    }
    leaveWsFromParty(ws) {
        this.partiesToConnections[ws.partyUuid].splice(this.partiesToConnections[ws.partyUuid].indexOf(ws), 1);
    }

    onConnection(ws, req) {
        console.log('New connection');
        let uuid = !Object.keys(this.parties).length ? this.createParty() : Object.keys(this.parties)[0];
        this.joinWsToParty(uuid, ws)

        ws.onmessage = this.onClientMessage.bind(this, ws);

        ws.send(JSON.stringify({
            partyList: Object.keys(this.parties),
            type: 'partyInit',
            uuid: uuid
        }));
        ws.on('close', () => {
            this.leaveWsFromParty(ws)
        });
    }


    onClientMessage(ws, message) {
        message = JSON.parse(message.data);
        let newParty;
        let response = {};
        let uuid = ws.partyUuid ?? null;
        switch (message.type) {
            case 'createParty':
                this.leaveWsFromParty(ws)
                uuid = this.createParty();
                response['uuid'] = uuid;
                response['type'] = 'partyInit';
                this.joinWsToParty(uuid, ws)
                break;
            case 'joinParty':
                this.leaveWsFromParty(ws)
                uuid = message.uuid;
                response['uuid'] = uuid;
                response['type'] = 'partyInit';
                this.joinWsToParty(uuid, ws)
                break;
            case 'deleteParty':
                uuid = ws.partyUuid;
                this.parties[uuid].kill();
                delete parties[uuid];
                newParty = Object.keys(this.parties).length === 0 ? this.createParty() : Object.keys(this.parties)[0];
                for (let client of this.partiesToConnections[uuid])
                {
                    this.leaveWsFromParty(client)
                    this.joinWsToParty(newParty, client)
                    client.send(JSON.stringify({
                        partyList: Object.keys(this.parties),
                        type: 'partyInit',
                        uuid: newParty
                    }));
                }
                response = null;
                break;
            default:
                response = null;
                uuid = message.uuid;
                this.parties[uuid].stdin.write(message.data + '\n');
                break;
        }
        if (response !== null)
        {
            response['partyList'] = Object.keys(this.parties);
            ws.send(JSON.stringify(response))
        }
    }
    handleProcessInput(data) {
        try {
            let strData = data.toString().trim();
            let uuid = strData.split(' ')[0];
            strData = strData.replace(uuid + ' ', '');
            this.parties[uuid].stdin.write(strData + '\n');
        } catch (e) {
            console.log(e)
        }
    }
    calculatePartyOutput(uuid, data) {
        this.partiesToConnections[uuid].forEach(ws => {
            ws.send(data);
        });
    }
}

module.exports = WsServer;