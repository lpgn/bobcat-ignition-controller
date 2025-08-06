/**
 * Settings Interface JavaScript for Bobcat Ignition Controller
 * Handles configuration management, WiFi setup, and OTA updates
 */

class SettingsManager {
    constructor() {
        this.settings = {};
        this.init();
    }

    init() {
        this.loadCurrentSettings();
        this.setupEventListeners();
    }

    async loadCurrentSettings() {
        try {
            const response = await fetch('/api/settings');
            if (response.ok) {
                this.settings = await response.json();
                this.populateForm();
            } else {
                this.showStatus('Failed to load current settings', 'error');
            }
        } catch (error) {
            console.error('Error loading settings:', error);
            this.loadDefaultValues();
        }
    }

    loadDefaultValues() {
        // Engine Parameters
        document.getElementById('glowDuration').value = 20;
        document.getElementById('crankingTimeout').value = 10;
        document.getElementById('cooldownDuration').value = 120;
        
        // Alarm Thresholds
        document.getElementById('maxTemp').value = 104;
        document.getElementById('minOilPressure').value = 69;
        document.getElementById('minVoltage').value = 11.0;
        document.getElementById('maxVoltage').value = 15.0;
        
        // WiFi (leave empty for user to fill)
        document.getElementById('wifiSSID').value = 'Bobcat-743';
        document.getElementById('wifiPassword').value = '';
        
        // Sensor Calibration
        document.getElementById('tempOffset').value = -40.0;
        document.getElementById('pressureScale').value = 0.17;
        document.getElementById('fuelEmpty').value = 200;
        document.getElementById('fuelFull').value = 3800;
    }

    populateForm() {
        if (!this.settings) return;
        
        // Engine Parameters
        if (this.settings.glowDuration) document.getElementById('glowDuration').value = this.settings.glowDuration;
        if (this.settings.crankingTimeout) document.getElementById('crankingTimeout').value = this.settings.crankingTimeout;
        if (this.settings.cooldownDuration) document.getElementById('cooldownDuration').value = this.settings.cooldownDuration;
        
        // Alarm Thresholds
        if (this.settings.maxTemp) document.getElementById('maxTemp').value = this.settings.maxTemp;
        if (this.settings.minOilPressure) document.getElementById('minOilPressure').value = this.settings.minOilPressure;
        if (this.settings.minVoltage) document.getElementById('minVoltage').value = this.settings.minVoltage;
        if (this.settings.maxVoltage) document.getElementById('maxVoltage').value = this.settings.maxVoltage;
        
        // WiFi
        if (this.settings.wifiSSID) document.getElementById('wifiSSID').value = this.settings.wifiSSID;
        // Never populate password for security
        
        // Sensor Calibration
        if (this.settings.tempOffset) document.getElementById('tempOffset').value = this.settings.tempOffset;
        if (this.settings.pressureScale) document.getElementById('pressureScale').value = this.settings.pressureScale;
        if (this.settings.fuelEmpty) document.getElementById('fuelEmpty').value = this.settings.fuelEmpty;
        if (this.settings.fuelFull) document.getElementById('fuelFull').value = this.settings.fuelFull;
    }

    setupEventListeners() {
        // Form validation
        document.querySelectorAll('.setting-input').forEach(input => {
            input.addEventListener('input', () => this.validateInput(input));
        });
    }

    validateInput(input) {
        const value = parseFloat(input.value);
        const min = parseFloat(input.min);
        const max = parseFloat(input.max);
        
        if (input.type === 'number' && (value < min || value > max)) {
            input.style.borderColor = '#e74c3c';
        } else {
            input.style.borderColor = '#555';
        }
    }

    collectFormData() {
        return {
            // Engine Parameters
            glowDuration: parseInt(document.getElementById('glowDuration').value),
            crankingTimeout: parseInt(document.getElementById('crankingTimeout').value),
            cooldownDuration: parseInt(document.getElementById('cooldownDuration').value),
            
            // Alarm Thresholds
            maxTemp: parseInt(document.getElementById('maxTemp').value),
            minOilPressure: parseInt(document.getElementById('minOilPressure').value),
            minVoltage: parseFloat(document.getElementById('minVoltage').value),
            maxVoltage: parseFloat(document.getElementById('maxVoltage').value),
            
            // WiFi Configuration
            wifiSSID: document.getElementById('wifiSSID').value,
            wifiPassword: document.getElementById('wifiPassword').value,
            
            // Sensor Calibration
            tempOffset: parseFloat(document.getElementById('tempOffset').value),
            pressureScale: parseFloat(document.getElementById('pressureScale').value),
            fuelEmpty: parseInt(document.getElementById('fuelEmpty').value),
            fuelFull: parseInt(document.getElementById('fuelFull').value)
        };
    }

    async saveSettings() {
        try {
            const settings = this.collectFormData();
            
            // Validate settings
            if (!this.validateSettings(settings)) {
                this.showStatus('Invalid settings. Please check your inputs.', 'error');
                return;
            }

            const response = await fetch('/api/settings', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(settings)
            });

            if (response.ok) {
                this.showStatus('Settings saved successfully! Changes will take effect after restart.', 'success');
                this.settings = settings;
            } else {
                const error = await response.text();
                this.showStatus(`Failed to save settings: ${error}`, 'error');
            }
        } catch (error) {
            console.error('Error saving settings:', error);
            this.showStatus('Network error while saving settings', 'error');
        }
    }

    validateSettings(settings) {
        // Basic validation
        if (settings.glowDuration < 5 || settings.glowDuration > 60) return false;
        if (settings.crankingTimeout < 5 || settings.crankingTimeout > 30) return false;
        if (settings.maxTemp < 80 || settings.maxTemp > 120) return false;
        if (settings.minOilPressure < 30 || settings.minOilPressure > 150) return false;
        if (settings.minVoltage < 10 || settings.minVoltage > 13) return false;
        if (settings.maxVoltage < 14 || settings.maxVoltage > 16) return false;
        if (settings.wifiSSID.length < 1 || settings.wifiSSID.length > 32) return false;
        if (settings.wifiPassword.length > 0 && settings.wifiPassword.length < 8) return false;
        
        return true;
    }

    loadDefaults() {
        if (confirm('Load default settings? This will overwrite all current values.')) {
            this.loadDefaultValues();
            this.showStatus('Default settings loaded. Click Save to apply.', 'success');
        }
    }

    async resetSystem() {
        if (!confirm('Factory reset will erase ALL settings and restart the device. Continue?')) {
            return;
        }
        
        if (!confirm('This action cannot be undone. Are you absolutely sure?')) {
            return;
        }

        try {
            const response = await fetch('/api/factory-reset', {
                method: 'POST'
            });

            if (response.ok) {
                this.showStatus('Factory reset initiated. Device will restart...', 'success');
                setTimeout(() => {
                    window.location.href = '/';
                }, 3000);
            } else {
                this.showStatus('Failed to initiate factory reset', 'error');
            }
        } catch (error) {
            console.error('Error during factory reset:', error);
            this.showStatus('Network error during factory reset', 'error');
        }
    }

    async initiateOTA() {
        // Open ElegantOTA interface in new tab
        window.open('/update', '_blank');
        this.showStatus('Opening OTA update interface...', 'info');
    }

    showStatus(message, type) {
        const statusDiv = document.getElementById('statusMessage');
        statusDiv.textContent = message;
        statusDiv.className = `status-message ${type}`;
        statusDiv.style.display = 'block';
        
        // Auto-hide after 5 seconds
        setTimeout(() => {
            statusDiv.style.display = 'none';
        }, 5000);
    }
}

// Global functions for HTML onclick handlers
function togglePasswordVisibility() {
    const passwordInput = document.getElementById('wifiPassword');
    const checkbox = document.getElementById('showPassword');
    
    passwordInput.type = checkbox.checked ? 'text' : 'password';
}

function saveSettings() {
    window.settingsManager.saveSettings();
}

function loadDefaults() {
    window.settingsManager.loadDefaults();
}

function resetSystem() {
    window.settingsManager.resetSystem();
}

function initiateOTA() {
    window.settingsManager.initiateOTA();
}

// Power management functions
function toggleSleepMode() {
    fetch('/control', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ action: 'toggle_sleep_mode' })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            window.settingsManager.showStatus(data.message, 'success');
            updatePowerStatus(); // Refresh power status
        } else {
            window.settingsManager.showStatus('Failed to toggle sleep mode: ' + data.message, 'error');
        }
    })
    .catch(error => {
        console.error('Error toggling sleep mode:', error);
        window.settingsManager.showStatus('Network error toggling sleep mode', 'error');
    });
}

function sleepNow() {
    if (!confirm('Put system to sleep now? You will need to press the BOOT button to wake it up.')) {
        return;
    }
    
    fetch('/control', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ action: 'sleep_now' })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            window.settingsManager.showStatus('System entering sleep mode...', 'success');
        } else {
            window.settingsManager.showStatus('Cannot sleep: ' + data.message, 'error');
        }
    })
    .catch(error => {
        console.error('Error entering sleep mode:', error);
        window.settingsManager.showStatus('Network error entering sleep mode', 'error');
    });
}

function updatePowerStatus() {
    fetch('/status')
    .then(response => response.json())
    .then(data => {
        // Update sleep mode status
        const sleepStatus = document.getElementById('sleepStatus');
        if (sleepStatus) {
            sleepStatus.textContent = `Sleep mode: ${data.sleep_mode_enabled ? 'Enabled' : 'Disabled'}`;
            sleepStatus.style.color = data.sleep_mode_enabled ? '#2ecc71' : '#e74c3c';
        }
        
        // Update activity time
        const activityTime = document.getElementById('activityTime');
        if (activityTime && data.time_since_activity !== undefined) {
            const minutes = Math.floor(data.time_since_activity / 60);
            const seconds = data.time_since_activity % 60;
            activityTime.textContent = `Time since activity: ${minutes}m ${seconds}s`;
            
            if (data.time_until_sleep !== undefined && data.time_until_sleep > 0) {
                const sleepMinutes = Math.floor(data.time_until_sleep / 60);
                const sleepSeconds = data.time_until_sleep % 60;
                activityTime.textContent += ` (sleep in: ${sleepMinutes}m ${sleepSeconds}s)`;
            }
        }
    })
    .catch(error => {
        console.error('Error updating power status:', error);
    });
}

// Initialize settings manager when page loads
document.addEventListener('DOMContentLoaded', () => {
    window.settingsManager = new SettingsManager();
    
    // Update power status every 5 seconds
    updatePowerStatus();
    setInterval(updatePowerStatus, 5000);
    
    // Start auto-refresh for sensor data
    refreshSensorData();
    window.sensorRefreshInterval = setInterval(refreshSensorData, 2000);
});

// Raw sensor data functions
let autoRefreshEnabled = true;
let sensorRefreshInterval = null;

function toggleAutoRefresh() {
    const checkbox = document.getElementById('autoRefresh');
    autoRefreshEnabled = checkbox.checked;
    
    if (autoRefreshEnabled) {
        if (sensorRefreshInterval) clearInterval(sensorRefreshInterval);
        sensorRefreshInterval = setInterval(refreshSensorData, 2000);
        refreshSensorData(); // Immediate refresh
    } else {
        if (sensorRefreshInterval) {
            clearInterval(sensorRefreshInterval);
            sensorRefreshInterval = null;
        }
    }
}

function refreshSensorData() {
    if (!autoRefreshEnabled) return;
    
    fetch('/api/raw-sensors')
    .then(response => response.json())
    .then(data => {
        // Update raw ADC readings
        document.getElementById('batteryRaw').textContent = data.battery_raw || '--';
        document.getElementById('tempRaw').textContent = data.temperature_raw || '--';
        document.getElementById('pressureRaw').textContent = data.pressure_raw || '--';
        document.getElementById('fuelRaw').textContent = data.fuel_raw || '--';
        
        // Update calculated values
        document.getElementById('batteryCalc').textContent = (data.battery_calculated || 0).toFixed(2) + ' V';
        document.getElementById('tempCalc').textContent = (data.temperature_calculated || 0).toFixed(1) + ' °C';
        document.getElementById('pressureCalc').textContent = (data.pressure_calculated || 0).toFixed(1) + ' kPa';
        document.getElementById('fuelCalc').textContent = (data.fuel_calculated || 0).toFixed(1) + ' %';
        
        // Update calibration input placeholders/values
        if (data.battery_divider) {
            const batteryDividerDisplay = document.getElementById('batteryDividerDisplay');
            if (batteryDividerDisplay) {
                batteryDividerDisplay.textContent = data.battery_divider.toFixed(6);
            }
        }
        if (data.temp_scale) {
            const tempScaleDisplay = document.getElementById('tempScaleDisplay');
            if (tempScaleDisplay) {
                tempScaleDisplay.textContent = data.temp_scale.toFixed(4);
            }
        }
        if (data.pressure_scale) {
            const pressureScaleDisplay = document.getElementById('pressureScaleDisplay');
            if (pressureScaleDisplay) {
                pressureScaleDisplay.textContent = data.pressure_scale.toFixed(4);
            }
        }
    })
    .catch(error => {
        console.error('Error refreshing sensor data:', error);
        // Show error in table
        const cells = ['batteryRaw', 'tempRaw', 'pressureRaw', 'fuelRaw', 
                      'batteryCalc', 'tempCalc', 'pressureCalc', 'fuelCalc'];
        cells.forEach(cellId => {
            const element = document.getElementById(cellId);
            if (element) element.textContent = 'Error';
        });
    });
}

function autoCalibrate() {
    // Get current raw sensor readings
    fetch('/api/raw-sensors')
    .then(response => response.json())
    .then(data => {
        const actualVoltage = parseFloat(document.getElementById('actualVoltage').value);
        const actualTemp = parseFloat(document.getElementById('actualTemp').value);
        const actualPressure = parseFloat(document.getElementById('actualPressure').value);
        const actualFuelLevel = parseFloat(document.getElementById('actualFuelLevel').value);
        
        if (!actualVoltage && !actualTemp && !actualPressure && !actualFuelLevel) {
            window.settingsManager.showStatus('Please enter at least one actual measurement value', 'error');
            return;
        }
        
        // Calculate new calibration constants
        const calibrationData = new FormData();
        let calibrationsApplied = [];
        
        // Battery voltage calibration
        if (actualVoltage && data.battery_raw) {
            // New divider = actual_voltage / raw_adc_reading
            const newDivider = actualVoltage / data.battery_raw;
            calibrationData.append('battery_divider', newDivider.toFixed(6));
            calibrationsApplied.push(`Battery: ${newDivider.toFixed(6)} (${actualVoltage}V)`);
        }
        
        // Temperature calibration
        if (actualTemp && data.temperature_raw) {
            // For pull-up configuration: V = 3.3V × (R_sensor ÷ (10kΩ + R_sensor))
            // We need to calculate new scale factor based on actual temperature
            // Assuming NTC with known behavior, calculate new scale
            const tempOffset = data.temp_offset || -40.0; // Use current offset
            const newScale = (actualTemp - tempOffset) / data.temperature_raw;
            calibrationData.append('temp_scale', newScale.toFixed(6));
            calibrationsApplied.push(`Temperature: ${newScale.toFixed(6)} (${actualTemp}°C)`);
        }
        
        // Pressure calibration  
        if (actualPressure && data.pressure_raw) {
            // new_scale = actual_pressure / raw_adc
            const newScale = actualPressure / data.pressure_raw;
            calibrationData.append('pressure_scale', newScale.toFixed(6));
            calibrationsApplied.push(`Pressure: ${newScale.toFixed(6)} (${actualPressure}kPa)`);
        }
        
        // Fuel level calibration - two-point calibration
        if (actualFuelLevel !== undefined && data.fuel_raw) {
            if (actualFuelLevel <= 10) {
                // This is empty/low reading - set as fuel_empty
                calibrationData.append('fuel_empty', data.fuel_raw.toString());
                calibrationsApplied.push(`Fuel Empty: ${data.fuel_raw} ADC (${actualFuelLevel}%)`);
            } else if (actualFuelLevel >= 90) {
                // This is full/high reading - set as fuel_full  
                calibrationData.append('fuel_full', data.fuel_raw.toString());
                calibrationsApplied.push(`Fuel Full: ${data.fuel_raw} ADC (${actualFuelLevel}%)`);
            } else {
                // Mid-range - interpolate between current empty/full values
                const currentEmpty = data.fuel_empty || 200;
                const currentFull = data.fuel_full || 3800;
                
                // Linear interpolation to adjust scale
                const targetADC = currentEmpty + (actualFuelLevel / 100.0) * (currentFull - currentEmpty);
                const scaleFactor = targetADC / data.fuel_raw;
                
                const newEmpty = Math.round(currentEmpty * scaleFactor);
                const newFull = Math.round(currentFull * scaleFactor);
                
                calibrationData.append('fuel_empty', newEmpty.toString());
                calibrationData.append('fuel_full', newFull.toString());
                calibrationsApplied.push(`Fuel Scale: Empty=${newEmpty}, Full=${newFull} ADC (${actualFuelLevel}%)`);
            }
        }
        
        if (calibrationsApplied.length === 0) {
            window.settingsManager.showStatus('No valid calibrations to apply', 'error');
            return;
        }
        
        // Confirm before applying
        const confirmMessage = `Apply these calibrations?\n\n${calibrationsApplied.join('\n')}\n\nChanges applied immediately.`;
        if (!confirm(confirmMessage)) {
            return;
        }
        
        // Send calibration data
        fetch('/api/calibration', {
            method: 'POST',
            body: calibrationData
        })
        .then(response => response.json())
        .then(result => {
            if (result.status === 'success') {
                window.settingsManager.showStatus(`Auto-calibration applied: ${calibrationsApplied.join(', ')}. Applied immediately.`, 'success');
                
                // Clear input fields
                document.getElementById('actualVoltage').value = '';
                document.getElementById('actualTemp').value = '';
                document.getElementById('actualPressure').value = '';
                if (document.getElementById('actualFuelLevel')) {
                    document.getElementById('actualFuelLevel').value = '';
                }
                
                // Refresh sensor data to show new calibrated values
                setTimeout(refreshSensorData, 1000);
            } else {
                window.settingsManager.showStatus(result.message || 'Auto-calibration failed', 'error');
            }
        })
        .catch(error => {
            console.error('Error applying auto-calibration:', error);
            window.settingsManager.showStatus('Network error during auto-calibration', 'error');
        });
    })
    .catch(error => {
        console.error('Error getting sensor data for calibration:', error);
        window.settingsManager.showStatus('Could not read sensor data for calibration', 'error');
    });
}

function applyCalibration() {
    if (!confirm('Apply new calibration constants? This will require a device restart to take effect.')) {
        return;
    }
    
    // Collect calibration data
    const calibrationData = new FormData();
    
    const batteryDivider = document.getElementById('batteryDivider').value;
    const tempScale = document.getElementById('tempScale').value;
    const pressureScale = document.getElementById('pressureScaleNew').value;
    
    if (batteryDivider) calibrationData.append('battery_divider', batteryDivider);
    if (tempScale) calibrationData.append('temp_scale', tempScale);
    if (pressureScale) calibrationData.append('pressure_scale', pressureScale);
    
    if (calibrationData.entries().next().done) {
        window.settingsManager.showStatus('No calibration values entered', 'error');
        return;
    }
    
    fetch('/api/calibration', {
        method: 'POST',
        body: calibrationData
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'received') {
            window.settingsManager.showStatus(data.message, 'success');
        } else {
            window.settingsManager.showStatus(data.message || 'Calibration update failed', 'error');
        }
    })
    .catch(error => {
        console.error('Error applying calibration:', error);
        window.settingsManager.showStatus('Network error applying calibration', 'error');
    });
}
