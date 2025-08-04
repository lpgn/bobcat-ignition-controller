// Ignition Controller JavaScript
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
        
        // Audio elements
        this.clickSound = document.getElementById('clickSound');
        this.glowSound = document.getElementById('glowSound');
        this.startSound = document.getElementById('startSound');
        
        // State management
        this.currentState = 'off';
        this.currentAngle = -30; // Start at OFF position (11 o'clock)
        this.isDragging = false;
        this.startAngle = 0;
        this.isTransitioning = false;
        
        // Position definitions (clock-based angles)
        this.positions = {
            off: { angle: -30, label: 'OFF', color: 'off' },      // 11 o'clock
            on: { angle: 0, label: 'ON', color: 'on' },           // 12 o'clock
            glow: { angle: 45, label: 'GLOW', color: 'glow' },    // 1:30 o'clock (halfway between 12 and 3)
            start: { angle: 90, label: 'START', color: 'start' }  // 3 o'clock
        };
        
        // State progression order
        this.stateOrder = ['off', 'on', 'glow', 'start'];
        
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.updateDisplay();
        this.updateKeyPosition();
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
            
            // Update state based on angle
            const newState = this.getStateFromAngle(this.currentAngle);
            if (newState !== this.currentState) {
                this.currentState = newState;
                this.updateDisplay();
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
        // Find the appropriate state based on angle ranges
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
        this.playSound(closestState);
        
        // Handle START position spring-back
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
        // Auto-return from START to ON
        this.currentAngle = this.positions.on.angle;
        this.currentState = 'on';
        this.updateKeyPosition();
        this.updateDisplay();
        this.playSound('on');
        
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
            label.classList.remove('active');
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
        
        // Update gauges based on ignition state
        this.updateGauges();
        
        // Update page title
        const position = this.positions[this.currentState];
        document.title = `Bobcat Ignition - ${position.label}`;
    }
    
    updateGauges() {
        let pressure = 0;      // BAR
        let temperature = 20;  // Celsius
        let voltage = 0;       // Volts
        let fuel = 75;         // Percentage
        
        // Simulate realistic values based on ignition state
        switch (this.currentState) {
            case 'off':
                pressure = 0;
                temperature = 20;
                voltage = 0;
                fuel = 75 + Math.random() * 5; // Fuel level stays relatively stable
                this.pressureGauge.classList.remove('active');
                this.tempGauge.classList.remove('active');
                this.voltageGauge.classList.remove('active');
                this.fuelGauge.classList.remove('active');
                break;
            case 'on':
                pressure = 1 + Math.random() * 0.3; // 1.0-1.3 BAR
                temperature = 25 + Math.random() * 5; // 25-30°C
                voltage = 12 + Math.random() * 0.5; // 12.0-12.5V
                fuel = 75 + Math.random() * 5;
                this.pressureGauge.classList.add('active');
                this.tempGauge.classList.add('active');
                this.voltageGauge.classList.add('active');
                this.fuelGauge.classList.add('active');
                break;
            case 'glow':
                pressure = 1.3 + Math.random() * 0.4; // 1.3-1.7 BAR
                temperature = 50 + Math.random() * 10; // 50-60°C (glow plugs heating)
                voltage = 11.5 + Math.random() * 0.5; // 11.5-12.0V (power draw from glow plugs)
                fuel = 75 + Math.random() * 5;
                this.pressureGauge.classList.add('active');
                this.tempGauge.classList.add('active');
                this.voltageGauge.classList.add('active');
                this.fuelGauge.classList.add('active');
                break;
            case 'start':
                pressure = 2.4 + Math.random() * 0.7; // 2.4-3.1 BAR (high pressure during start)
                temperature = 60 + Math.random() * 10; // 60-70°C
                voltage = 10 + Math.random() * 1; // 10-11V (voltage drop during cranking)
                fuel = 74 + Math.random() * 5; // Slight fuel consumption
                this.pressureGauge.classList.add('active');
                this.tempGauge.classList.add('active');
                this.voltageGauge.classList.add('active');
                this.fuelGauge.classList.add('active');
                break;
        }
        
        // Update needle positions (0-180 degrees for half circle, -90 to +90)
        const pressureAngle = Math.min((pressure / 3.5) * 180 - 90, 90); // Max 3.5 BAR
        const tempAngle = Math.min(((temperature - 20) / 80) * 180 - 90, 90); // 20-100°C range
        const voltageAngle = Math.min(((voltage - 8) / 8) * 180 - 90, 90); // 8-16V range
        const fuelAngle = Math.min((fuel / 100) * 180 - 90, 90); // 0-100% range
        
        this.pressureNeedle.style.transform = `translateX(-50%) rotate(${pressureAngle}deg)`;
        this.tempNeedle.style.transform = `translateX(-50%) rotate(${tempAngle}deg)`;
        this.voltageNeedle.style.transform = `translateX(-50%) rotate(${voltageAngle}deg)`;
        this.fuelNeedle.style.transform = `translateX(-50%) rotate(${fuelAngle}deg)`;
        
        // Update value displays
        this.pressureValue.textContent = `${pressure.toFixed(1)} BAR`;
        this.tempValue.textContent = `${Math.round(temperature)}°C`;
        this.voltageValue.textContent = `${voltage.toFixed(1)}V`;
        this.fuelValue.textContent = `${Math.round(fuel)}%`;
    }
    
    playSound(state) {
        // Audio disabled for better user experience
        // The visual feedback is sufficient for this interface
        return;
        
        /* Original audio code kept for reference but disabled
        try {
            // Stop all current sounds
            this.clickSound.pause();
            this.glowSound.pause();
            this.startSound.pause();
            
            this.clickSound.currentTime = 0;
            this.glowSound.currentTime = 0;
            this.startSound.currentTime = 0;
            
            // Set very low volume for subtle feedback
            this.clickSound.volume = 0.1;
            this.glowSound.volume = 0.1;
            this.startSound.volume = 0.1;
            
            // Play appropriate sound
            switch (state) {
                case 'glow':
                    this.glowSound.play().catch(() => {});
                    break;
                case 'start':
                    this.startSound.play().catch(() => {});
                    break;
                default:
                    this.clickSound.play().catch(() => {});
                    break;
            }
        } catch (error) {
            // Ignore audio errors
            console.log('Audio playback not available');
        }
        */
    }
    
    // Public method to programmatically set state (for testing/debugging)
    setState(stateName) {
        if (this.positions[stateName] && !this.isTransitioning) {
            this.currentState = stateName;
            this.currentAngle = this.positions[stateName].angle;
            this.updateKeyPosition();
            this.updateDisplay();
        }
    }
    
    // Public method to get current state
    getState() {
        return {
            state: this.currentState,
            angle: this.currentAngle,
            label: this.positions[this.currentState].label
        };
    }
}

// Initialize the ignition controller when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.ignitionController = new IgnitionController();
    
    // Add some helpful console methods for debugging
    console.log('Bobcat Ignition Controller loaded!');
    console.log('Available commands:');
    console.log('- ignitionController.getState() - Get current state');
    console.log('- ignitionController.setState("off"|"on"|"glow"|"start") - Set state programmatically');
});

// Handle page visibility changes (pause audio when tab is hidden)
document.addEventListener('visibilitychange', () => {
    if (document.hidden && window.ignitionController) {
        // Pause any playing audio when tab becomes hidden
        try {
            window.ignitionController.clickSound.pause();
            window.ignitionController.glowSound.pause();
            window.ignitionController.startSound.pause();
        } catch (error) {
            // Ignore errors
        }
    }
});

// Prevent zoom on double-tap for mobile devices
let lastTouchEnd = 0;
document.addEventListener('touchend', (event) => {
    const now = (new Date()).getTime();
    if (now - lastTouchEnd <= 300) {
        event.preventDefault();
    }
    lastTouchEnd = now;
}, false);
