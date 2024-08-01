const WebSocket = require('ws');
const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 80; // WebSocket server port

const wss = new WebSocket.Server({ port: PORT });


wss.on('connection', ws => {
    console.log('Client connected');

    ws.on('message', message => {
        console.log('Received message:', message);
        const buffer = Buffer.from(message, 'utf-8'); // Creating a buffer from a UTF-8 string
        const utf8String = buffer.toString('utf-8'); // Converting the buffer back to a UTF-8 string
        // console.log(utf8String);
        // wss.clients.forEach(client => {
        //     if (client.readyState === WebSocket.OPEN) {
        //         client.send(utf8String);
        //     }
        // });
        wss.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(utf8String);
            }
        });
    });

    ws.on('close', () => {
        console.log('Client disconnected');
    });

    ws.on('error', err => {
        console.error('WebSocket error:', err);
    });
});

console.log(`WebSocket server listening on port ${PORT}`);
