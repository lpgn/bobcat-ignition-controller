/**
 * Settings Management for Bobcat Ignition Controller
 * Fixed version with working auto-calibration
 */

class SettingsManager {
    constructor() {
        this.settings = {};
        this.autoSaveTimeout = null;
        this.init();
    }

    init() {
        this.loadSettings();
        this.setupEventListeners();
    }

    setDefaults() {
        // Engine Parameters
        document.getElementById('glowPlugDuration').value = 20;
        document.getElementById('crankingTimeout').value = 10;
        document.getElementById('cooldownDuration').value = 120;
        
        // Alarm Thresholds
        document.getElementById('maxTemp').value = 104;
        document.getElementById('minOilPressure').value = 69;
        document.getElementById('minVoltage').value = 11.0;
        document.getElementById('maxVoltage').value = 15.0;
        
        // Sensor Calibration (fuel low threshold only, calibration handled automatically)
        document.getElementById('fuelLowThreshold').value = 15;
    }

    populateForm() {
        if (!this.settings) return;
        
        // Engine Parameters
        if (this.settings.glowPlugDuration) document.getElementById('glowPlugDuration').value = this.settings.glowPlugDuration / 1000;
        if (this.settings.crankingTimeout) document.getElementById('crankingTimeout').value = this.settings.crankingTimeout / 1000;
        if (this.settings.cooldownDuration) document.getElementById('cooldownDuration').value = this.settings.cooldownDuration / 1000;
        
        // Alarm Thresholds
        if (this.settings.maxCoolantTemp) document.getElementById('maxTemp').value = this.settings.maxCoolantTemp;
        if (this.settings.minOilPressure) document.getElementById('minOilPressure').value = this.settings.minOilPressure;
        if (this.settings.minBatteryVoltage) document.getElementById('minVoltage').value = this.settings.minBatteryVoltage;
        if (this.settings.maxBatteryVoltage) document.getElementById('maxVoltage').value = this.settings.maxBatteryVoltage;
        
        // WiFi Configuration (SSID only, never populate password for security)
        if (this.settings.wifiSSID) document.getElementById('wifiSSID').value = this.settings.wifiSSID;
        // Never populate password for security
        
        // Sensor Calibration (only fuel threshold is user-configurable)
        if (this.settings.fuelLevelLowThreshold !== undefined) document.getElementById('fuelLowThreshold').value = this.settings.fuelLevelLowThreshold;
    }

    setupEventListeners() {
        // Form validation
        const numberInputs = document.querySelectorAll('input[type="number"]');
        numberInputs.forEach(input => {
            input.addEventListener('input', () => this.validateInput(input));
        });
    }

    validateInput(input) {
        const value = parseFloat(input.value);
        const min = parseFloat(input.min);
        const max = parseFloat(input.max);
        
        if (value < min || value > max) {
            input.style.borderColor = '#e74c3c';
            return false;
        } else {
            input.style.borderColor = '#555';
            return true;
        }
    }

    collectFormData() {
        return {
            // Engine Parameters (convert seconds to milliseconds for internal use)
            glowPlugDuration: parseInt(document.getElementById('glowPlugDuration').value) * 1000,
            crankingTimeout: parseInt(document.getElementById('crankingTimeout').value) * 1000,
            cooldownDuration: parseInt(document.getElementById('cooldownDuration').value) * 1000,
            
            // Alarm Thresholds
            maxCoolantTemp: parseInt(document.getElementById('maxTemp').value),
            minOilPressure: parseInt(document.getElementById('minOilPressure').value),
            minBatteryVoltage: parseFloat(document.getElementById('minVoltage').value),
            maxBatteryVoltage: parseFloat(document.getElementById('maxVoltage').value),
            
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
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(settings),
            });
            
            const result = await response.json();
            
            if (result.status === 'success') {
                this.showStatus('Settings saved successfully!', 'success');
                this.settings = settings;
            } else {
                this.showStatus(result.message || 'Save failed', 'error');
            }
        } catch (error) {
            console.error('Error saving settings:', error);
            this.showStatus('Network error while saving settings', 'error');
        }
    }

    async loadSettings() {
        try {
            const response = await fetch('/api/settings');
            if (response.ok) {
                this.settings = await response.json();
                this.populateForm();
            } else {
                console.warn('Could not load settings, using defaults');
                this.setDefaults();
            }
        } catch (error) {
            console.error('Error loading settings:', error);
            this.setDefaults();
        }
    }

    validateSettings(settings) {
        // Basic validation
        if (settings.glowPlugDuration < 5000 || settings.glowPlugDuration > 60000) return false;
        if (settings.crankingTimeout < 5000 || settings.crankingTimeout > 30000) return false;
        if (settings.minBatteryVoltage < 10.0 || settings.minBatteryVoltage > 13.0) return false;
        if (settings.maxBatteryVoltage < 13.0 || settings.maxBatteryVoltage > 16.0) return false;
        
        return true;
    }

    showStatus(message, type = 'info') {
        // Create or get status element
        let statusElement = document.getElementById('settingsStatus');
        if (!statusElement) {
            statusElement = document.createElement('div');
            statusElement.id = 'settingsStatus';
            statusElement.style.cssText = `
                position: fixed;
                top: 20px;
                right: 20px;
                padding: 12px 20px;
                border-radius: 8px;
                font-weight: 500;
                z-index: 1000;
                max-width: 300px;
                opacity: 0;
                transition: opacity 0.3s ease;
            `;
            document.body.appendChild(statusElement);
        }
        
        // Set colors based on type
        const colors = {
            success: { bg: '#2ecc71', text: '#fff' },
            error: { bg: '#e74c3c', text: '#fff' },
            info: { bg: '#3498db', text: '#fff' }
        };
        
        const color = colors[type] || colors.info;
        statusElement.style.backgroundColor = color.bg;
        statusElement.style.color = color.text;
        statusElement.textContent = message;
        
        // Show and auto-hide
        statusElement.style.opacity = '1';
        setTimeout(() => {
            statusElement.style.opacity = '0';
        }, 3000);
    }

    async resetToDefaults() {
        if (confirm('Reset all settings to defaults? This cannot be undone.')) {
            try {
                const response = await fetch('/api/reset', { method: 'POST' });
                const result = await response.json();
                
                if (result.status === 'success') {
                    this.showStatus('Settings reset to defaults', 'success');
                    this.setDefaults();
                } else {
                    this.showStatus('Reset failed', 'error');
                }
            } catch (error) {
                console.error('Error resetting settings:', error);
                this.showStatus('Network error during reset', 'error');
            }
        }
    }

    exportSettings() {
        const settings = this.collectFormData();
        const dataStr = JSON.stringify(settings, null, 2);
        const dataBlob = new Blob([dataStr], {type: 'application/json'});
        
        const link = document.createElement('a');
        link.href = URL.createObjectURL(dataBlob);
        link.download = 'bobcat-settings.json';
        link.click();
        
        this.showStatus('Settings exported successfully', 'success');
    }
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
        refreshSensorData();
        if (sensorRefreshInterval) clearInterval(sensorRefreshInterval);
        sensorRefreshInterval = setInterval(refreshSensorData, 2000);
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
        // Update sensor readings in table
        document.getElementById('batteryRaw').textContent = data.battery_raw || '--';
        document.getElementById('batteryCalc').textContent = (data.battery_calculated ? data.battery_calculated.toFixed(2) + ' V' : '-- V');
        
        document.getElementById('tempRaw').textContent = data.temperature_raw || '--';
        document.getElementById('tempCalc').textContent = (data.temperature_calculated ? data.temperature_calculated.toFixed(1) + ' °C' : '-- °C');
        
        document.getElementById('pressureRaw').textContent = data.pressure_raw || '--';
        document.getElementById('pressureCalc').textContent = (data.pressure_calculated ? data.pressure_calculated.toFixed(1) + ' kPa' : '-- kPa');
        
        document.getElementById('fuelRaw').textContent = data.fuel_raw || '--';
        document.getElementById('fuelCalc').textContent = (data.fuel_calculated ? data.fuel_calculated.toFixed(1) + ' %' : '-- %');
    })
    .catch(error => {
        console.error('Error fetching sensor data:', error);
        // Set all to error state
        document.getElementById('batteryRaw').textContent = 'ERR';
        document.getElementById('tempRaw').textContent = 'ERR';
        document.getElementById('pressureRaw').textContent = 'ERR';
        document.getElementById('fuelRaw').textContent = 'ERR';
    });
}

// Handle Enter key press
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

// Auto-calibrate a single sensor when actual value is entered - FIXED VERSION
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

// Simplified auto-calibrate function (no longer needed for individual calibration)
function autoCalibrate() {
    // Simple auto-save functionality - calibration is handled automatically by backend
    window.settingsManager.showStatus('Calibration happens automatically when you enter actual values', 'info');
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

// Show auto-save status briefly
function showAutoSaveStatus() {
    const statusElement = document.getElementById('autoSaveStatus');
    if (statusElement) {
        statusElement.style.display = 'inline';
        statusElement.style.opacity = '1';
        
        setTimeout(() => {
            statusElement.style.opacity = '0';
            setTimeout(() => {
                statusElement.style.display = 'none';
            }, 300);
        }, 1500);
    }
}

// Power management functions
async function updatePowerStatus() {
    try {
        const response = await fetch('/api/power-status');
        const data = await response.json();
        
        // Update sleep mode status
        const sleepModeElement = document.querySelector('[id*="sleepMode"]');
        if (sleepModeElement) {
            sleepModeElement.textContent = `Sleep mode: ${data.sleepEnabled ? 'Enabled' : 'Disabled'}`;
        }
        
        // Update activity timer
        const activityElement = document.querySelector('[id*="activityTime"]');
        if (activityElement) {
            const minutes = Math.floor(data.timeSinceActivity / 60);
            const seconds = data.timeSinceActivity % 60;
            const sleepIn = Math.max(0, 1800 - data.timeSinceActivity); // 30 minutes
            const sleepMinutes = Math.floor(sleepIn / 60);
            const sleepSeconds = sleepIn % 60;
            activityElement.textContent = `Time since activity: ${minutes}m ${seconds}s (sleep in: ${sleepMinutes}m ${sleepSeconds}s)`;
        }
    } catch (error) {
        console.error('Error updating power status:', error);
    }
}

async function toggleSleepMode() {
    try {
        const response = await fetch('/api/toggle-sleep', { method: 'POST' });
        const result = await response.json();
        
        if (result.status === 'success') {
            window.settingsManager.showStatus(`Sleep mode ${result.enabled ? 'enabled' : 'disabled'}`, 'success');
            updatePowerStatus();
        } else {
            window.settingsManager.showStatus('Failed to toggle sleep mode', 'error');
        }
    } catch (error) {
        console.error('Error toggling sleep mode:', error);
        window.settingsManager.showStatus('Network error', 'error');
    }
}

async function sleepNow() {
    if (confirm('Put device to sleep now? It will wake up when the main control page is accessed.')) {
        try {
            await fetch('/api/sleep-now', { method: 'POST' });
            window.settingsManager.showStatus('Entering sleep mode...', 'info');
        } catch (error) {
            console.error('Error entering sleep mode:', error);
            window.settingsManager.showStatus('Network error', 'error');
        }
    }
}

// Utility functions
function resetCalibration() {
    if (confirm('Reset all sensor calibrations to defaults? This will require re-calibrating all sensors.')) {
        fetch('/api/reset-calibration', { method: 'POST' })
        .then(response => response.json())
        .then(result => {
            if (result.status === 'success') {
                window.settingsManager.showStatus('Calibration reset successfully', 'success');
                setTimeout(refreshSensorData, 1000);
            } else {
                window.settingsManager.showStatus('Reset failed', 'error');
            }
        })
        .catch(error => {
            console.error('Error resetting calibration:', error);
            window.settingsManager.showStatus('Network error', 'error');
        });
    }
}

function exportSettings() {
    if (window.settingsManager) {
        window.settingsManager.exportSettings();
    }
}

function startOTAUpdate() {
    if (confirm('Start over-the-air firmware update? Device will restart after completion.')) {
        window.open('/update', '_blank');
    }
}

// Factory reset function
async function factoryReset() {
    const confirmText = "FACTORY RESET";
    const userInput = prompt(`This will erase ALL settings and return to factory defaults.\n\nType "${confirmText}" to confirm:`);
    
    if (userInput === confirmText) {
        try {
            const response = await fetch('/api/factory-reset', { method: 'POST' });
            const result = await response.json();
            
            if (result.status === 'success') {
                alert('Factory reset complete. Device will restart.');
                window.location.reload();
            } else {
                window.settingsManager.showStatus('Factory reset failed', 'error');
            }
        } catch (error) {
            console.error('Error during factory reset:', error);
            window.settingsManager.showStatus('Network error during factory reset', 'error');
        }
    }
}

// Toggle password visibility
function togglePasswordVisibility() {
    const passwordInput = document.getElementById('wifiPassword');
    const checkbox = document.getElementById('showPassword');
    
    if (checkbox.checked) {
        passwordInput.type = 'text';
    } else {
        passwordInput.type = 'password';
    }
}
