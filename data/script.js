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
        stopPolling();
        this.runSequence();
        this.timer = setInterval(() => this.runSequence(), 50);
    }

    stop() {
        this.active = false;
        clearInterval(this.timer);
        this.step = 0;
        startPolling();
        console.log('Demo Mode Stopped');
        // Reset gauges
        this.controller.updateGauges(null);
    }

    runSequence() {
        this.step++;
        const cycleLength = 2000;
        const t = this.step % cycleLength;

        // 0-100: Ignition OFF -> ON
        if (t === 10) this.controller.setKeyPositionVisual('on');

        // 100-200: Gauge Sweep (Startup check)
        if (t > 50 && t < 150) {
            const sweep = (t - 50) / 100;
            const value = sweep < 0.5 ? sweep * 2 : (1 - sweep) * 2;
            this.controller.updateGauges({
                oil_pressure: value * 100,
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

        // 380: Running
        if (t === 380) {
            this.controller.setKeyPositionVisual('on');
            this.controller.updateGauges({
                oil_pressure: 40,
                engine_temp: 60,
                battery_voltage: 13.8,
                fuel_level: 75,
                hyd_pressure: 1500
            });
        }

        // 400-800: Work simulation
        if (t > 400 && t < 1800) {
            const rev = (Math.sin(t * 0.1) + 1) / 2;
            this.controller.updateGauges({
                oil_pressure: 40 + rev * 20,
                engine_temp: 80 + rev * 10,
                battery_voltage: 13.8 + (Math.random() * 0.2),
                fuel_level: 75 - ((t - 400) / 1400) * 5,
                hyd_pressure: 1500 + rev * 1000
            });
        }

        // 1900: Shutdown
        if (t === 1900) {
            this.controller.setKeyPositionVisual('off');
            this.controller.updateGauges(null);
        }
    }
}

class IgnitionController {
    constructor() {
        this.key = document.getElementById('ignitionKey');
        this.face = document.querySelector('.face');

        this.labels = {
            off: document.querySelector('.off-label'),
            on: document.querySelector('.on-label'),
            glow: document.querySelector('.glow-label'),
            start: document.querySelector('.start-label')
        };

        this.tempNeedle = document.getElementById('tempNeedle'); // Bar
        this.fuelNeedle = document.getElementById('fuelNeedle'); // Bar
        this.pressureNeedle = document.getElementById('pressureNeedle'); // Bar
        this.voltageNeedle = document.getElementById('voltageNeedle'); // Bar
        this.hydNeedle = document.getElementById('hydNeedle');   // Ring (SVG Circle)

        // Text Values
        this.pressureValue = document.getElementById('pressureValue');
        this.tempValue = document.getElementById('tempValue');
        this.voltageValue = document.getElementById('voltageValue');
        this.fuelValue = document.getElementById('fuelValue');
        this.hydValue = document.getElementById('hydValue');

        // Audio
        this.sounds = {
            click: document.getElementById('clickSound'),
            glow: document.getElementById('glowSound'),
            start: document.getElementById('startSound')
        };

        // State
        this.currentState = 'off';
        this.currentAngle = -30;
        this.isDragging = false;
        this.startAngle = 0;
        this.isTransitioning = false;

        this.positions = {
            off: { angle: -30, label: 'OFF', color: 'off' },
            on: { angle: 0, label: 'ON', color: 'on' },
            glow: { angle: 45, label: 'GLOW', color: 'glow' },
            start: { angle: 90, label: 'START', color: 'start' }
        };

        this.stateOrder = ['off', 'on', 'glow', 'start'];

        this.demoMode = new DemoMode(this);
        this.init();
    }

    init() {
        this.setupEventListeners();
        this.updateDisplay();
        this.updateKeyPosition();

        // Check for demo
        const urlParams = new URLSearchParams(window.location.search);
        if (urlParams.has('demo')) {
            this.demoMode.start();
        }
    }

    setupEventListeners() {
        // Key Interaction
        const startHandler = this.handleStart.bind(this);
        const moveHandler = this.handleMove.bind(this);
        const endHandler = this.handleEnd.bind(this);

        this.key.addEventListener('mousedown', startHandler);
        document.addEventListener('mousemove', moveHandler);
        document.addEventListener('mouseup', endHandler);

        this.key.addEventListener('touchstart', startHandler, { passive: false });
        document.addEventListener('touchmove', moveHandler, { passive: false });
        document.addEventListener('touchend', endHandler);

        // Buttons
        const stopBtn = document.querySelector('[data-action="emergency_stop"]');
        if (stopBtn) stopBtn.addEventListener('click', () => this.emergencyStop());

        const lightBtn = document.querySelector('[data-action="toggle_lights"]');
        if (lightBtn) lightBtn.addEventListener('click', () => this.toggleWorkLights());

        // Secret Demo Trigger (Click Logo)
        const brand = document.querySelector('.brand-logo');
        if (brand) {
            brand.addEventListener('click', () => {
                this.demoMode.active ? this.demoMode.stop() : this.demoMode.start();
            });
        }
    }

    handleStart(event) {
        if (this.isTransitioning || this.demoMode.active) return;
        event.preventDefault();
        this.isDragging = true;
        this.key.style.cursor = 'grabbing';

        const rect = this.key.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;
        const clientX = event.clientX || (event.touches && event.touches[0].clientX);
        const clientY = event.clientY || (event.touches && event.touches[0].clientY);

        this.startAngle = this.calculateAngle(clientX, clientY, centerX, centerY) - this.currentAngle;
        this.playSound('click');
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
                this.playSound('click');
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

        this.updateKeyPosition();
        this.updateDisplay();

        if (closestState === 'start') {
            this.playSound('start');
            setTimeout(() => this.springBackFromStart(), 300);
        } else {
            if (closestState === 'glow') this.playSound('glow');
            setTimeout(() => {
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
        Object.values(this.labels).forEach(label => label.classList.remove('active'));
        if (this.labels[this.currentState]) {
            this.labels[this.currentState].classList.add('active');
        }

        // Glow Countdown logic handled by updateCountdownDisplay
    }

    playSound(type) {
        if (this.sounds[type]) {
            this.sounds[type].currentTime = 0;
            this.sounds[type].play().catch(e => console.log('Audio error:', e));
        }
    }

    sendStateChangeToBackend(state) {
        if (this.demoMode.active) return;
        const stateToPosition = { 'off': 0, 'on': 1, 'glow': 2, 'start': 3 };
        const position = stateToPosition[state];
        // Log or fetch
        console.log(`State change: ${state}`);
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
        console.log('EMERGENCY STOP');
        this.sendCommand('emergency_stop');
    }

    toggleWorkLights() {
        const btn = document.querySelector('[data-action="toggle_lights"]');
        btn.classList.toggle('active'); // Visual only until backend confirms
        this.sendCommand('lights');
    }

    // --- Backend Sync ---
    updateFromBackend(status) {
        if (this.demoMode.active) return;

        // Key Sync
        const backendKeyPosition = status.key_position || 0;
        const positionToState = ['off', 'on', 'glow', 'start'];
        const backendState = positionToState[backendKeyPosition];

        if (!this.isDragging && backendState !== this.currentState) {
            this.setKeyPositionVisual(backendState);
        }

        this.updateGauges(status);
        updateWarningLights(status);
        // Toggle light button state
        const lightBtn = document.querySelector('[data-action="toggle_lights"]');
        if (lightBtn && status.lights_on !== undefined) {
            if (status.lights_on) lightBtn.classList.add('active');
            else lightBtn.classList.remove('active');
        }
    }

    updateGauges(status) {
        // console.log('DEBUG: updateGauges called with', status);
        let pressure, temperature, voltage, fuel, hydraulic;

        if (status) {
            pressure = status.pressure ?? status.oil_pressure ?? 0;
            temperature = status.engine_temp ?? status.temperature ?? 20; // Default 20C
            voltage = status.battery_voltage ?? status.voltage ?? 0;
            fuel = status.fuel_level ?? status.fuel ?? 0;
            hydraulic = status.hyd_pressure ?? status.hydraulic ?? 0;
        } else {
            pressure = 0; temperature = 20; voltage = 0; fuel = 0; hydraulic = 0;
        }

        // 1. Hydraulic (Hero Ring) - Stroke Dashoffset
        // 0-3000 PSI. 420 is dashed array length. 
        const hPct = Math.min(Math.max(hydraulic / 3000, 0), 1);
        if (this.hydNeedle) {
            const offset = 420 - (hPct * 420);
            this.hydNeedle.style.strokeDashoffset = offset;
        }
        if (this.hydValue) this.hydValue.textContent = Math.round(hydraulic);

        // 2. Temp (Bar) - 40C to 120C
        const tPct = Math.min(Math.max((temperature - 40) / 80, 0), 1);
        if (this.tempNeedle) {
            this.tempNeedle.style.height = `${tPct * 100}%`;
            this.tempNeedle.style.background = tPct > 0.9 ? 'var(--amg-red)' : 'linear-gradient(to top, var(--amg-green), #fff)';
        }
        if (this.tempValue) this.tempValue.textContent = `${Math.round(temperature)}°C`;

        // 3. Fuel (Bar) - 0-100%
        const fPct = Math.min(Math.max(fuel / 100, 0), 1);
        if (this.fuelNeedle) {
            this.fuelNeedle.style.height = `${fPct * 100}%`;
            this.fuelNeedle.style.background = fPct < 0.1 ? 'var(--amg-red)' : 'linear-gradient(to top, var(--amg-green), #fff)';
        }
        if (this.fuelValue) this.fuelValue.textContent = `${Math.round(fuel)}%`;

        // 4. Pressure (Info Box) - 0-100 PSI
        if (this.pressureValue) this.pressureValue.textContent = `${Math.round(pressure)}`;

        // 5. Voltage (Info Box) - 10-15V
        if (this.voltageValue) this.voltageValue.textContent = `${voltage.toFixed(1)}`;
    }
}

// Global & Polling
let pollingInterval;
let ignitionController;

document.addEventListener('DOMContentLoaded', () => {
    ignitionController = new IgnitionController();
    startPolling();
});

function startPolling() {
    if (pollingInterval) clearInterval(pollingInterval);
    pollingInterval = setInterval(updateStatus, 1000);
}

function stopPolling() {
    if (pollingInterval) clearInterval(pollingInterval);
}

function updateStatus() {
    if (ignitionController && ignitionController.demoMode.active) return;
    fetch('/status')
        .then(r => r.json())
        .then(data => {
            if (ignitionController) ignitionController.updateFromBackend(data);
            updateMasterStatus(data);
        })
        .catch(e => { });
}

function updateWarningLights(status) {
    if (!status) return;
    const warnings = {
        'oil-warning': status.low_oil_pressure,
        'temp-warning': status.high_temperature,
        'battery-warning': status.low_battery
    };

    for (const [id, active] of Object.entries(warnings)) {
        const el = document.getElementById(id);
        if (el) {
            if (active) el.classList.add('active');
            else el.classList.remove('active');
        }
    }
}

function updateMasterStatus(status) {
    const el = document.getElementById('master-status');
    if (!el) return;
    const s = status ? status.state : 'OFF';
    el.textContent = s;
    if (s === 'RUNNING') el.style.color = 'var(--amg-green)';
    else if (s === 'EMERGENCY_STOP') el.style.color = 'var(--amg-red)';
    else el.style.color = '#fff';
}
