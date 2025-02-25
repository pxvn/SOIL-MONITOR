#ifndef WEBUI_H
#define WEBUI_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Plant Guardian Pro</title>
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
    <style>
        :root {
            --primary: #4CAF50;
            --secondary: #2196F3;
            --warning: #FFC107;
            --danger: #F44336;
            --success: #4CAF50;
            --background: #F5F5F5;
            --card: #FFFFFF;
            --text: #333333;
            --border: #E0E0E0;
            --shadow: rgba(0, 0, 0, 0.1);
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Roboto', sans-serif;
        }

        body {
            background: var(--background);
            color: var(--text);
            min-height: 100vh;
            line-height: 1.6;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 2rem;
        }

        .header {
            background: var(--primary);
            color: white;
            padding: 2rem;
            border-radius: 20px;
            margin-bottom: 2rem;
            position: relative;
            overflow: hidden;
            box-shadow: 0 4px 20px var(--shadow);
        }

        .header::after {
            content: '';
            position: absolute;
            top: -50%;
            right: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.2) 0%, transparent 60%);
            transform: rotate(30deg);
        }

        .header-content {
            position: relative;
            z-index: 1;
        }

        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 0.5rem;
            font-weight: 700;
        }

        .header p {
            font-size: 1.2rem;
            opacity: 0.9;
        }

        .status-badge {
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            padding: 0.5rem 1rem;
            background: var(--success);
            color: white;
            border-radius: 50px;
            font-size: 0.9rem;
            margin-top: 1rem;
            transition: background-color 0.3s ease;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 2rem;
            margin-bottom: 2rem;
        }

        .card {
            background: var(--card);
            border-radius: 20px;
            padding: 1.5rem;
            box-shadow: 0 4px 20px var(--shadow);
            transition: transform 0.3s ease;
        }

        .card:hover {
            transform: translateY(-5px);
        }

        .card.error {
            border: 2px solid var(--danger);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
        }

        .card-icon {
            font-size: 2.5rem;
            color: var(--primary);
        }

        .card-trend {
            display: flex;
            align-items: center;
            font-size: 0.9rem;
            color: var(--success);
        }

        .card-trend.negative {
            color: var(--danger);
        }

        .card-value {
            font-size: 2.5rem;
            font-weight: 700;
            margin: 1rem 0;
        }

        .card-label {
            color: #666;
            font-size: 1rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
        }

        .dot-success { background: var(--success); }
        .dot-warning { background: var(--warning); }
        .dot-danger { background: var(--danger); }

        .controls {
            background: var(--card);
            border-radius: 20px;
            padding: 1.5rem;
            margin-bottom: 2rem;
            box-shadow: 0 4px 20px var(--shadow);
        }

        .controls-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid var(--border);
        }

        .switch {
            position: relative;
            display: inline-block;
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
            background-color: var(--success);
        }

        input:checked + .slider:before {
            transform: translateX(26px);
        }

        .button {
            padding: 1rem 2rem;
            border: none;
            border-radius: 15px;
            background: var(--primary);
            color: white;
            font-weight: 500;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
            width: 100%;
            font-size: 1.1rem;
            transition: all 0.3s ease;
        }

        .button:hover {
            background: #43A047;
            transform: translateY(-2px);
        }

        .button:active {
            transform: translateY(0);
        }

        .toast {
            position: fixed;
            bottom: 2rem;
            left: 50%;
            transform: translateX(-50%);
            background: rgba(0, 0, 0, 0.8);
            color: white;
            padding: 1rem 2rem;
            border-radius: 50px;
            font-size: 0.9rem;
            opacity: 0;
            transition: opacity 0.3s ease;
            z-index: 1000;
        }

        .toast.show {
            opacity: 1;
        }

        @media (max-width: 768px) {
            .container {
                padding: 1rem;
            }

            .header {
                padding: 1.5rem;
                border-radius: 15px;
            }

            .header h1 {
                font-size: 2rem;
            }

            .grid {
                gap: 1rem;
            }

            .card {
                padding: 1rem;
            }

            .card-value {
                font-size: 2rem;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="header-content">
                <h1>Plant Guardian Pro</h1>
                <p>Smart Plant Monitoring System</p>
                <div class="status-badge" id="connection-status">
                    <span class="material-icons">wifi</span>
                    <span id="connection-text">Connected</span>
                </div>
            </div>
        </div>

        <div class="grid">
            <div class="card" id="moisture-card">
                <div class="card-header">
                    <span class="material-icons card-icon">water_drop</span>
                    <div class="card-trend" id="moisture-trend"></div>
                </div>
                <div class="card-value" id="moisture-value">---%</div>
                <div class="card-label">
                    <span class="status-dot dot-success"></span>
                    Soil Moisture
                </div>
            </div>

            <div class="card" id="temperature-card">
                <div class="card-header">
                    <span class="material-icons card-icon">thermostat</span>
                    <div class="card-trend" id="temperature-trend"></div>
                </div>
                <div class="card-value" id="temperature-value">---°C</div>
                <div class="card-label">
                    <span class="status-dot dot-success"></span>
                    Temperature
                </div>
            </div>

            <div class="card" id="humidity-card">
                <div class="card-header">
                    <span class="material-icons card-icon">water</span>
                    <div class="card-trend" id="humidity-trend"></div>
                </div>
                <div class="card-value" id="humidity-value">---%</div>
                <div class="card-label">
                    <span class="status-dot dot-success"></span>
                    Humidity
                </div>
            </div>

            <div class="card" id="water-level-card">
                <div class="card-header">
                    <span class="material-icons card-icon">opacity</span>
                    <div class="card-trend" id="water-level-trend"></div>
                </div>
                <div class="card-value" id="water-level-value">---%</div>
                <div class="card-label">
                    <span class="status-dot dot-success"></span>
                    Water Level
                </div>
            </div>
        </div>

        <div class="controls">
            <div class="controls-header">
                <span>Automatic Mode</span>
                <label class="switch">
                    <input type="checkbox" id="auto-mode" checked onchange="toggleAutoMode()">
                    <span class="slider"></span>
                </label>
            </div>
            <button class="button" onclick="toggleWatering()" id="water-button">
                <span class="material-icons">power</span>
                Start Watering
            </button>
        </div>
    </div>

    <div class="toast" id="toast"></div>

    <script>
        let isWatering = false;
        let lastValues = {
            moisture: 0,
            temperature: 0,
            humidity: 0,
            waterLevel: 0
        };
        let eventSource;

        function initEventSource() {
            if (eventSource) {
                eventSource.close();
            }

            eventSource = new EventSource('/events');

            eventSource.addEventListener('wifi', function(e) {
                const status = e.data;
                document.getElementById('connection-text').textContent = status === 'connected' ? 'Connected' : 'Disconnected';
                document.getElementById('connection-status').style.background = status === 'connected' ? 'var(--success)' : 'var(--danger)';
                
                if (status === 'disconnected') {
                    ['moisture', 'temperature', 'humidity', 'water-level'].forEach(id => 
                        updateSensorCard(null, id, '', 0)
                    );
                }
            });

            eventSource.addEventListener('sensors', function(e) {
                const data = JSON.parse(e.data);
                updateSensorCard(data.soil_moisture, 'moisture', '%', lastValues.moisture);
                updateSensorCard(data.temperature, 'temperature', '°C', lastValues.temperature);
                updateSensorCard(data.humidity, 'humidity', '%', lastValues.humidity);
                updateSensorCard(data.water_level, 'water-level', '%', lastValues.waterLevel);

                lastValues = {
                    moisture: data.soil_moisture,
                    temperature: data.temperature,
                    humidity: data.humidity,
                    waterLevel: data.water_level
                };
            });

            eventSource.addEventListener('pump', function(e) {
                const status = e.data;
                isWatering = status === 'pump_on';
                const button = document.getElementById('water-button');
                
                if (isWatering) {
                    button.innerHTML = '<span class="material-icons">power_off</span>Stop Watering';
                    showToast('Watering started');
                } else {
                    button.innerHTML = '<span class="material-icons">power</span>Start Watering';
                    showToast('Watering stopped');
                }
            });

            eventSource.addEventListener('auto', function(e) {
                const status = e.data;
                const checkbox = document.getElementById('auto-mode');
                checkbox.checked = status === 'auto_on';
                showToast(status === 'auto_on' ? 'Automatic mode enabled' : 'Automatic mode disabled');
            });

            eventSource.onerror = function(err) {
                console.error("EventSource failed:", err);
                document.getElementById('connection-text').textContent = 'Disconnected';
                document.getElementById('connection-status').style.background = 'var(--danger)';
                
                ['moisture', 'temperature', 'humidity', 'water-level'].forEach(id => 
                    updateSensorCard(null, id, '', 0)
                );

                // Try to reconnect after 5 seconds
                setTimeout(initEventSource, 5000);
            };
        }

        const showToast = (message) => {
            const toast = document.getElementById('toast');
            toast.textContent = message;
            toast.classList.add('show');
            setTimeout(() => toast.classList.remove('show'), 3000);
        };

        const toggleAutoMode = () => {
            const isAutoMode = document.getElementById('auto-mode').checked;
            fetch('/api/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ auto_mode: isAutoMode })
            });
        };

        const toggleWatering = () => {
            isWatering = !isWatering;
            fetch('/api/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ pump: isWatering })
            });
        };

        const updateSensorCard = (value, id, unit, lastValue) => {
            const card = document.getElementById(`${id}-card`);
            const valueElement = document.getElementById(`${id}-value`);
            const trendElement = document.getElementById(`${id}-trend`);
            const dotElement = card.querySelector('.status-dot');

            if (value === null) {
                card.classList.add('error');
                valueElement.textContent = 'Error';
                trendElement.innerHTML = '<span class="material-icons">error</span>Disconnected';
                dotElement.className = 'status-dot dot-danger';
                return;
            }

            card.classList.remove('error');
            valueElement.textContent = `${value}${unit}`;

            // Update status dot based on value ranges
            if (id === 'moisture') {
                if (value < 30) {
                    dotElement.className = 'status-dot dot-danger';
                } else if (value < 50) {
                    dotElement.className = 'status-dot dot-warning';
                } else {
                    dotElement.className = 'status-dot dot-success';
                }
            } else if (id === 'temperature') {
                if (value > 30 || value < 15) {
                    dotElement.className = 'status-dot dot-danger';
                } else if (value > 28 || value < 18) {
                    dotElement.className = 'status-dot dot-warning';
                } else {
                    dotElement.className = 'status-dot dot-success';
                }
            } else if (id === 'humidity') {
                if (value < 40 || value > 80) {
                    dotElement.className = 'status-dot dot-danger';
                } else if (value < 50 || value > 70) {
                    dotElement.className = 'status-dot dot-warning';
                } else {
                    dotElement.className = 'status-dot dot-success';
                }
            } else if (id === 'water-level') {
                if (value < 20) {
                    dotElement.className = 'status-dot dot-danger';
                } else if (value < 40) {
                    dotElement.className = 'status-dot dot-warning';
                } else {
                    dotElement.className = 'status-dot dot-success';
                }
            }

            if (lastValue !== 0) {
                const change = ((value - lastValue) / lastValue * 100).toFixed(1);
                const isPositive = change > 0;
                trendElement.innerHTML = `
                    <span class="material-icons">${isPositive ? 'arrow_upward' : 'arrow_downward'}</span>
                    ${isPositive ? '+' : ''}${change}%
                `;
                trendElement.className = `card-trend${isPositive ? '' : ' negative'}`;
            }
        };

        // Initialize the EventSource connection
        initEventSource();

        // Initial status fetch
        fetch('/api/status')
            .then(response => response.json())
            .then(data => {
                updateSensorCard(data.soil_moisture, 'moisture', '%', 0);
                updateSensorCard(data.temperature, 'temperature', '°C', 0);
                updateSensorCard(data.humidity, 'humidity', '%', 0);
                updateSensorCard(data.water_level, 'water-level', '%', 0);
                
                document.getElementById('auto-mode').checked = data.auto_mode;
                document.getElementById('connection-text').textContent = data.wifi_connected ? 'Connected' : 'Disconnected';
                document.getElementById('connection-status').style.background = data.wifi_connected ? 'var(--success)' : 'var(--danger)';
            })
            .catch(error => {
                console.error('Error fetching initial status:', error);
                document.getElementById('connection-text').textContent = 'Disconnected';
                document.getElementById('connection-status').style.background = 'var(--danger)';
                ['moisture', 'temperature', 'humidity', 'water-level'].forEach(id => 
                    updateSensorCard(null, id, '', 0)
                );
            });
    </script>
</body>
</html>
)rawliteral";

#endif 