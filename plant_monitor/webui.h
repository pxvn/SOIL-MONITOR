#ifndef WEBUI_H
#define WEBUI_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Plant Guardian Pro</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: system-ui, -apple-system, sans-serif;
            -webkit-tap-highlight-color: transparent;
        }
        :root {
            --primary: #4CAF50;
            --danger: #f44336;
            --warning: #ff9800;
            --bg: #f5f5f5;
            --card: #fff;
        }
        body {
            background: var(--bg);
            padding: 1rem;
            line-height: 1.5;
        }
        .header {
            background: var(--primary);
            color: white;
            padding: 1.5rem;
            border-radius: 1rem;
            margin-bottom: 1rem;
            text-align: center;
        }
        .status {
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            background: rgba(255,255,255,0.2);
            padding: 0.5rem 1rem;
            border-radius: 2rem;
            margin-top: 0.5rem;
            font-size: 0.9rem;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
            gap: 1rem;
            margin-bottom: 1rem;
        }
        .card {
            background: var(--card);
            padding: 1rem;
            border-radius: 1rem;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        .card.error {
            border: 2px solid var(--danger);
        }
        .value {
            font-size: 2rem;
            font-weight: bold;
            margin: 0.5rem 0;
        }
        .label {
            font-size: 0.9rem;
            color: #666;
        }
        .controls {
            background: var(--card);
            padding: 1rem;
            border-radius: 1rem;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .switch-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid #eee;
        }
        .switch {
            position: relative;
            width: 60px;
            height: 34px;
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: var(--primary);
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        .button {
            width: 100%;
            padding: 1rem;
            border: none;
            border-radius: 0.5rem;
            background: var(--primary);
            color: white;
            font-size: 1rem;
            font-weight: bold;
            cursor: pointer;
        }
        .button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }
        .toast {
            position: fixed;
            bottom: 1rem;
            left: 50%;
            transform: translateX(-50%);
            background: rgba(0,0,0,0.8);
            color: white;
            padding: 0.75rem 1.5rem;
            border-radius: 2rem;
            font-size: 0.9rem;
            opacity: 0;
            transition: 0.3s;
        }
        .toast.show {
            opacity: 1;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>Plant Guardian Pro</h1>
        <p>Smart Plant Monitor</p>
        <div class="status" id="wifi-status">
            <span>●</span>
            <span id="status-text">Connected</span>
        </div>
    </div>

    <div class="grid">
        <div class="card" id="moisture-card">
            <div class="value" id="moisture-value">---%</div>
            <div class="label">Soil Moisture</div>
        </div>
        <div class="card" id="temp-card">
            <div class="value" id="temp-value">---°C</div>
            <div class="label">Temperature</div>
        </div>
        <div class="card" id="humidity-card">
            <div class="value" id="humidity-value">---%</div>
            <div class="label">Humidity</div>
        </div>
    </div>

    <div class="controls">
        <div class="switch-row">
            <span>Auto Mode</span>
            <label class="switch">
                <input type="checkbox" id="auto-mode" checked onchange="toggleAuto()">
                <span class="slider"></span>
            </label>
        </div>
        <button class="button" id="pump-button" onclick="togglePump()">
            Start Watering
        </button>
    </div>

    <div class="toast" id="toast"></div>

    <script>
        let pumpActive = false;
        let eventSource;

        function showToast(message) {
            const toast = document.getElementById('toast');
            toast.textContent = message;
            toast.classList.add('show');
            setTimeout(() => toast.classList.remove('show'), 3000);
        }

        function updateCard(id, value, unit) {
            const card = document.getElementById(id + '-card');
            const valueElem = document.getElementById(id + '-value');
            
            if (value === null) {
                card.classList.add('error');
                valueElem.textContent = 'Error';
                return;
            }
            
            card.classList.remove('error');
            valueElem.textContent = `${value}${unit}`;
        }

        function toggleAuto() {
            const isAuto = document.getElementById('auto-mode').checked;
            fetch('/api/control?auto=' + isAuto)
                .catch(() => showToast('Failed to change mode'));
        }

        function togglePump() {
            pumpActive = !pumpActive;
            const btn = document.getElementById('pump-button');
            btn.textContent = pumpActive ? 'Stop Watering' : 'Start Watering';
            
            fetch('/api/control?pump=' + pumpActive)
                .catch(() => {
                    pumpActive = !pumpActive;
                    btn.textContent = pumpActive ? 'Stop Watering' : 'Start Watering';
                    showToast('Failed to control pump');
                });
        }

        function initEventSource() {
            if (eventSource) eventSource.close();
            
            eventSource = new EventSource('/events');
            
            eventSource.addEventListener('sensors', e => {
                const data = JSON.parse(e.data);
                updateCard('moisture', data.soil_moisture, '%');
                updateCard('temp', data.temperature, '°C');
                updateCard('humidity', data.humidity, '%');
                
                document.getElementById('pump-button').disabled = data.sensor_error;
            });

            eventSource.addEventListener('pump', e => {
                pumpActive = e.data === 'pump_on';
                document.getElementById('pump-button').textContent = 
                    pumpActive ? 'Stop Watering' : 'Start Watering';
                showToast(pumpActive ? 'Watering started' : 'Watering stopped');
            });

            eventSource.addEventListener('auto', e => {
                const isAuto = e.data === 'auto_on';
                document.getElementById('auto-mode').checked = isAuto;
                showToast(isAuto ? 'Auto mode enabled' : 'Auto mode disabled');
            });

            eventSource.onerror = () => {
                document.getElementById('status-text').textContent = 'Disconnected';
                document.getElementById('wifi-status').style.background = 'rgba(244,67,54,0.2)';
                setTimeout(initEventSource, 5000);
            };
        }

        // Start the app
        document.addEventListener('DOMContentLoaded', () => {
            initEventSource();
            
            // Initial status fetch
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    updateCard('moisture', data.soil_moisture, '%');
                    updateCard('temp', data.temperature, '°C');
                    updateCard('humidity', data.humidity, '%');
                    document.getElementById('auto-mode').checked = data.auto_mode;
                    document.getElementById('pump-button').textContent = 
                        data.pump_active ? 'Stop Watering' : 'Start Watering';
                })
                .catch(() => showToast('Failed to get initial status'));
        });
    </script>
</body>
</html>
)rawliteral";

#endif 