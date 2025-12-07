// Bobcat Ignition Controller - AMG Edition
class DemoMode {
    constructor(controller) {
        this.controller = controller;
        this.active = false;
        this.timer = null;
        this.step = 0;
    }

    start() {
        if (this.active) return;
        this.active = true;
        console.log('Starting Demo Mode');

        // Disable backend polling during demo
        stopPolling();

        this.runSequence();
        this.timer = setInterval(() => this.runSequence(), 50);
    }

    stop() {
        this.active = false;
        clearInterval(this.timer);
        this.step = 0; // Reset counter for fresh start
        startPolling(); // Resume normal operation
        console.log('Demo Mode Stopped');
    }

    runSequence() {
        // Time-based sequence (ticks counter)
        this.step++;
        const cycleLength = 2000; // 100 seconds total cycle at 50ms intervals
        const t = this.step % cycleLength;

        // 0-100: Ignition OFF -> ON
        if (t === 10) this.controller.setKeyPositionVisual('on');

        // 100-200: Gauge Sweep (Startup check)
        if (t > 50 && t < 150) {
            const sweep = (t - 50) / 100;
            // Sweep up then down
            const value = sweep < 0.5 ? sweep * 2 : (1 - sweep) * 2;
            this.controller.updateGauges({
                oil_pressure: value * 100, // Max PSI
                engine_temp: 20 + value * 100,
                battery_voltage: 12 + value * 2,
                fuel_level: value * 100,
                hyd_pressure: value * 3000
            });
        }

        // 200-300: Glow Plugs
        if (t === 200) this.controller.setKeyPositionVisual('glow');

        // 350: Start
        if (t === 350) this.controller.setKeyPositionVisual('start');

        // 380: Running (Engine start)
        if (t === 380) {
            this.controller.setKeyPositionVisual('on');
            // Simulate engine running params
            this.controller.updateGauges({
                oil_pressure: 40,
                engine_temp: 60,
                battery_voltage: 13.8,
                fuel_level: 75,
                hyd_pressure: 1500
            });
        }

        // 400-800: Work simulation (Revving)
        if (t > 400 && t < 1800) {
            // Sine wave for RPM-like effect
            const rev = (Math.sin(t * 0.1) + 1) / 2;
            this.controller.updateGauges({
                oil_pressure: 40 + rev * 20,
                engine_temp: 80 + rev * 10,
                battery_voltage: 13.8 + (Math.random() * 0.2),
                fuel_level: 75 - ((t - 400) / 1400) * 5, // Slight fuel usage
                hyd_pressure: 1500 + rev * 1000
            });
        }

        // 1900: Shutdown
        if (t === 1900) {
            this.controller.setKeyPositionVisual('off');
            this.controller.updateGauges({
                oil_pressure: 0,
                engine_temp: 70, // Stays warm
                battery_voltage: 12.5,
                fuel_level: 70,
                hyd_pressure: 0
            });
        }
    }
}

class IgnitionController {
    constructor() {
        this.key = document.getElementById('ignitionKey');
        this.face = document.querySelector('.face');

        // Position label elements
        this.labels = {
            off: document.querySelector('.off-label'),
            on: document.querySelector('.on-label'),
            glow: document.querySelector('.glow-label'),
            start: document.querySelector('.start-label')
        };

        // Gauge elements
        this.pressureNeedle = document.getElementById('pressureNeedle');
        this.tempNeedle = document.getElementById('tempNeedle');
        this.voltageNeedle = document.getElementById('voltageNeedle');
        this.fuelNeedle = document.getElementById('fuelNeedle');
        this.hydNeedle = document.getElementById('hydNeedle');

        this.pressureValue = document.getElementById('pressureValue');
        this.tempValue = document.getElementById('tempValue');
        this.voltageValue = document.getElementById('voltageValue');
        this.fuelValue = document.getElementById('fuelValue');
        this.hydValue = document.getElementById('hydValue');

        // Work lights button
        this.workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
        this.workLightsActive = false;

        // State management
        this.currentState = 'off';
        this.currentAngle = -30; // 11 o'clock
        this.isDragging = false;
        this.startAngle = 0;
        this.isTransitioning = false;

        // Position definitions
        this.positions = {
            off: { angle: -30, label: 'OFF', color: 'off' },
            on: { angle: 0, label: 'ON', color: 'on' },
            glow: { angle: 45, label: 'GLOW', color: 'glow' },
            start: { angle: 90, label: 'START', color: 'start' }
        };

        this.stateOrder = ['off', 'on', 'glow', 'start'];
        this.backendKeyPosition = 0;

        this.demoMode = new DemoMode(this);

        this.init();
    }

    init() {
        this.setupEventListeners();
        this.updateDisplay();
        this.updateKeyPosition();
        this.initializeGauges();

        // Check for demo mode in URL
        const urlParams = new URLSearchParams(window.location.search);
        if (urlParams.has('demo')) {
            this.demoMode.start();
        }
    }

    setupEventListeners() {
        // Mouse/Touch events for Key (unchanged logic)
        this.key.addEventListener('mousedown', this.handleStart.bind(this));
        document.addEventListener('mousemove', this.handleMove.bind(this));
        document.addEventListener('mouseup', this.handleEnd.bind(this));

        this.key.addEventListener('touchstart', this.handleStart.bind(this), { passive: false });
        document.addEventListener('touchmove', this.handleMove.bind(this), { passive: false });
        document.addEventListener('touchend', this.handleEnd.bind(this));

        this.key.addEventListener('contextmenu', (e) => e.preventDefault());
        this.key.addEventListener('dragstart', (e) => e.preventDefault());

        // Buttons
        const emergencyBtn = document.querySelector('.emergency-btn');
        if (emergencyBtn) emergencyBtn.addEventListener('click', () => this.emergencyStop());

        const workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
        if (workLightsBtn) workLightsBtn.addEventListener('click', () => this.toggleWorkLights());

        // Secret Demo Mode Trigger: Long press "ON" label (3 seconds)
        const onLabel = document.querySelector('.on-label');
        if (onLabel) {
            let pressTimer;
            const triggerDemo = () => {
                pressTimer = setTimeout(() => {
                    if (this.demoMode.active) this.demoMode.stop();
                    else this.demoMode.start();
                }, 3000);
            };
            const cancelDemo = () => clearTimeout(pressTimer);

            // Mouse support
            onLabel.addEventListener('mousedown', triggerDemo);
            onLabel.addEventListener('mouseup', cancelDemo);
            onLabel.addEventListener('mouseleave', cancelDemo);

            // Touch support
            onLabel.addEventListener('touchstart', (e) => { e.preventDefault(); triggerDemo(); }, { passive: false });
            onLabel.addEventListener('touchend', cancelDemo);
            onLabel.addEventListener('touchcancel', cancelDemo);
        }
    }

    // ... Drag handling methods (keep existing logic) ...
    handleStart(event) {
        if (this.isTransitioning || this.demoMode.active) return; // Lock if demo active

        event.preventDefault();
        this.isDragging = true;
        this.key.style.cursor = 'grabbing';

        const rect = this.key.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;
        const clientX = event.clientX || (event.touches && event.touches[0].clientX);
        const clientY = event.clientY || (event.touches && event.touches[0].clientY);

        this.startAngle = this.calculateAngle(clientX, clientY, centerX, centerY) - this.currentAngle;
    }

    handleMove(event) {
        if (!this.isDragging || this.isTransitioning) return;
        event.preventDefault();

        const rect = this.key.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;
        const clientX = event.clientX || (event.touches && event.touches[0].clientX);
        const clientY = event.clientY || (event.touches && event.touches[0].clientY);

        const currentMouseAngle = this.calculateAngle(clientX, clientY, centerX, centerY);
        let targetAngle = currentMouseAngle - this.startAngle;

        targetAngle = this.normalizeAngle(targetAngle);
        if (targetAngle > 180) targetAngle = targetAngle - 360;

        const constrainedAngle = this.constrainAngle(targetAngle);

        if (constrainedAngle !== this.currentAngle) {
            this.currentAngle = constrainedAngle;
            this.updateKeyPosition();

            const newState = this.getStateFromAngle(this.currentAngle);
            if (newState !== this.currentState) {
                this.currentState = newState;
                this.updateDisplay();
                this.sendStateChangeToBackend(newState);
            }
        }
    }

    handleEnd(event) {
        if (!this.isDragging) return;
        this.isDragging = false;
        this.key.style.cursor = 'pointer';
        this.snapToPosition();
    }

    calculateAngle(x, y, centerX, centerY) {
        const deltaX = x - centerX;
        const deltaY = y - centerY;
        let angle = Math.atan2(deltaY, deltaX) * (180 / Math.PI);
        angle = angle + 90;
        return this.normalizeAngle(angle);
    }

    normalizeAngle(angle) {
        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;
        return angle;
    }

    constrainAngle(targetAngle) {
        const currentStateIndex = this.stateOrder.indexOf(this.currentState);
        let minAngle = this.positions.off.angle;
        let maxAngle = this.positions[this.stateOrder[Math.min(currentStateIndex + 1, this.stateOrder.length - 1)]].angle;
        if (targetAngle < this.currentAngle) {
            minAngle = this.positions.off.angle;
            maxAngle = this.currentAngle;
        }
        return Math.max(minAngle, Math.min(maxAngle, targetAngle));
    }

    getStateFromAngle(angle) {
        if (angle >= -30 && angle < -15) return 'off';
        if (angle >= -15 && angle < 22.5) return 'on';
        if (angle >= 22.5 && angle < 67.5) return 'glow';
        if (angle >= 67.5) return 'start';
        return 'off';
    }

    snapToPosition() {
        this.isTransitioning = true;
        let closestState = 'off';
        let closestDistance = Infinity;

        for (const [state, position] of Object.entries(this.positions)) {
            const distance = Math.abs(this.currentAngle - position.angle);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestState = state;
            }
        }

        this.currentAngle = this.positions[closestState].angle;
        this.currentState = closestState;

        this.key.classList.add('transitioning');
        this.updateKeyPosition();
        this.updateDisplay();

        if (closestState === 'start') {
            setTimeout(() => this.springBackFromStart(), 300);
        } else {
            setTimeout(() => {
                this.key.classList.remove('transitioning');
                this.isTransitioning = false;
            }, 200);
        }
    }

    springBackFromStart() {
        this.currentAngle = this.positions.on.angle;
        this.currentState = 'on';
        this.updateKeyPosition();
        this.updateDisplay();
        this.sendCommand('key_start_hold', { held: false });
        setTimeout(() => {
            this.key.classList.remove('transitioning');
            this.isTransitioning = false;
        }, 200);
    }

    updateKeyPosition() {
        this.key.style.transform = `rotate(${this.currentAngle}deg)`;
    }

    setKeyPositionVisual(state) {
        if (this.positions[state]) {
            this.currentState = state;
            this.currentAngle = this.positions[state].angle;
            this.updateKeyPosition();
            this.updateDisplay();
        }
    }

    updateDisplay() {
        Object.values(this.labels).forEach(label => {
            if (label) label.classList.remove('active');
        });
        if (this.labels[this.currentState]) {
            this.labels[this.currentState].classList.add('active');
        }

        this.face.classList.remove('on-active', 'glow-active', 'start-active');
        if (this.currentState === 'on') this.face.classList.add('on-active');
        else if (this.currentState === 'glow') this.face.classList.add('glow-active');
        else if (this.currentState === 'start') this.face.classList.add('start-active');

        document.title = `Bobcat Ignition - ${this.positions[this.currentState].label}`;
    }

    sendStateChangeToBackend(state) {
        if (this.demoMode.active) return;
        const stateToPosition = { 'off': 0, 'on': 1, 'glow': 2, 'start': 3 };
        const position = stateToPosition[state];
        if (position === 3) this.sendCommand('key_start_hold', { held: true });
        else this.sendCommand('key_position', { position: position });
    }

    sendCommand(action, data = {}) {
        if (this.demoMode.active) return;

        fetch('/control', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ action: action, ...data })
        }).catch(error => console.error('Error sending command:', error));
    }

    emergencyStop() {
        console.log('EMERGENCY STOP ACTIVATED!');
        this.sendCommand('emergency_stop');
    }

    toggleWorkLights() {
        this.workLightsActive = !this.workLightsActive;
        const workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
        if (workLightsBtn) {
            if (this.workLightsActive) workLightsBtn.classList.add('active');
            else workLightsBtn.classList.remove('active');
        }
        this.sendCommand('lights');
    }

    updateFromBackend(status) {
        if (this.demoMode.active) return;

        const backendKeyPosition = status.key_position || 0;
        const positionToState = ['off', 'on', 'glow', 'start'];
        const backendState = positionToState[backendKeyPosition];

        if (!this.isDragging && backendState !== this.currentState) {
            this.currentState = backendState;
            this.currentAngle = this.positions[backendState].angle;
            this.updateKeyPosition();
            this.updateDisplay();
        }

        this.backendKeyPosition = backendKeyPosition;
        this.updateGauges(status);

        // Sync Work Lights
        if (status.lights_on !== undefined) {
            this.workLightsActive = status.lights_on;
            const workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
            if (workLightsBtn) {
                if (this.workLightsActive) workLightsBtn.classList.add('active');
                else workLightsBtn.classList.remove('active');
            }
        }
    }

    initializeGauges() {
        const startAngle = 135; // 0 value = 135deg
        [this.pressureNeedle, this.tempNeedle, this.voltageNeedle, this.fuelNeedle, this.hydNeedle].forEach(needle => {
            if (needle) needle.style.transform = `rotate(${startAngle}deg)`;
        });
    }

    updateGauges(status) {
        let pressure, temperature, voltage, fuel, hydraulic;

        if (status) {
            // Backend sends: pressure, engine_temp, battery_voltage, fuel_level, hyd_pressure
            // Handle both possible property names for compatibility
            pressure = status.pressure ?? status.oil_pressure ?? 0;
            temperature = status.engine_temp ?? status.temperature ?? 0;
            voltage = status.battery_voltage ?? status.voltage ?? 0;
            fuel = status.fuel_level ?? status.fuel ?? 0;
            hydraulic = status.hyd_pressure ?? status.hydraulic ?? 0;
        } else {
            // Fallback
            pressure = 0; temperature = 20; voltage = 0; fuel = 0; hydraulic = 0;
        }

        // Map to 270 degree range (135 -> 405)
        // Range Logic:
        // Pressure: 0-100 PSI (0-7 BAR). 0 -> 135deg, 100 -> 405deg.
        const pAngle = 135 + Math.min(Math.max(pressure / 100, 0), 1) * 270;

        // Temperature: 40C - 120C. 
        // 40 -> 135deg, 120 -> 405deg.
        const tPct = Math.min(Math.max((temperature - 40) / 80, 0), 1);
        const tAngle = 135 + tPct * 270;

        // Voltage: 8V - 16V
        const vPct = Math.min(Math.max((voltage - 8) / 8, 0), 1);
        const vAngle = 135 + vPct * 270;

        // Fuel: 0-100%
        const fAngle = 135 + Math.min(Math.max(fuel / 100, 0), 1) * 270;

        // Hydraulic: 0-3000 PSI ?
        const hAngle = 135 + Math.min(Math.max(hydraulic / 3000, 0), 1) * 270;

        if (this.pressureNeedle) this.pressureNeedle.style.transform = `rotate(${pAngle}deg)`;
        if (this.tempNeedle) this.tempNeedle.style.transform = `rotate(${tAngle}deg)`;
        if (this.voltageNeedle) this.voltageNeedle.style.transform = `rotate(${vAngle}deg)`;
        if (this.fuelNeedle) this.fuelNeedle.style.transform = `rotate(${fAngle}deg)`;
        if (this.hydNeedle) this.hydNeedle.style.transform = `rotate(${hAngle}deg)`;

        // Update Text Values
        if (this.pressureValue) this.pressureValue.textContent = `${(pressure * 0.0689).toFixed(1)} BAR`;
        if (this.tempValue) this.tempValue.textContent = `${Math.round(temperature)}°C`;
        if (this.voltageValue) this.voltageValue.textContent = `${voltage.toFixed(1)}V`;
        if (this.fuelValue) this.fuelValue.textContent = `${Math.round(fuel)}%`;
        if (this.hydValue) this.hydValue.textContent = `${Math.round(hydraulic)} PSI`;
    }
}

// ... Main polling code ...
let pollingInterval;
let ignitionController;
const POLLING_INTERVAL = 1000;

document.addEventListener('DOMContentLoaded', function () {
    console.log('Bobcat Ignition Controller Initializing...');
    ignitionController = new IgnitionController();
    startPolling();
    // No initial status update call needed, interval handles it? 
    // Better to call one immediately
    updateStatus();
});

function updateStatus() {
    if (ignitionController && ignitionController.demoMode.active) return;

    fetch('/status')
        .then(response => response.json())
        .then(data => {
            updateDashboard(data);
        })
        .catch(error => {
            // console.error('Error fetching status:', error); 
        });
}

function updateDashboard(status) {
    if (ignitionController) {
        ignitionController.updateFromBackend(status);
    }
    // ... existing status update funcs ...
    updateCountdownDisplay(status);
    updateWarningLights(status);
    updateMasterStatus(status);
}

// Keep helper functions ...
function updateCountdownDisplay(status) {
    const countdownElement = document.querySelector('#glow-timer');
    if (countdownElement) {
        if ((status.glow_active || status.glow_plugs_on) && status.countdown && status.countdown > 0) {
            countdownElement.textContent = `${status.countdown}s`;
            countdownElement.style.display = 'block';
        } else if (status.glow_active || status.glow_plugs_on) {
            countdownElement.textContent = 'Ready';
            countdownElement.style.display = 'block';
        } else {
            countdownElement.style.display = 'none';
        }
    }
}

function updateWarningLights(status) {
    // ... same as before ...
    const oilLight = document.querySelector('#oil-warning .light');
    if (oilLight) {
        if (status.low_oil_pressure) oilLight.classList.add('active');
        else oilLight.classList.remove('active');
    }
    const tempLight = document.querySelector('#temp-warning .light');
    if (tempLight) {
        if (status.high_temperature) tempLight.classList.add('active');
        else tempLight.classList.remove('active');
    }
    const batteryLight = document.querySelector('#battery-warning .light');
    if (batteryLight) {
        if (status.low_battery) batteryLight.classList.add('active');
        else batteryLight.classList.remove('active');
    }
    const chargeLight = document.querySelector('#charge-indicator .light');
    if (chargeLight) {
        if (status.charging || (status.battery_voltage && status.battery_voltage > 13.0)) chargeLight.classList.add('active');
        else chargeLight.classList.remove('active');
    }
    const engineLight = document.querySelector('#engine-run .light');
    if (engineLight) {
        if (status.state === 'RUNNING') engineLight.classList.add('active');
        else engineLight.classList.remove('active');
    }
    const fuelLight = document.querySelector('#fuel-warning .light');
    if (fuelLight) {
        if (status.fuel_level && status.fuel_level < 25) fuelLight.classList.add('active');
        else fuelLight.classList.remove('active');
    }
}

function updateMasterStatus(status) {
    const masterStatus = document.querySelector('.master-status');
    if (masterStatus) {
        let statusText = 'SYSTEM';
        let statusClass = 'master-status';

        // ... Logic same as before, simplified switch ...
        switch (status.state) {
            case 'OFF': statusText = 'OFFLINE'; break;
            case 'ON':
                statusText = 'STANDBY';
                statusClass += ' status-ready';
                break;
            case 'GLOW_HEATING':
                statusText = 'HEATING';
                statusClass += ' status-glow-heating';
                break;
            case 'STARTING':
                statusText = 'STARTING';
                statusClass += ' status-starting';
                break;
            case 'RUNNING':
                statusText = 'RUNNING';
                statusClass += ' status-running';
                break;
            case 'EMERGENCY_STOP':
                statusText = 'EMERGENCY';
                statusClass += ' status-alert';
                break;
            default: statusText = 'STANDBY';
        }
        masterStatus.textContent = statusText;
        masterStatus.className = statusClass;
    }
}

function startPolling() {
    if (pollingInterval) clearInterval(pollingInterval);
    pollingInterval = setInterval(updateStatus, POLLING_INTERVAL);
}

function stopPolling() {
    if (pollingInterval) {
        clearInterval(pollingInterval);
        pollingInterval = null;
    }
}

window.addEventListener('beforeunload', stopPolling);

// Prevent zoom on double-tap
let lastTouchEnd = 0;
document.addEventListener('touchend', (event) => {
    const now = (new Date()).getTime();
    if (now - lastTouchEnd <= 300) event.preventDefault();
    lastTouchEnd = now;
}, false);

