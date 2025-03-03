const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>Agriculture Monitoring System</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        :root {
            --primary-color: #4CAF50;
            --error-color: #f44336;
            --background-color: #f5f5f5;
            --card-color: #ffffff;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: Arial, sans-serif;
            background: var(--background-color);
            color: #333;
            min-height: 80vh;
            display: flex;
            flex-direction: column;
            padding: 12px;
        }
        .header {
            text-align: center;
            padding: 8px;
            margin-bottom: 15px;
        }
        .header h1 {
            color: var(--primary-color);
            font-size: 20px;
            margin: 0;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px;
            padding: 0;
            max-width: 600px;
            margin: 0 auto;
            flex: 1;
        }
        .card {
            background: var(--card-color);
            border-radius: 12px;
            padding: 15px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            text-align: center;
            min-height: 110px;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            align-items: center;
        }
        .status {
            padding: 5px 12px;
            border-radius: 15px;
            font-size: 13px;
            display: inline-block;
            font-weight: 500;
            margin-bottom: 5px;
        }
        .connected { background: rgba(76,175,80,0.1); color: var(--primary-color); }
        .disconnected { background: rgba(244,67,54,0.1); color: var(--error-color); }
        .value {
            font-size: 28px;
            font-weight: 600;
            margin: 8px 0;
            color: #2c3e50;
        }
        .error {
            color: var(--error-color);
            font-size: 12px;
            margin-top: 4px;
            display: none;
        }
        .label {
            color: #666;
            font-size: 14px;
            font-weight: 500;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 6px;
        }
        .label svg {
            width: 18px;
            height: 18px;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 52px;
            height: 28px;
            margin-top: 10px;
        }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #ccc;
            transition: .3s;
            border-radius: 28px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 22px;
            width: 22px;
            left: 3px;
            bottom: 3px;
            background-color: white;
            transition: .3s;
            border-radius: 50%;
        }
        input:checked + .slider { background-color: var(--primary-color); }
        input:checked + .slider:before { transform: translateX(24px); }
        .sensor-error {
            border: 1px solid var(--error-color);
        }
        .sensor-error .value { color: var(--error-color); }
        .sensor-error .error { display: block; }
        .pump-timer {
            font-size: 13px;
            color: #666;
            margin-top: 8px;
            visibility: hidden;
            font-weight: 500;
        }
        .pump-timer.active {
            visibility: visible;
            color: var(--primary-color);
        }
        @media (max-width: 400px) {
            .grid { gap: 10px; }
            .card { padding: 12px; min-height: 100px; }
            .value { font-size: 24px; }
        }
    </style>
</head>
<body>
    <div class="header">
        <span id="connection-status" class="status connected">Connected</span>
        <h1>Agriculture Monitoring System</h1>
    </div>

    <div class="grid">
        <div class="card" id="moisture-card">
            <div class="label">
                <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12,20A6,6 0 0,1 6,14C6,10 12,3.25 12,3.25C12,3.25 18,10 18,14A6,6 0 0,1 12,20Z" />
                </svg>
                Soil Moisture
            </div>
            <div class="value"><span id="moisture">--</span>%</div>
            <div class="error">Sensor disconnected</div>
        </div>

        <div class="card" id="temperature-card">
            <div class="label">
                <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M15,13V5A3,3 0 0,0 12,2A3,3 0 0,0 9,5V13A5,5 0 1,0 15,13M12,4A1,1 0 0,1 13,5V12.1L13.7,12.8C14.5,13.6 15,14.7 15,16A3,3 0 0,1 12,19A3,3 0 0,1 9,16C9,14.7 9.5,13.6 10.3,12.8L11,12.1V5A1,1 0 0,1 12,4Z" />
                </svg>
                Temperature
            </div>
            <div class="value"><span id="temperature">--</span> C</div>
            <div class="error">Sensor disconnected</div>
        </div>

        <div class="card" id="humidity-card">
            <div class="label">
                <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12,3.25C12,3.25 6,10 6,14C6,18.42 9.58,22 14,22C18.42,22 22,18.42 22,14C22,10 16,3.25 16,3.25M14,20A6,6 0 0,1 8,14C8,11.37 11.27,7.14 13,5.21C14.73,7.14 18,11.37 18,14A6,6 0 0,1 14,20Z" />
                </svg>
                Humidity
            </div>
            <div class="value"><span id="humidity">--</span>%</div>
            <div class="error">Sensor disconnected</div>
        </div>

        <div class="card">
            <div class="label">Water Pump</div>
            <label class="switch">
                <input type="checkbox" id="pumpSwitch" onclick="togglePump()">
                <span class="slider"></span>
            </label>
            <div id="pumpTimer" class="pump-timer">Auto-off in: 5s</div>
        </div>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        var pumpTimeout;
        const PUMP_MAX_TIME = 5000;
        let lastMessageTime = Date.now();
        const CONNECTION_TIMEOUT = 5000;

        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onopen = () => {
                document.getElementById('connection-status').className = 'status connected';
                document.getElementById('connection-status').innerHTML = 'Connected';
            };
            websocket.onclose = () => {
                document.getElementById('connection-status').className = 'status disconnected';
                document.getElementById('connection-status').innerHTML = 'Disconnected';
                updateAllSensorsError(true);
                setTimeout(initWebSocket, 2000);
            };
            websocket.onmessage = (event) => {
                lastMessageTime = Date.now();
                var data = JSON.parse(event.data);
                updateSensorValue('moisture', data.moisture, data.moisture_error);
                updateSensorValue('temperature', data.temperature, data.temperature_error);
                updateSensorValue('humidity', data.humidity, data.humidity_error);
                document.getElementById('pumpSwitch').checked = data.pumpState === 1;
            };
        }

        function updateAllSensorsError(hasError) {
            ['moisture', 'temperature', 'humidity'].forEach(sensorId => {
                const card = document.getElementById(sensorId + '-card');
                const element = document.getElementById(sensorId);
                if (hasError) {
                    element.innerHTML = '--';
                    card.classList.add('sensor-error');
                }
            });
        }

        function updateSensorValue(sensorId, value, hasError) {
            const element = document.getElementById(sensorId);
            const card = document.getElementById(sensorId + '-card');
            if (hasError) {
                element.innerHTML = '--';
                card.classList.add('sensor-error');
            } else {
                element.innerHTML = value;
                card.classList.remove('sensor-error');
            }
        }

        function togglePump() {
            const pumpSwitch = document.getElementById('pumpSwitch');
            const pumpTimer = document.getElementById('pumpTimer');
            
            if (pumpSwitch.checked) {
                let timeLeft = 5;
                pumpTimer.classList.add('active');
                
                const countdown = setInterval(() => {
                    timeLeft--;
                    pumpTimer.textContent = `Auto-off in: ${timeLeft}s`;
                    if (timeLeft <= 0) {
                        clearInterval(countdown);
                        pumpSwitch.checked = false;
                        pumpTimer.classList.remove('active');
                        websocket.send(JSON.stringify({command: 'pump', state: 0}));
                    }
                }, 1000);

                if (pumpTimeout) clearTimeout(pumpTimeout);
                pumpTimeout = setTimeout(() => {
                    clearInterval(countdown);
                    pumpTimer.classList.remove('active');
                }, PUMP_MAX_TIME);
            } else {
                if (pumpTimeout) clearTimeout(pumpTimeout);
                pumpTimer.classList.remove('active');
            }
            websocket.send(JSON.stringify({command: 'pump', state: pumpSwitch.checked ? 1 : 0}));
        }

        setInterval(() => {
            if (Date.now() - lastMessageTime > CONNECTION_TIMEOUT) {
                document.getElementById('connection-status').className = 'status disconnected';
                document.getElementById('connection-status').innerHTML = 'Disconnected';
                updateAllSensorsError(true);
            }
        }, 1000);

        window.addEventListener('load', initWebSocket);
    </script>
</body>
</html>
)rawliteral"; 