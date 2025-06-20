document.addEventListener('DOMContentLoaded', () => {
    const powerOnBtn = document.getElementById('power-on-btn');
    const powerOffBtn = document.getElementById('power-off-btn');
    const startBtn = document.getElementById('start-btn');
    const overrideBtn = document.getElementById('override-btn');
    const lightsBtn = document.getElementById('lights-btn');
    const statusMessage = document.getElementById('status-message');
    const tempValue = document.getElementById('temp-value');
    const pressureValue = document.getElementById('pressure-value');
    const batteryValue = document.getElementById('battery-value');

    powerOnBtn.addEventListener('click', () => {
        fetch('/power_on')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatus();
            });
    });

    powerOffBtn.addEventListener('click', () => {
        fetch('/power_off')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatus();
            });
    });

    startBtn.addEventListener('click', () => {
        // Send a request to the ESP32 to start the engine
        fetch('/start')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatus();
            });
    });

    stopBtn.addEventListener('click', () => {
        // Send a request to the ESP32 to stop the engine
        fetch('/stop')
            .then(response => response.text())
            .then(data => {
                console.log(data);
                updateStatus();
            });
    });

    overrideBtn.addEventListener('click', () => {
        if (confirm("Are you sure you want to override all safety checks and start the engine? This is a dangerous action.")) {
            fetch('/override')
                .then(response => response.text())
                .then(data => {
                    console.log(data);
                    updateStatus();
                });
        }
    });

    lightsBtn.addEventListener('click', () => {
        fetch('/toggle_lights')
            .then(response => response.text())
            .then(data => console.log(data));
    });

    function updateStatus() {
        // Fetch the latest status from the ESP32
        fetch('/status')
            .then(response => response.json())
            .then(data => {
                statusMessage.textContent = data.status;
                tempValue.innerHTML = `${data.temperature} &deg;C`;
                pressureValue.textContent = `${data.pressure} PSI`;
                batteryValue.textContent = `${data.battery} V`;

                // Add or remove the alert class based on the status
                if (data.status === 'LOW_OIL_PRESSURE' || data.status === 'HIGH_TEMPERATURE') {
                    statusMessage.classList.add('status-alert');
                } else {
                    statusMessage.classList.remove('status-alert');
                }
            });
    }

    // Update the status every 2 seconds
    setInterval(updateStatus, 2000);
});
