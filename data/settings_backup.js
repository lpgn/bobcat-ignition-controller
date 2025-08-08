/**
 * Settings Interface JavaScript for Bobcat Ignition Controller
 * Handles configuration management, WiFi setup, and OTA updates
 * Now with auto-save functionality and unified sensor display
 */

class SettingsManager {
    constructor() {
        this.settings = {};
        this.autoSaveTimeout = null;
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
        
        // Sensor Calibration (fuel low threshold only, calibration handled automatically)
        document.getElementById('fuelLowThreshold').value = 15;
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
        
        // Sensor Calibration (only fuel threshold is user-configurable)
        if (this.settings.fuelLevelLowThreshold !== undefined) document.getElementById('fuelLowThreshold').value = this.settings.fuelLevelLowThreshold;
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
            
            // Sensor Calibration (only fuel threshold, others are auto-calculated)
            fuelLevelLowThreshold: parseInt(document.getElementById('fuelLowThreshold').value)
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

function updateSensorStatus(elementId, rawValue, status) {
    const element = document.getElementById(elementId);
    
    // Reset styles
    element.style.color = '';
    element.style.fontWeight = '';
    
    // Apply status-based styling
    if (status === 'BROKEN') {
        element.style.color = '#ff6b6b';
        element.style.fontWeight = 'bold';
        element.parentElement.title = 'Sensor appears to be disconnected or broken';
    } else if (status === 'CHECK') {
        element.style.color = '#ffa500';
        element.parentElement.title = 'Sensor reading may be incorrect - check connections';
    } else {
        element.style.color = '#2ecc71';
        element.parentElement.title = 'Sensor operating normally';
    }
}

function refreshSensorData() {
    if (!autoRefreshEnabled) return;
    
    fetch('/api/raw-sensors')
    .then(response => response.json())
    .then(data => {
        // Update raw ADC readings with status indicators
        document.getElementById('batteryRaw').textContent = data.battery_raw || '--';
        document.getElementById('tempRaw').textContent = data.temperature_raw || '--';
        document.getElementById('pressureRaw').textContent = data.pressure_raw || '--';
        document.getElementById('fuelRaw').textContent = data.fuel_raw || '--';
        
        // Add status indicators for broken sensors
        updateSensorStatus('batteryRaw', data.battery_raw, data.battery_status);
        updateSensorStatus('tempRaw', data.temperature_raw, data.temperature_status);
        updateSensorStatus('pressureRaw', data.pressure_raw, data.pressure_status);
        updateSensorStatus('fuelRaw', data.fuel_raw, data.fuel_status);
        
        // Show pressure sensor diagnostic if available
        if (data.pressure_diagnostic) {
            const pressureCell = document.getElementById('pressureRaw').parentElement;
            pressureCell.title = data.pressure_diagnostic;
            
            // Add warning icon for broken pressure sensor
            if (data.pressure_status === 'BROKEN') {
                document.getElementById('pressureRaw').textContent += ' ⚠️';
                document.getElementById('pressureRaw').style.color = '#ff6b6b';
            }
        }
        
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
    // Simple auto-save functionality - calibration is handled automatically by backend
    window.settingsManager.showStatus('Calibration is handled automatically when you enter actual values', 'info');
    autoSave();
}
// Auto-save functionality
function autoSave() {
    // Clear existing timeout
    if (window.settingsManager && window.settingsManager.autoSaveTimeout) {
        clearTimeout(window.settingsManager.autoSaveTimeout);
    }
    
    // Set new timeout for 1 second delay
    const timeout = setTimeout(() => {
        if (window.settingsManager) {
            window.settingsManager.saveSettings();
            showAutoSaveStatus();
        }
    }, 1000);
    
    if (window.settingsManager) {
        window.settingsManager.autoSaveTimeout = timeout;
    }
}

// Handle Enter key press to trigger save and auto-calibration for actual values
function handleEnterKey(event) {
    if (event.key === 'Enter') {
        event.preventDefault();
        
        // Check if this is an "actual value" input field
        const inputId = event.target.id;
        const isActualValueField = inputId.startsWith('actual');
        
        if (isActualValueField && event.target.value.trim() !== '') {
            // For actual value fields, trigger auto-calibration
            if (confirm(`Auto-calibrate ${getFieldDisplayName(inputId)} with actual value ${event.target.value}?`)) {
                autoCalibrateSingle(inputId, event.target.value);
            }
        } else {
            // For other fields, just auto-save
            autoSave();
        }
        
        event.target.blur(); // Remove focus from input
    }
}

// Get user-friendly field name for confirmation
function getFieldDisplayName(inputId) {
    const fieldNames = {
        'actualVoltage': 'Battery Voltage',
        'actualTemp': 'Engine Temperature', 
        'actualPressure': 'Oil Pressure',
        'actualFuelLevel': 'Fuel Level'
    };
    return fieldNames[inputId] || inputId;
}

// Auto-calibrate a single sensor when actual value is entered
function autoCalibrateSingle(inputId, actualValue) {
    // Get current sensor data first
    fetch('/api/raw-sensors')
    .then(response => response.json())
    .then(sensorData => {
        const calibrationData = new FormData();
        let calibrationApplied = false;
        
        // Determine which sensor to calibrate based on input ID
        switch(inputId) {
            case 'actualVoltage':
                if (sensorData.battery_raw && actualValue) {
                    // Battery voltage calibration: new_divider = actual_voltage / raw_adc_value
                    const newDivider = parseFloat(actualValue) / sensorData.battery_raw;
                    calibrationData.append('battery_divider', newDivider.toFixed(6));
                    calibrationApplied = true;
                }
                break;
                
            case 'actualTemp':
                if (sensorData.temperature_raw && actualValue) {
                    // Temperature calibration: scale = (150 - actual_temp) / raw_adc
                    const baseTemp = 150.0; // Reference temperature for inverted NTC
                    const newScale = (baseTemp - parseFloat(actualValue)) / sensorData.temperature_raw;
                    calibrationData.append('temp_scale', newScale.toFixed(6));
                    calibrationApplied = true;
                }
                break;
                
            case 'actualPressure':
                if (sensorData.pressure_raw && actualValue) {
                    // Pressure calibration: scale = actual_pressure / raw_adc
                    const newScale = parseFloat(actualValue) / sensorData.pressure_raw;
                    calibrationData.append('pressure_scale', newScale.toFixed(6));
                    calibrationApplied = true;
                }
                break;
                
            case 'actualFuelLevel':
                if (sensorData.fuel_raw && actualValue) {
                    const fuelPercent = parseFloat(actualValue);
                    if (fuelPercent <= 10) {
                        // Low fuel reading - set as empty
                        calibrationData.append('fuel_empty', sensorData.fuel_raw.toString());
                        calibrationApplied = true;
                    } else if (fuelPercent >= 90) {
                        // High fuel reading - set as full
                        calibrationData.append('fuel_full', sensorData.fuel_raw.toString());
                        calibrationApplied = true;
                    } else {
                        // Mid-range - interpolate between current empty/full
                        const currentEmpty = sensorData.fuel_empty || 200;
                        const currentFull = sensorData.fuel_full || 3800;
                        
                        // Calculate what the ADC should be for this percentage
                        const expectedADC = currentEmpty + (fuelPercent / 100.0) * (currentFull - currentEmpty);
                        const scaleFactor = expectedADC / sensorData.fuel_raw;
                        
                        // Scale both empty and full proportionally
                        const newEmpty = Math.round(currentEmpty * scaleFactor);
                        const newFull = Math.round(currentFull * scaleFactor);
                        
                        calibrationData.append('fuel_empty', newEmpty.toString());
                        calibrationData.append('fuel_full', newFull.toString());
                        calibrationApplied = true;
                    }
                }
                break;
        }
        
        if (!calibrationApplied) {
            window.settingsManager.showStatus('Unable to calibrate - no sensor data available', 'error');
            return;
        }
        
        // Apply calibration via API
        fetch('/api/calibration', {
            method: 'POST',
            body: calibrationData
        })
        .then(response => response.json())
        .then(result => {
            if (result.status === 'success' || result.status === 'received') {
                window.settingsManager.showStatus(`${getFieldDisplayName(inputId)} calibrated successfully!`, 'success');
                
                // Clear the input field
                document.getElementById(inputId).value = '';
                
                // Refresh sensor data to show new calibrated values
                setTimeout(refreshSensorData, 1000);
                
                // Auto-save settings
                autoSave();
            } else {
                window.settingsManager.showStatus(result.message || 'Calibration failed', 'error');
            }
        })
        .catch(error => {
            console.error('Error applying calibration:', error);
            window.settingsManager.showStatus('Network error during calibration', 'error');
        });
    })
    .catch(error => {
        console.error('Error getting sensor data:', error);
        window.settingsManager.showStatus('Could not read sensor data for calibration', 'error');
    });
}
            case 'actualPressure':
                if (sensorData.pressure_raw && actualValue) {
                    const newScale = parseFloat(actualValue) / sensorData.pressure_raw;
                    calibrationData.append('pressure_scale', newScale.toFixed(3));
                    
                    // Update the calibration display
                    document.getElementById('pressureScale').value = newScale.toFixed(3);
                }
                break;
                
            case 'actualFuelLevel':
                if (sensorData.fuel_raw && actualValue) {
                    const fuelEmpty = parseInt(document.getElementById('fuelEmpty').value) || 200;
                    const currentPercent = parseFloat(actualValue);
                    const newFuelFull = fuelEmpty + ((sensorData.fuel_raw - fuelEmpty) * 100 / currentPercent);
                    
                    calibrationData.append('fuel_full', Math.round(newFuelFull));
                    
                    // Update the calibration display
                    document.getElementById('fuelFull').value = Math.round(newFuelFull);
                }
                break;
        }
        
        // Apply the calibration if we have data to send
        if (!calibrationData.entries().next().done) {
            return fetch('/api/calibration', {
                method: 'POST',
                body: calibrationData
            });
        } else {
            throw new Error('No calibration data to apply');
        }
    })
    .then(response => response.json())
    .then(result => {
        if (result.status === 'received') {
            // Clear the actual value input after successful calibration
            document.getElementById(inputId).value = '';
            
            // Show success message
            if (window.settingsManager) {
                window.settingsManager.showStatus(`${getFieldDisplayName(inputId)} calibrated successfully`, 'success');
            }
            
            // Auto-save the settings
            autoSave();
            
            // Refresh sensor data to show new calibrated values
            setTimeout(refreshSensorData, 1000);
        } else {
            if (window.settingsManager) {
                window.settingsManager.showStatus(result.message || 'Calibration failed', 'error');
            }
        }
    })
    .catch(error => {
        console.error('Error during single calibration:', error);
        if (window.settingsManager) {
            window.settingsManager.showStatus('Error during calibration: ' + error.message, 'error');
        }
    });
}

// Show auto-save status briefly
function showAutoSaveStatus() {
    const statusElement = document.getElementById('autoSaveStatus');
    if (statusElement) {
        statusElement.style.display = 'block';
        statusElement.innerHTML = '✓ Settings auto-saved';
        statusElement.style.color = '#2ecc71';
        
        // Hide after 2 seconds
        setTimeout(() => {
            statusElement.style.display = 'none';
        }, 2000);
    }
}

// Reset calibration to defaults
function resetCalibration() {
    if (!confirm('Reset all calibration values to factory defaults?')) {
        return;
    }
    
    // Reset to default values
    document.getElementById('tempOffset').value = -40.0;
    document.getElementById('pressureScale').value = 0.17;
    document.getElementById('fuelEmpty').value = 200;
    document.getElementById('fuelFull').value = 3800;
    document.getElementById('minVoltage').value = 11.0;
    document.getElementById('maxVoltage').value = 15.0;
    document.getElementById('maxTemp').value = 104;
    document.getElementById('minOilPressure').value = 69;
    
    // Clear actual value inputs
    document.getElementById('actualVoltage').value = '';
    document.getElementById('actualTemp').value = '';
    document.getElementById('actualPressure').value = '';
    document.getElementById('actualFuelLevel').value = '';
    
    // Auto-save the reset values
    autoSave();
    
    if (window.settingsManager) {
        window.settingsManager.showStatus('Calibration reset to factory defaults', 'success');
    }
}

// Export settings as JSON
function exportSettings() {
    if (window.settingsManager && window.settingsManager.settings) {
        const dataStr = JSON.stringify(window.settingsManager.settings, null, 2);
        const dataBlob = new Blob([dataStr], {type: 'application/json'});
        
        const link = document.createElement('a');
        link.href = URL.createObjectURL(dataBlob);
        link.download = 'bobcat-743-settings.json';
        link.click();
        
        window.settingsManager.showStatus('Settings exported successfully', 'success');
    } else {
        if (window.settingsManager) {
            window.settingsManager.showStatus('No settings to export', 'error');
        }
    }
}
