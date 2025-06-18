document.addEventListener('DOMContentLoaded', () => {
    const startBtn = document.getElementById('start-btn');
    const stopBtn = document.getElementById('stop-btn');
    const statusMessage = document.getElementById('status-message');
    const tempValue = document.getElementById('temp-value');
    const pressureValue = document.getElementById('pressure-value');
    const batteryValue = document.getElementById('battery-value');

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

    function updateStatus() {
        // Fetch the latest status from the ESP32
        fetch('/status')
            .then(response => response.json())
            .then(data => {
                statusMessage.textContent = data.status;
                tempValue.innerHTML = `${data.temperature} &deg;C`;
                pressureValue.textContent = `${data.pressure} PSI`;
                batteryValue.textContent = `${data.battery} V`;
            });
    }

    // Update the status every 2 seconds
    setInterval(updateStatus, 2000);
});
