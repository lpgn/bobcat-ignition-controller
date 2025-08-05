// Bobcat Ignition Controller - Using Original Working Design with Backend API
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
        
        this.pressureValue = document.getElementById('pressureValue');
        this.tempValue = document.getElementById('tempValue');
        this.voltageValue = document.getElementById('voltageValue');
        this.fuelValue = document.getElementById('fuelValue');
        
        this.pressureGauge = document.querySelector('.pressure-gauge');
        this.tempGauge = document.querySelector('.temp-gauge');
        this.voltageGauge = document.querySelector('.voltage-gauge');
        this.fuelGauge = document.querySelector('.fuel-gauge');
        
        // Work lights button
        this.workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
        this.workLightsActive = false;
        
        // State management - using your original working logic
        this.currentState = 'off';
        this.currentAngle = -30; // Start at OFF position (11 o'clock)
        this.isDragging = false;
        this.startAngle = 0;
        this.isTransitioning = false;
        
        // Position definitions (clock-based angles) - your original working design
        this.positions = {
            off: { angle: -30, label: 'OFF', color: 'off' },      // 11 o'clock
            on: { angle: 0, label: 'ON', color: 'on' },           // 12 o'clock
            glow: { angle: 45, label: 'GLOW', color: 'glow' },    // 1:30 o'clock
            start: { angle: 90, label: 'START', color: 'start' }  // 3 o'clock
        };
        
        // State progression order
        this.stateOrder = ['off', 'on', 'glow', 'start'];
        
        // Backend integration
        this.backendKeyPosition = 0;
        
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.updateDisplay();
        this.updateKeyPosition();
        // Initialize the lighting effect (default to dark/off state)
        this.updateCenterLighting();
    }
    
    setupEventListeners() {
        // Mouse events
        this.key.addEventListener('mousedown', this.handleStart.bind(this));
        document.addEventListener('mousemove', this.handleMove.bind(this));
        document.addEventListener('mouseup', this.handleEnd.bind(this));
        
        // Touch events for mobile
        this.key.addEventListener('touchstart', this.handleStart.bind(this), { passive: false });
        document.addEventListener('touchmove', this.handleMove.bind(this), { passive: false });
        document.addEventListener('touchend', this.handleEnd.bind(this));
        
        // Prevent context menu on right click
        this.key.addEventListener('contextmenu', (e) => e.preventDefault());
        
        // Prevent default drag behavior
        this.key.addEventListener('dragstart', (e) => e.preventDefault());
        
        // Emergency stop button
        const emergencyBtn = document.querySelector('.emergency-btn');
        if (emergencyBtn) {
            emergencyBtn.addEventListener('click', () => this.emergencyStop());
        }
        
        // Work lights button
        const workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
        if (workLightsBtn) {
            workLightsBtn.addEventListener('click', () => this.toggleWorkLights());
        }
    }
    
    handleStart(event) {
        if (this.isTransitioning) return;
        
        event.preventDefault();
        this.isDragging = true;
        this.key.style.cursor = 'grabbing';
        
        // Get the starting angle relative to the center
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
        
        // Normalize angle to 0-360 range
        targetAngle = this.normalizeAngle(targetAngle);
        
        // Convert to our coordinate system (-30 to 90)
        if (targetAngle > 180) {
            targetAngle = targetAngle - 360;
        }
        
        // Constrain movement based on current state and clockwise-only rule
        const constrainedAngle = this.constrainAngle(targetAngle);
        
        if (constrainedAngle !== this.currentAngle) {
            this.currentAngle = constrainedAngle;
            this.updateKeyPosition();
            
            // Update state based on angle - IMMEDIATE RESPONSE like your original
            const newState = this.getStateFromAngle(this.currentAngle);
            if (newState !== this.currentState) {
                this.currentState = newState;
                this.updateDisplay();
                
                // Send command to backend immediately (API integration)
                this.sendStateChangeToBackend(newState);
            }
        }
    }
    
    handleEnd(event) {
        if (!this.isDragging) return;
        
        this.isDragging = false;
        this.key.style.cursor = 'pointer';
        
        // Snap to nearest valid position
        this.snapToPosition();
    }
    
    calculateAngle(x, y, centerX, centerY) {
        const deltaX = x - centerX;
        const deltaY = y - centerY;
        let angle = Math.atan2(deltaY, deltaX) * (180 / Math.PI);
        
        // Convert to our coordinate system where 0° is pointing up
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
        
        // Allow movement within current range and forward progression
        let minAngle = this.positions.off.angle;
        let maxAngle = this.positions[this.stateOrder[Math.min(currentStateIndex + 1, this.stateOrder.length - 1)]].angle;
        
        // Allow backward movement to previous states
        if (targetAngle < this.currentAngle) {
            minAngle = this.positions.off.angle;
            maxAngle = this.currentAngle;
        }
        
        return Math.max(minAngle, Math.min(maxAngle, targetAngle));
    }
    
    getStateFromAngle(angle) {
        // Find the appropriate state based on angle ranges - your original logic
        if (angle >= -30 && angle < -15) return 'off';    // 11 o'clock area
        if (angle >= -15 && angle < 22.5) return 'on';    // 12 o'clock area
        if (angle >= 22.5 && angle < 67.5) return 'glow'; // 1:30 o'clock area
        if (angle >= 67.5) return 'start';                // 3 o'clock area
        return 'off';
    }
    
    snapToPosition() {
        this.isTransitioning = true;
        
        // Find the closest valid position
        let closestState = 'off';
        let closestDistance = Infinity;
        
        for (const [state, position] of Object.entries(this.positions)) {
            const distance = Math.abs(this.currentAngle - position.angle);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestState = state;
            }
        }
        
        // Snap to the closest position
        this.currentAngle = this.positions[closestState].angle;
        this.currentState = closestState;
        
        // Enable transition animation
        this.key.classList.add('transitioning');
        this.updateKeyPosition();
        this.updateDisplay();
        
        // Handle START position spring-back - your original behavior
        if (closestState === 'start') {
            setTimeout(() => {
                this.springBackFromStart();
            }, 300);
        } else {
            // Remove transition class after animation
            setTimeout(() => {
                this.key.classList.remove('transitioning');
                this.isTransitioning = false;
            }, 200);
        }
    }
    
    springBackFromStart() {
        // Auto-return from START to ON - your original behavior
        this.currentAngle = this.positions.on.angle;
        this.currentState = 'on';
        this.updateKeyPosition();
        this.updateDisplay();
        
        // Send backend command for spring-back
        this.sendCommand('key_start_hold', { held: false });
        
        setTimeout(() => {
            this.key.classList.remove('transitioning');
            this.isTransitioning = false;
        }, 200);
    }
    
    updateKeyPosition() {
        this.key.style.transform = `rotate(${this.currentAngle}deg)`;
    }
    
    updateDisplay() {
        // Remove active class from all labels
        Object.values(this.labels).forEach(label => {
            if (label) label.classList.remove('active');
        });
        
        // Add active class to current position label
        if (this.labels[this.currentState]) {
            this.labels[this.currentState].classList.add('active');
        }
        
        // Control face glow effects for different positions
        this.face.classList.remove('on-active', 'glow-active', 'start-active');
        
        if (this.currentState === 'on') {
            this.face.classList.add('on-active');
        } else if (this.currentState === 'glow') {
            this.face.classList.add('glow-active');
        } else if (this.currentState === 'start') {
            this.face.classList.add('start-active');
        }
        
        // Update gauges based on ignition state - your original behavior
        this.updateGauges();
        
        // Update page title
        const position = this.positions[this.currentState];
        document.title = `Bobcat Ignition - ${position.label}`;
    }
    
    // API Integration Methods
    sendStateChangeToBackend(state) {
        const stateToPosition = { 'off': 0, 'on': 1, 'glow': 2, 'start': 3 };
        const position = stateToPosition[state];
        
        if (position === 3) { // START
            this.sendCommand('key_start_hold', { held: true });
        } else {
            this.sendCommand('key_position', { position: position });
        }
    }
    
    sendCommand(action, data = {}) {
        const payload = { action: action, ...data };
        
        fetch('/control', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(payload)
        })
        .then(response => response.json())
        .then(data => {
            if (!data.success) {
                console.error('Command failed:', data.message);
            }
        })
        .catch(error => {
            console.error('Error sending command:', error);
        });
    }
    
    emergencyStop() {
        console.log('EMERGENCY STOP ACTIVATED!');
        this.sendCommand('emergency_stop');
    }
    
    toggleWorkLights() {
        console.log('WORK LIGHTS TOGGLE');
        this.workLightsActive = !this.workLightsActive;
        
        // Update visual state immediately for responsive feel
        this.updateWorkLightsVisual();
        
        // Send command to backend
        this.sendCommand('lights');
    }

    updateWorkLightsVisual() {
        const workLightsBtn = document.querySelector('[data-action="toggle_lights"]');
        if (workLightsBtn) {
            if (this.workLightsActive) {
                workLightsBtn.classList.add('active');
            } else {
                workLightsBtn.classList.remove('active');
            }
        }
        
        // Update center lighting effect
        this.updateCenterLighting();
    }

    updateCenterLighting() {
        const center = document.querySelector('.bezel');
        const lightButton = document.querySelector('[data-action="toggle_lights"]');
        
        if (center && lightButton) {
            if (this.workLightsActive) {
                // Darken the main circle (easier on eyes at night)
                center.style.setProperty('--center-brightness', '0.6');
                center.style.setProperty('--center-shadow', '0 0 40px rgba(0, 0, 0, 0.7)');
                
                // Light up only the bevel (border) around the light button - subtle glow
                lightButton.style.setProperty('--button-border-glow', '0 0 8px rgba(255, 255, 100, 0.6)');
                lightButton.style.setProperty('--button-border-color', '#ffdd00');
            } else {
                // Normal lighting when lights are off
                center.style.setProperty('--center-brightness', '1.0');
                center.style.setProperty('--center-shadow', '0 0 20px rgba(0, 0, 0, 0.4)');
                
                // Remove glow from light button border
                lightButton.style.setProperty('--button-border-glow', 'none');
                lightButton.style.setProperty('--button-border-color', '#555');
            }
        }
    }
    
    // Backend sync method
    updateFromBackend(status) {
        const backendKeyPosition = status.key_position || 0;
        const positionToState = ['off', 'on', 'glow', 'start'];
        const backendState = positionToState[backendKeyPosition];
        
        // Only update if not currently dragging
        if (!this.isDragging && backendState !== this.currentState) {
            this.currentState = backendState;
            this.currentAngle = this.positions[backendState].angle;
            this.updateKeyPosition();
            this.updateDisplay();
        }
        
        this.backendKeyPosition = backendKeyPosition;
        this.updateGauges(status);
    }
    
    updateGauges(status) {
        // Use backend sensor data or simulate based on state
        let pressure, temperature, voltage, fuel;
        
        if (status) {
            // Use real backend data
            pressure = (status.oil_pressure || 0) * 0.0689476; // Convert PSI to BAR
            temperature = status.engine_temp || 20;
            voltage = status.battery_voltage || 0;
            fuel = status.fuel_level || 75;
        } else {
            // Simulate realistic values based on ignition state - your original logic
            switch (this.currentState) {
                case 'off':
                    pressure = 0;
                    temperature = 20;
                    voltage = 0;
                    fuel = 75 + Math.random() * 5;
                    break;
                case 'on':
                    pressure = 1 + Math.random() * 0.3;
                    temperature = 25 + Math.random() * 5;
                    voltage = 12 + Math.random() * 0.5;
                    fuel = 75 + Math.random() * 5;
                    break;
                case 'glow':
                    pressure = 1.3 + Math.random() * 0.4;
                    temperature = 50 + Math.random() * 10;
                    voltage = 11.5 + Math.random() * 0.5;
                    fuel = 75 + Math.random() * 5;
                    break;
                case 'start':
                    pressure = 2.4 + Math.random() * 0.7;
                    temperature = 60 + Math.random() * 10;
                    voltage = 10 + Math.random() * 1;
                    fuel = 74 + Math.random() * 5;
                    break;
            }
        }
        
        // Activate gauges based on state
        const gaugesActive = this.currentState !== 'off';
        
        [this.pressureGauge, this.tempGauge, this.voltageGauge, this.fuelGauge].forEach(gauge => {
            if (gauge) {
                if (gaugesActive) {
                    gauge.classList.add('active');
                } else {
                    gauge.classList.remove('active');
                }
            }
        });
        
        // Update needle positions (0-180 degrees for half circle, -90 to +90)
        const pressureAngle = Math.min(Math.max((pressure / 4.0) * 180 - 90, -90), 90);
        const tempAngle = Math.min(Math.max(((temperature - 20) / 80) * 180 - 90, -90), 90);
        const voltageAngle = Math.min(Math.max(((voltage - 8) / 8) * 180 - 90, -90), 90);
        const fuelAngle = Math.min(Math.max((fuel / 100) * 180 - 90, -90), 90);
        
        if (this.pressureNeedle) this.pressureNeedle.style.transform = `translate(-50%, -50%) rotate(${pressureAngle}deg)`;
        if (this.tempNeedle) this.tempNeedle.style.transform = `translate(-50%, -50%) rotate(${tempAngle}deg)`;
        if (this.voltageNeedle) this.voltageNeedle.style.transform = `translate(-50%, -50%) rotate(${voltageAngle}deg)`;
        if (this.fuelNeedle) this.fuelNeedle.style.transform = `translate(-50%, -50%) rotate(${fuelAngle}deg)`;
        
        // Update value displays
        if (this.pressureValue) this.pressureValue.textContent = `${pressure.toFixed(1)} BAR`;
        if (this.tempValue) this.tempValue.textContent = `${Math.round(temperature)}°C`;
        if (this.voltageValue) this.voltageValue.textContent = `${voltage.toFixed(1)}V`;
        if (this.fuelValue) this.fuelValue.textContent = `${Math.round(fuel)}%`;
    }
}

// Main dashboard controller that integrates the ignition key with backend polling
let pollingInterval;
let ignitionController;
const POLLING_INTERVAL = 1000; // 1 second

// Initialize dashboard when page loads
document.addEventListener('DOMContentLoaded', function() {
    console.log('Bobcat Ignition Controller Initializing...');
    
    // Initialize the ignition key controller
    ignitionController = new IgnitionController();
    
    // Start backend polling
    startPolling();
    
    // Initial status update
    updateStatus();
    
    console.log('Ignition Controller loaded!');
});

function updateStatus() {
    fetch('/status')
        .then(response => response.json())
        .then(data => {
            console.log('Status update:', data);
            updateDashboard(data);
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            console.error('CONNECTION ERROR: Check network connection');
        });
}

function updateDashboard(status) {
    // Update ignition controller with backend data
    if (ignitionController) {
        ignitionController.updateFromBackend(status);
    }
    
    // Update countdown display
    updateCountdownDisplay(status);
    
    // Update warning lights
    updateWarningLights(status);
    
    // Update master status in header
    updateMasterStatus(status);
}

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
    // Oil pressure light
    const oilLight = document.querySelector('#oil-warning .light');
    if (oilLight) {
        if (status.low_oil_pressure) {
            oilLight.classList.add('active');
        } else {
            oilLight.classList.remove('active');
        }
    }
    
    // Temperature light
    const tempLight = document.querySelector('#temp-warning .light');
    if (tempLight) {
        if (status.high_temperature) {
            tempLight.classList.add('active');
        } else {
            tempLight.classList.remove('active');
        }
    }
    
    // Battery light
    const batteryLight = document.querySelector('#battery-warning .light');
    if (batteryLight) {
        if (status.low_battery) {
            batteryLight.classList.add('active');
        } else {
            batteryLight.classList.remove('active');
        }
    }
    
    // Charge indicator
    const chargeLight = document.querySelector('#charge-indicator .light');
    if (chargeLight) {
        if (status.charging || (status.battery_voltage && status.battery_voltage > 13.0)) {
            chargeLight.classList.add('active');
        } else {
            chargeLight.classList.remove('active');
        }
    }
    
    // Engine run light
    const engineLight = document.querySelector('#engine-run .light');
    if (engineLight) {
        if (status.state === 'RUNNING') {
            engineLight.classList.add('active');
        } else {
            engineLight.classList.remove('active');
        }
    }
    
    // Fuel warning light
    const fuelLight = document.querySelector('#fuel-warning .light');
    if (fuelLight) {
        if (status.fuel_level && status.fuel_level < 25) {
            fuelLight.classList.add('active');
        } else {
            fuelLight.classList.remove('active');
        }
    }
}

function updateMasterStatus(status) {
    const masterStatus = document.querySelector('.master-status');
    if (masterStatus) {
        let statusText = 'SYSTEM';
        let statusClass = 'master-status';
        
        switch(status.state) {
            case 'OFF':
                statusText = 'OFFLINE';
                break;
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
            default:
                statusText = 'STANDBY';
        }
        
        masterStatus.textContent = statusText;
        masterStatus.className = statusClass;
    }
}

function startPolling() {
    // Clear any existing interval
    if (pollingInterval) {
        clearInterval(pollingInterval);
    }
    
    // Start polling for status updates
    pollingInterval = setInterval(updateStatus, POLLING_INTERVAL);
    console.log('Status polling started');
}

function stopPolling() {
    if (pollingInterval) {
        clearInterval(pollingInterval);
        pollingInterval = null;
        console.log('Status polling stopped');
    }
}

// Clean up when page is unloaded
window.addEventListener('beforeunload', stopPolling);

// Prevent zoom on double-tap for mobile devices
let lastTouchEnd = 0;
document.addEventListener('touchend', (event) => {
    const now = (new Date()).getTime();
    if (now - lastTouchEnd <= 300) {
        event.preventDefault();
    }
    lastTouchEnd = now;
}, false);

// Add helpful console methods for debugging
console.log('Available commands:');
console.log('- ignitionController.setKeyPosition(0-3) - Set key position');
console.log('- ignitionController.emergencyStop() - Emergency stop');
