// Bobcat 743 Dashboard Control Script
let currentKeyPosition = 'off';
let pollingInterval;
let isCranking = false;
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
        const position = button.dataset.position;
        
        if (position === 'start') {
            // START position: crank while held, return to ON when released
            button.addEventListener('mousedown', function(e) {
                e.preventDefault();
                startCranking();
            });
            
            button.addEventListener('mouseup', function(e) {
                e.preventDefault();
                stopCranking();
            });
            
            button.addEventListener('mouseleave', function(e) {
                e.preventDefault();
                stopCranking();
            });
            
            // Touch events for mobile
            button.addEventListener('touchstart', function(e) {
                e.preventDefault();
                startCranking();
            });
            
            button.addEventListener('touchend', function(e) {
                e.preventDefault();
                stopCranking();
            });
            
            button.addEventListener('touchcancel', function(e) {
                e.preventDefault();
                stopCranking();
            });
        } else {
            // OFF and ON positions: toggle states
            button.addEventListener('click', function() {
                setKeyPosition(this.dataset.position);
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
    console.log('Key position changed to:', position);
    
    if (!position) {
        console.error('No position provided to setKeyPosition');
        return;
    }
    
    currentKeyPosition = position;
    
    // Update visual key switch
    document.querySelectorAll('.key-position').forEach(btn => {
        btn.classList.remove('active');
    });
    
    const targetButton = document.querySelector(`[data-position="${position}"]`);
    if (targetButton) {
        targetButton.classList.add('active');
    } else {
        console.error('Target button not found for position:', position);
    }
    
    // Send command to controller
    let command;
    switch(position) {
        case 'off':
            command = 'shutdown';
            break;
        case 'on':
            command = 'power_on';
            break;
        case 'start':
            command = 'start';
            // Auto-return to ON position after start attempt
            setTimeout(() => {
                if (currentKeyPosition === 'start') {
                    setKeyPosition('on');
                }
            }, 2000);
            break;
        default:
            console.error('Unknown key position:', position);
            return;
    }
    
    if (command) {
        sendCommand(command);
    }
}

function startCranking() {
    if (isCranking) return; // Prevent multiple starts
    
    console.log('Starting engine crank (key held in START position)');
    isCranking = true;
    
    // Visual feedback - highlight START button
    const startButton = document.querySelector('[data-position="start"]');
    if (startButton) {
        startButton.classList.add('active');
    }
    
    // Send start command
    sendCommand('start');
    
    // Update status display
    const statusScreen = document.querySelector('.main-status');
    if (statusScreen) {
        statusScreen.textContent = 'CRANKING ENGINE';
        statusScreen.className = 'main-status status-starting';
    }
}

function stopCranking() {
    if (!isCranking) return; // Not currently cranking
    
    console.log('Stopping engine crank (key released from START position)');
    isCranking = false;
    
    // Visual feedback - remove START highlight, show ON as active
    const startButton = document.querySelector('[data-position="start"]');
    const onButton = document.querySelector('[data-position="on"]');
    
    if (startButton) {
        startButton.classList.remove('active');
    }
    
    if (onButton) {
        onButton.classList.add('active');
    }
    
    // Update current position to ON
    currentKeyPosition = 'on';
    
    // Send command to stop cranking and return to ON state
    sendCommand('stop_crank');
    
    // Update status after a brief delay
    setTimeout(() => {
        updateStatus();
    }, 500);
}

function sendCommand(action) {
    console.log('Sending command:', action);
    
    // Show immediate feedback
    showCommandFeedback(action);
    
    fetch('/control', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ action: action })
    })
    .then(response => response.json())
    .then(data => {
        console.log('Command response:', data);
        if (data.success) {
            console.log('Command executed successfully:', action);
        } else {
            console.error('Command failed:', data.message);
            showAlert('Command failed: ' + data.message);
        }
        // Update status after command
        setTimeout(updateStatus, 500);
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
    setKeyPosition('off');
    
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
            detailedText = 'Turn key to START to begin glow plug heating';
            break;
        case 'GLOW_HEATING':
            statusText = 'GLOW PLUGS HEATING';
            statusClass += ' status-glow-heating';
            detailedText = status.countdown ? 
                `${status.countdown}s remaining (or turn key to START to force crank)` : 
                'Turn key to START to force crank';
            break;
        case 'READY':
            statusText = 'READY TO START';
            statusClass += ' status-ready';
            detailedText = 'Turn key to START position to crank engine';
            break;
        case 'STARTING':
            statusText = 'STARTING ENGINE';
            statusClass += ' status-starting';
            detailedText = 'Engine cranking...';
            break;
        case 'RUNNING':
            statusText = 'ENGINE RUNNING';
            statusClass += ' status-running';
            detailedText = 'All systems operational (turn key to START for hot restart)';
            break;
        case 'LOW_OIL_PRESSURE':
            statusText = 'LOW OIL PRESSURE';
            statusClass += ' status-alert';
            detailedText = 'Check oil level - turn key to START to override';
            break;
        case 'HIGH_TEMPERATURE':
            statusText = 'HIGH TEMPERATURE';
            statusClass += ' status-alert';
            detailedText = 'Engine overheating - turn key to START to override';
            break;
        case 'EMERGENCY_STOP':
            statusText = 'EMERGENCY STOP';
            statusClass += ' status-alert';
            detailedText = 'Turn key to START to override and restart';
            break;
        default:
            statusText = status.state;
            detailedText = 'Turn key to START to attempt engine start';
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
        if (status.state === 'GLOW_HEATING') {
            glowLight.classList.add('active');
        } else {
            glowLight.classList.remove('active');
        }
    }
    
    if (countdownElement) {
        if (status.countdown && status.countdown > 0) {
            countdownElement.textContent = `${status.countdown}s`;
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
            case 'GLOW_HEATING':
                statusText = 'HEATING';
                statusClass += ' status-glow-heating';
                break;
            case 'READY':
                statusText = 'READY';
                statusClass += ' status-ready';
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
