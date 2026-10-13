const net = require('net');
const http = require('http');
const express = require('express');
const { WebSocketServer } = require('ws');
const path = require('path');
const Aedes = require('aedes').Aedes || require('aedes').default || require('aedes');
const aedes = new Aedes();


// 1. MQTT TCP Broker di port 1883 (untuk NodeMCU ESP8266)
const MQTT_PORT = 1883;
const tcpServer = net.createServer((socket) => {
    aedes.handle(socket);
});

tcpServer.listen(MQTT_PORT, '0.0.0.0', () => {
    console.log(`[MQTT] Broker TCP aktif di port ${MQTT_PORT} (0.0.0.0:${MQTT_PORT})`);
});

// 2. Web Server & WebSocket Dashboard di port 3000
const app = express();
const httpServer = http.createServer(app);
const wss = new WebSocketServer({ server: httpServer });

app.use(express.static(path.join(__dirname, 'public')));

// Broadcast data telemetry ke semua browser dashboard
function broadcastToBrowser(data) {
    const payload = JSON.stringify(data);
    wss.clients.forEach(client => {
        if (client.readyState === 1) {
            client.send(payload);
        }
    });
}

// Event Listeners Aedes
aedes.on('client', (client) => {
    console.log(`[MQTT] Klien terhubung: ${client ? client.id : 'unknown'}`);
    broadcastToBrowser({ type: 'client_connect', id: client ? client.id : 'unknown', time: new Date().toLocaleTimeString() });
});

aedes.on('clientDisconnect', (client) => {
    console.log(`[MQTT] Klien terputus: ${client ? client.id : 'unknown'}`);
    broadcastToBrowser({ type: 'client_disconnect', id: client ? client.id : 'unknown', time: new Date().toLocaleTimeString() });
});

aedes.on('publish', (packet, client) => {
    if (packet && packet.topic) {
        // Jangan cetak internal telemetry aedes
        if (packet.topic.startsWith('$SYS')) return;

        const payloadStr = packet.payload ? packet.payload.toString() : '';
        console.log(`[MQTT MSG] Topik: ${packet.topic} => ${payloadStr}`);

        broadcastToBrowser({
            type: 'mqtt_message',
            topic: packet.topic,
            payload: payloadStr,
            time: new Date().toLocaleTimeString()
        });
    }
});

const WEB_PORT = 3000;
httpServer.listen(WEB_PORT, '0.0.0.0', () => {
    console.log(`[DASHBOARD] Web UI aktif di http://localhost:${WEB_PORT}`);
});
