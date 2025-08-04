// Bobcat 743 Dashboard Control Script
let currentKeyPosition = 0; // 0=OFF, 1=ON, 2=GLOW, 3=START
let pollingInterval;
let isCranking = false;
let startKeyHeld = false;
const POLLING_INTERVAL = 1000; // 1 second

// Initialize dashboard when page loads
document.addEventListener('DOMContentLoaded', function() {
    console.log('Bobcat 743 Dashboard Initializing...');
    initializeDashboard();
    startPolling();
});

function initializeDashboard() {
    console.log('Setting up event listeners...');
    
    // Set up key switch event listeners
    const keyPositions = document.querySelectorAll('.key-position');
    console.log('Found', keyPositions.length, 'key position buttons');
    keyPositions.forEach(button => {
        const position = parseInt(button.dataset.position);
        
        if (position === 3) { // START position
            // START position: crank while held, return to ON when released
            button.addEventListener('mousedown', function(e) {
                e.preventDefault();
                holdStartPosition();
            });
            
            button.addEventListener('mouseup', function(e) {
                e.preventDefault();
                releaseStartPosition();
            });
            
            button.addEventListener('mouseleave', function(e) {
                e.preventDefault();
                releaseStartPosition();
            });
            
            // Touch events for mobile
            button.addEventListener('touchstart', function(e) {
                e.preventDefault();
                holdStartPosition();
            });
            
            button.addEventListener('touchend', function(e) {
                e.preventDefault();
                releaseStartPosition();
            });
            
            button.addEventListener('touchcancel', function(e) {
                e.preventDefault();
                releaseStartPosition();
            });
        } else {
            // OFF, ON, and GLOW positions: sequential progression
            button.addEventListener('click', function() {
                setKeyPosition(parseInt(this.dataset.position));
            });
        }
    });
    
    // Set up auxiliary button event listeners
    const auxButtons = document.querySelectorAll('.aux-btn');
    console.log('Found', auxButtons.length, 'auxiliary buttons');
    auxButtons.forEach(button => {
        button.addEventListener('click', function() {
            const action = this.dataset.action;
            if (action) {
                sendCommand(action);
            } else {
                console.warn('Button missing data-action attribute:', this);
            }
        });
    });
    
    // Set up emergency stop
    const emergencyBtn = document.querySelector('.emergency-btn');
    if (emergencyBtn) {
        console.log('Emergency button found, setting up listener');
        emergencyBtn.addEventListener('click', function() {
            emergencyStop();
        });
    } else {
        console.warn('Emergency button not found');
    }
    
    console.log('Dashboard initialization complete');
    
    // Initial status update
    updateStatus();
}

function setKeyPosition(position) {
    console.log('Key position command sent:', position);
    
    if (position < 0 || position > 3) {
        console.error('Invalid key position:', position);
        return;
    }
    
    // NO local logic - just send command to backend
    // Backend will decide if it's valid and handle all state transitions
    sendCommand('key_position', { position: position });
    
    // Update status immediately to reflect backend changes
    setTimeout(updateStatus, 100);
}

function holdStartPosition() {
    if (currentKeyPosition < 2) {
        showAlert('Turn key to GLOW position first');
        return;
    }
    
    if (startKeyHeld) return; // Already holding
    
    console.log('START key held - cranking engine');
    startKeyHeld = true;
    isCranking = true;
    
    // Visual feedback - highlight START button
    const startButton = document.querySelector('[data-position="3"]');
    if (startButton) {
        startButton.classList.add('active', 'cranking');
    }
    
    // Send start command
    sendCommand('key_start_hold', { held: true });
    
    // Update status display
    const statusScreen = document.querySelector('.main-status');
    if (statusScreen) {
        statusScreen.textContent = 'CRANKING ENGINE';
        statusScreen.className = 'main-status status-starting';
    }
}

function releaseStartPosition() {
    if (!startKeyHeld) return; // Not currently holding
    
    console.log('START key released - returning to ON position');
    startKeyHeld = false;
    isCranking = false;
    
    // Visual feedback - remove START highlight, return to GLOW position
    const startButton = document.querySelector('[data-position="3"]');
    const glowButton = document.querySelector('[data-position="2"]');
    
    if (startButton) {
        startButton.classList.remove('active', 'cranking');
    }
    
    if (glowButton) {
        glowButton.classList.add('active');
    }
    
    // Return to GLOW position (realistic key behavior)
    currentKeyPosition = 2;
    
    // Send command to stop cranking
    sendCommand('key_start_hold', { held: false });
    
    // Update status after a brief delay
    setTimeout(() => {
        updateStatus();
    }, 500);
}

function sendCommand(action, data = {}) {
    console.log('Sending command:', action, data);
    
    // Show immediate feedback
    showCommandFeedback(action);
    
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
        console.log('Command response:', data);
        if (data.success) {
            console.log('Command executed successfully:', action);
            // Show success message briefly if it's informative
            if (data.message && data.message.includes('Cannot skip') || data.message.includes('Emergency')) {
                showAlert(data.message);
            }
        } else {
            console.error('Command failed:', data.message);
            showAlert('Command failed: ' + data.message);
        }
        // Update status immediately after command to get latest state
        setTimeout(updateStatus, 100);
    })
    .catch(error => {
        console.error('Error sending command:', error);
        showAlert('Communication error');
    });
}

function showCommandFeedback(action) {
    const statusScreen = document.querySelector('.main-status');
    const originalText = statusScreen.textContent;
    
    let feedbackText;
    switch(action) {
        case 'start':
            feedbackText = 'STARTING...';
            break;
        case 'shutdown':
            feedbackText = 'SHUTTING DOWN...';
            break;
        case 'power_on':
            feedbackText = 'POWERING ON...';
            break;
        case 'emergency_stop':
            feedbackText = 'EMERGENCY STOP!';
            break;
        case 'lights':
            feedbackText = 'TOGGLING LIGHTS...';
            break;
        case 'horn':
            feedbackText = 'HORN ACTIVATED';
            break;
        default:
            feedbackText = 'PROCESSING...';
    }
    
    statusScreen.textContent = feedbackText;
    statusScreen.className = 'main-status status-alert';
    
    // Restore after 2 seconds
    setTimeout(() => {
        statusScreen.textContent = originalText;
        updateStatus();
    }, 2000);
}

function emergencyStop() {
    console.log('EMERGENCY STOP ACTIVATED!');
    sendCommand('emergency_stop');
    
    // Reset key to OFF position
    setKeyPosition(0);
    startKeyHeld = false;
    isCranking = false;
    
    // Flash emergency state
    const statusScreen = document.querySelector('.main-status');
    statusScreen.textContent = 'EMERGENCY STOP!';
    statusScreen.className = 'main-status status-alert';
    
    // Flash warning lights
    document.querySelectorAll('.light.red').forEach(light => {
        light.classList.add('active');
    });
}

function updateStatus() {
    fetch('/status')
        .then(response => response.json())
        .then(data => {
            console.log('Status update:', data);
            updateDashboard(data);
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            showConnectionError();
        });
}

function updateDashboard(status) {
    // Update main status display
    updateMainStatus(status);
    
    // Update key position from backend (authoritative source)
    updateKeyPosition(status.key_position || 0);
    
    // Update glow plug indicator and countdown
    updateGlowPlug(status);
    
    // Update warning lights
    updateWarningLights(status);
    
    // Update gauges
    updateGauges(status);
    
    // Update auxiliary button states
    updateAuxiliaryStates(status);
    
    // Update master status in header
    updateMasterStatus(status);
}

function updateKeyPosition(backendKeyPosition) {
    // Update our local tracking to match backend
    currentKeyPosition = backendKeyPosition;
    
    // Update visual key switch to match backend state
    document.querySelectorAll('.key-position').forEach(btn => {
        btn.classList.remove('active');
    });
    
    const activeButton = document.querySelector(`[data-position="${backendKeyPosition}"]`);
    if (activeButton) {
        activeButton.classList.add('active');
    }
}

function updateMainStatus(status) {
    const statusElement = document.querySelector('.main-status');
    const detailedElement = document.querySelector('.detailed-status');
    
    if (!statusElement) return;
    
    let statusText = status.state || 'UNKNOWN';
    let statusClass = 'main-status';
    let detailedText = '';
    
    switch(status.state) {
        case 'OFF':
            statusText = 'SYSTEM OFF';
            statusClass += ' status-off';
            detailedText = 'Turn key to ON position';
            break;
        case 'ON':
            statusText = 'SYSTEM ON';
            statusClass += ' status-ready';
            detailedText = 'Turn key to GLOW position to heat glow plugs';
            break;
        case 'GLOW_HEATING':
            statusText = 'GLOW PLUGS HEATING';
            statusClass += ' status-glow-heating';
            detailedText = status.countdown ? 
                `${status.countdown}s remaining (or hold START to force crank)` : 
                'Glow plugs ready - hold START to crank engine';
            break;
        case 'STARTING':
            statusText = 'CRANKING ENGINE';
            statusClass += ' status-starting';
            detailedText = 'Engine cranking... (release START key when engine starts)';
            break;
        case 'RUNNING':
            statusText = 'ENGINE RUNNING';
            statusClass += ' status-running';
            detailedText = 'All systems operational (key in ON position)';
            break;
        case 'LOW_OIL_PRESSURE':
            statusText = 'LOW OIL PRESSURE';
            statusClass += ' status-alert';
            detailedText = 'Check oil level - hold START to override';
            break;
        case 'HIGH_TEMPERATURE':
            statusText = 'HIGH TEMPERATURE';
            statusClass += ' status-alert';
            detailedText = 'Engine overheating - hold START to override';
            break;
        case 'EMERGENCY_STOP':
            statusText = 'EMERGENCY STOP';
            statusClass += ' status-alert';
            detailedText = 'Emergency stop activated - turn key to OFF to reset';
            break;
        default:
            statusText = status.state;
            detailedText = 'Turn key step by step: OFF → ON → GLOW → START';
    }
    
    statusElement.textContent = statusText;
    statusElement.className = statusClass;
    
    if (detailedElement) {
        detailedElement.textContent = detailedText;
    }
}

function updateGlowPlug(status) {
    const glowLight = document.querySelector('.light-bulb');
    const countdownElement = document.querySelector('.countdown');
    
    if (glowLight) {
        // Use actual relay status for accurate indication
        if (status.glow_plugs_on || status.glow_active) {
            glowLight.classList.add('active');
        } else {
            glowLight.classList.remove('active');
        }
    }
    
    if (countdownElement) {
        // Show countdown if glow plugs are active OR if there's a countdown value
        if ((status.glow_active || status.glow_plugs_on) && status.countdown && status.countdown > 0) {
            countdownElement.textContent = `${status.countdown}s`;
            countdownElement.style.display = 'block';
        } else if (status.glow_active || status.glow_plugs_on) {
            // Glow plugs are on but no countdown (heating complete)
            countdownElement.textContent = 'Ready';
            countdownElement.style.display = 'block';
        } else {
            countdownElement.textContent = '--';
            countdownElement.style.display = 'block';
        }
    }
}

function updateWarningLights(status) {
    // Engine warning light
    const engineLight = document.querySelector('[data-warning="engine"] .light');
    if (engineLight) {
        if (status.state === 'EMERGENCY_STOP' || status.engine_fault) {
            engineLight.classList.add('active');
        } else {
            engineLight.classList.remove('active');
        }
    }
    
    // Oil pressure light
    const oilLight = document.querySelector('[data-warning="oil"] .light');
    if (oilLight) {
        if (status.low_oil_pressure) {
            oilLight.classList.add('active');
        } else {
            oilLight.classList.remove('active');
        }
    }
    
    // Temperature light
    const tempLight = document.querySelector('[data-warning="temp"] .light');
    if (tempLight) {
        if (status.high_temperature) {
            tempLight.classList.add('active');
        } else {
            tempLight.classList.remove('active');
        }
    }
    
    // Battery light
    const batteryLight = document.querySelector('[data-warning="battery"] .light');
    if (batteryLight) {
        if (status.low_battery) {
            batteryLight.classList.add('active');
        } else {
            batteryLight.classList.remove('active');
        }
    }
    
    // Work lights indicator
    const workLight = document.querySelector('[data-warning="work"] .light');
    if (workLight) {
        if (status.lights_on) {
            workLight.classList.add('active');
        } else {
            workLight.classList.remove('active');
        }
    }
    
    // System ready light
    const readyLight = document.querySelector('[data-warning="ready"] .light');
    if (readyLight) {
        if (status.state === 'READY' || status.state === 'RUNNING') {
            readyLight.classList.add('active');
        } else {
            readyLight.classList.remove('active');
        }
    }
}

function updateGauges(status) {
    // Fuel gauge
    updateGauge('fuel', status.fuel_level || 75, 0, 100);
    
    // Temperature gauge
    updateGauge('temp', status.engine_temp || 85, 0, 120);
    
    // Oil pressure (digital)
    updateDigitalGauge('oil-pressure', status.oil_pressure || 45, 'PSI');
    
    // Battery voltage (digital)
    updateDigitalGauge('battery', status.battery_voltage || 12.8, 'V');
    
    // Engine hours (digital)
    updateDigitalGauge('hours', status.engine_hours || 1234, 'HR');
}

function updateGauge(gaugeName, value, min, max) {
    const needle = document.querySelector(`[data-gauge="${gaugeName}"] .gauge-needle`);
    const valueElement = document.querySelector(`[data-gauge="${gaugeName}"] .gauge-value`);
    
    if (needle) {
        // Calculate rotation (-90 to 90 degrees)
        const percentage = Math.max(0, Math.min(100, ((value - min) / (max - min)) * 100));
        const rotation = -90 + (percentage * 1.8); // 180 degrees total range
        needle.style.transform = `translate(-50%, -100%) rotate(${rotation}deg)`;
    }
    
    if (valueElement) {
        valueElement.textContent = Math.round(value);
    }
}

function updateDigitalGauge(gaugeName, value, unit) {
    const valueElement = document.querySelector(`[data-gauge="${gaugeName}"] .digital-value`);
    if (valueElement) {
        if (typeof value === 'number') {
            valueElement.textContent = value.toFixed(1) + (unit || '');
        } else {
            valueElement.textContent = value + (unit || '');
        }
    }
}

function updateAuxiliaryStates(status) {
    // Update lights button
    const lightsBtn = document.querySelector('[data-action="lights"]');
    if (lightsBtn) {
        if (status.lights_on) {
            lightsBtn.classList.add('active');
        } else {
            lightsBtn.classList.remove('active');
        }
    }
    
    // Add other auxiliary states as needed
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

function showConnectionError() {
    const statusElement = document.querySelector('.main-status');
    const detailedElement = document.querySelector('.detailed-status');
    
    if (statusElement) {
        statusElement.textContent = 'CONNECTION ERROR';
        statusElement.className = 'main-status status-alert';
    }
    
    if (detailedElement) {
        detailedElement.textContent = 'Check network connection';
    }
}

function showAlert(message) {
    // You could implement a proper alert system here
    console.warn('Alert:', message);
    
    // For now, just update the status briefly
    const statusElement = document.querySelector('.main-status');
    if (statusElement) {
        const originalText = statusElement.textContent;
        const originalClass = statusElement.className;
        
        statusElement.textContent = message;
        statusElement.className = 'main-status status-alert';
        
        setTimeout(() => {
            statusElement.textContent = originalText;
            statusElement.className = originalClass;
        }, 3000);
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
