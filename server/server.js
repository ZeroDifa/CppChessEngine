const { spawn } = require('child_process');
const express = require('express');
const WebSocket = require('ws');
const {response} = require("express");
const { log } = require('console');
//env_config from parent folder

require('dotenv').config({path: '../.env'});


const app = express();

app.use(express.static('./client'));
app.listen(8080, () => {
    console.log('Server is running on http://localhost:8080');
    console.log('Press Ctrl+C to stop the server');
});
const wss = new WebSocket.Server({port: 1337});
log('Server started');

const calculators = {};
const calculatorsToConnections = {};
createParty({uuid: 'First'});
createParty({uuid: '-o2', path: '../main_o2.exe'});
createParty({uuid: 'no_sort', path: '../main.exe', options: ['--no_sort']});
createParty({uuid: 'no_threads', path: '../main.exe', options: ['--no_threads']});
function createParty(args = {}  )
{
    const calculator = spawn(args.path ?? '../main.exe', args.options);
    let uuid = args.uuid ?? Math.random().toString(36).substring(7);
    calculator['uuid'] = uuid;
    calculators[uuid] = calculator;
    calculatorsToConnections[uuid] = [];
    calculator.on('close', (code)  => {
        console.log(`Calculator process exited with code ${code}`, uuid);
    });
    calculator.stdout.on('data', (data) => {
        console.log(`stdout: ${data}`);
        calculatorOutput(calculator.uuid, data.toString().trim())
    });
    calculator.stderr.on('data', (data) => {
        console.error(`stderr: ${data}`);
        calculatorOutput(calculator.uuid, data.toString().trim())
    })
    return uuid;
}
function calculatorOutput(uuid, data) {
    // try {
    //     JSON.parse(data);
    //     calculatorsToConnections[uuid].forEach(ws => {
    //         ws.send(data);
    //     });
    // } catch (e) {
    //     return false;
    // }
    // if data has }{ then split it
    if (data.includes('}{'))
    {
        return
    }
    calculatorsToConnections[uuid].forEach(ws => {
        ws.send(data);
    });
}
// консольный eval
process.stdin.on('data', (data) => {
    try {
        let strData = data.toString().trim();
        let uuid = strData.split(' ')[0];
        strData = strData.replace(uuid + ' ', '');
        calculators[uuid].stdin.write(strData + '\n');
    } catch (e) {
        console.log(e)
    }
})
function joinWsToParty(uuid, ws)
{
    ws.partyUuid = uuid;
    calculatorsToConnections[uuid].push(ws);
}
function leaveWsFromParty(ws)
{
    calculatorsToConnections[ws.partyUuid].splice(calculatorsToConnections[ws.partyUuid].indexOf(ws), 1);
}
wss.on('connection', (ws, req) => {
    let uuid = !Object.keys(calculators).length ? createParty() : Object.keys(calculators)[0];
    joinWsToParty(uuid, ws)
    ws.on('message', (message) => {
        message = JSON.parse(message);
        let newParty;
        let response = {};
        switch (message.type) {
            case 'createParty':
                leaveWsFromParty(ws)
                uuid = createParty();
                response['uuid'] = uuid;
                response['type'] = 'partyInit';
                joinWsToParty(uuid, ws)
                break;
            case 'joinParty':
                leaveWsFromParty(ws)
                uuid = message.uuid;
                response['uuid'] = uuid;
                response['type'] = 'partyInit';
                joinWsToParty(uuid, ws)
                break;
            case 'deleteParty':
                uuid = ws.partyUuid;
                calculators[uuid].kill();
                delete calculators[uuid];
                newParty = Object.keys(calculators).length === 0 ? createParty() : Object.keys(calculators)[0];
                for (let client of calculatorsToConnections[uuid])
                {
                    leaveWsFromParty(client)
                    joinWsToParty(newParty, client)
                    client.send(JSON.stringify({
                        partyList: Object.keys(calculators),
                        type: 'partyInit',
                        uuid: newParty
                    }));
                }
                response = null;
                break;
            default:
                response = null;
                uuid = message.uuid;
                calculators[uuid].stdin.write(message.data + '\n');
                break;
        }
        if (response !== null)
        {
            response['partyList'] = Object.keys(calculators);
            ws.send(JSON.stringify(response))
        }
    });

    ws.send(JSON.stringify({
        partyList: Object.keys(calculators),
        type: 'partyInit',
        uuid: uuid
    }));
    ws.on('close', () => {
        leaveWsFromParty(ws)
    });
});
