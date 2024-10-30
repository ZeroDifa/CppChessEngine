const express = require('express');
const app = express();
require('dotenv').config({path: '../.env'});

app.use(express.static('./client'));
app.listen(8080, () => {
    console.log('Server is running on http://localhost:8080');
    console.log('Press Ctrl+C to stop the server');
});

const WsServer = require('./WsServer');
console.log(WsServer);
const server = new WsServer(1337);

server.createParty({uuid: 'First', path: process.env.DIST_PATH + '/main.exe'});
server.createParty({uuid: '-o2', path: process.env.DIST_PATH + '/main_o2.exe'});
server.createParty({uuid: 'no_sort', path: process.env.DIST_PATH + '/main.exe', options: ['--no_sort']});
server.createParty({uuid: 'no_threads', path: process.env.DIST_PATH + '/main.exe', options: ['--no_threads']});

process.stdin.on('data', (data) => {
    server.handleProcessInput(data);
})