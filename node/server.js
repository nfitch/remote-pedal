#!/usr/bin/env node

var WebSocketServer = require('ws').Server;
var server = require('http').createServer();
var repl = require('repl');

var prompt = '> '
var wss = new WebSocketServer({server: server});
var conns = [];

wss.on('connection', function (ws) {
    console.log('\nclient connected');
    conns.push(ws);
    ws.on('close', function () {
        conns.splice(conns.indexOf(ws), 1);
        console.log('\nclient disconnected');
    });
});

var help = [
    'c => print connections',
    's => send message: s <l|r|m> [duration] [pot]',
    'h => help'
].join('\n');

var buttons = {
    "l": "left",
    "r": "right",
    "m": "middle"
}

function sendMessage(parts, cb) {
    if (!buttons.hasOwnProperty(parts[1])) {
        console.log("invalid button: " + parts[1]);
        return (cb(null, parts[0]));
    }
    if (conns.length < 1) {
        console.log("no connections... bailing...");
        return (cb(null, parts[0]));
    }
    var m = {
        "v": 0,
        "b": buttons[parts[1]],
        "d": parts[2] ? parseInt(parts[2]) : 0,
        "p": parts[3] ? parseInt(parts[3]) : 0,
    };
    var data = JSON.stringify(m, 0);
    var count = 0;
    function r() {
        ++count;
        if (count == conns.length) {
            return (cb(null, parts[0]));
        }
    }
    conns.forEach(function (conn) {
        conn.send(data, r);
    });
}

function ev(cmd, context, filename, cb) {
    var parts = cmd.trim().split(" ");
    switch(parts[0]) {
    case 'c':
        console.log(conns);
        break;
    case 's':
        return (sendMessage(parts, cb));
        break;
    case 'h':
        console.log(help);
        break;
    default:
        console.log(parts[0] + " not understood");
    }
    return (cb(null, parts[0]));
}

server.listen(8090, function () {
    console.log('Listening on http://localhost:8090');
    repl.start({prompt: prompt, eval: ev});
});
