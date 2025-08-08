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
    // Backend returns seconds already for these fields
    if (this.settings.glowDuration != null) document.getElementById('glowPlugDuration').value = this.settings.glowDuration;
    if (this.settings.crankingTimeout != null) document.getElementById('crankingTimeout').value = this.settings.crankingTimeout;
    if (this.settings.cooldownDuration != null) document.getElementById('cooldownDuration').value = this.settings.cooldownDuration;
        
        // Alarm Thresholds
    if (this.settings.maxTemp != null) document.getElementById('maxTemp').value = this.settings.maxTemp;
    if (this.settings.minOilPressure != null) document.getElementById('minOilPressure').value = this.settings.minOilPressure;
    if (this.settings.minVoltage != null) document.getElementById('minVoltage').value = this.settings.minVoltage;
    if (this.settings.maxVoltage != null) document.getElementById('maxVoltage').value = this.settings.maxVoltage;
    if (this.settings.minHydPressure != null) {
        const el = document.getElementById('minHydPressure');
        if (el) el.value = this.settings.minHydPressure;
    }
        
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
            // Backend expects seconds for these fields
            glowDuration: parseInt(document.getElementById('glowPlugDuration').value),
            crankingTimeout: parseInt(document.getElementById('crankingTimeout').value),
            cooldownDuration: parseInt(document.getElementById('cooldownDuration').value),
            
            // Alarm Thresholds
            maxTemp: parseInt(document.getElementById('maxTemp').value),
            minOilPressure: parseInt(document.getElementById('minOilPressure').value),
            minHydPressure: parseInt(document.getElementById('minHydPressure').value || '0'),
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

            // Post each field individually as { key, value } to match backend API
            const posts = [];
            const postSetting = (key, value) => fetch('/api/settings', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ key, value: String(value) })
            });

            posts.push(postSetting('glowDuration', settings.glowDuration));
            posts.push(postSetting('crankingTimeout', settings.crankingTimeout));
            posts.push(postSetting('cooldownDuration', settings.cooldownDuration));
            posts.push(postSetting('maxTemp', settings.maxTemp));
            posts.push(postSetting('minOilPressure', settings.minOilPressure));
            posts.push(postSetting('minVoltage', settings.minVoltage));
            posts.push(postSetting('maxVoltage', settings.maxVoltage));
            if (!Number.isNaN(settings.minHydPressure)) posts.push(postSetting('minHydPressure', settings.minHydPressure));
            if (settings.wifiSSID) posts.push(postSetting('wifiSSID', settings.wifiSSID));
            if (settings.wifiPassword) posts.push(postSetting('wifiPassword', settings.wifiPassword));
            if (!Number.isNaN(settings.fuelLevelLowThreshold)) posts.push(postSetting('fuelLevelLowThreshold', settings.fuelLevelLowThreshold));

            const results = await Promise.all(posts.map(p => p.catch(e => e)));
            const allOk = results.every(r => r && r.ok);
            if (allOk) {
                this.showStatus('Settings saved successfully!', 'success');
                this.settings = settings;
            } else {
                this.showStatus('Some settings failed to save', 'error');
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
        // Basic validation (seconds for durations)
        if (settings.glowDuration < 5 || settings.glowDuration > 60) return false;
        if (settings.crankingTimeout < 5 || settings.crankingTimeout > 30) return false;
        if (settings.cooldownDuration < 60 || settings.cooldownDuration > 300) return false;
        if (settings.minVoltage < 10.0 || settings.minVoltage > 13.0) return false;
        if (settings.maxVoltage < 13.0 || settings.maxVoltage > 16.0) return false;
        if (settings.maxVoltage <= settings.minVoltage) return false;
        
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

    // Initial load
    loadAllSettings();
    updateWiFiStatus(); // Initial check

    // Set up a timer to periodically update WiFi status
    setInterval(updateWiFiStatus, 5000); // Update every 5 seconds
});

function updateWiFiStatus() {
    fetch('/status')
        .then(response => response.json())
        .then(data => {
            const homeStatusEl = document.getElementById('homeNetworkStatus');
            if (data.wifi_connected) {
                const ip = data.wifi_ip || 'connected';
                homeStatusEl.textContent = `Connected (${ip})`;
                homeStatusEl.classList.remove('error');
                homeStatusEl.classList.add('success');
            } else {
                homeStatusEl.textContent = 'Not Connected';
                homeStatusEl.classList.remove('success');
                homeStatusEl.classList.add('error');
            }

            const apStatusEl = document.getElementById('apStatus');
            apStatusEl.textContent = data.ap_ip ? `Active at ${data.ap_ip}` : 'AP Active';
        })
        .catch(error => {
            console.error('Error fetching WiFi status:', error);
            document.getElementById('homeNetworkStatus').textContent = 'Error';
            document.getElementById('apStatus').textContent = 'Error';
        });
}

function loadAllSettings() {
    fetch('/api/settings')
        .then(response => response.json())
        .then(data => {
            window.settingsManager.settings = data;
            window.settingsManager.populateForm();
        })
        .catch(error => {
            console.error('Error loading settings:', error);
            window.settingsManager.setDefaults();
        });
}

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
    const batteryRawEl = document.getElementById('batteryRaw');
    const batteryCalcEl = document.getElementById('batteryCalc');
    if (batteryRawEl) batteryRawEl.textContent = (data.battery_raw ?? '--');
    if (batteryCalcEl) batteryCalcEl.textContent = (data.battery_calculated != null ? data.battery_calculated.toFixed(2) : '--');
        
    const tempRawEl = document.getElementById('tempRaw');
    const tempCalcEl = document.getElementById('tempCalc');
    if (tempRawEl) tempRawEl.textContent = (data.temperature_raw ?? '--');
    if (tempCalcEl) tempCalcEl.textContent = (data.temperature_calculated != null ? data.temperature_calculated.toFixed(1) : '--');
        
    const pressureRawEl = document.getElementById('pressureRaw');
    const pressureCalcEl = document.getElementById('pressureCalc');
    if (pressureRawEl) pressureRawEl.textContent = (data.pressure_raw ?? '--');
    if (pressureCalcEl) pressureCalcEl.textContent = (data.pressure_calculated != null ? data.pressure_calculated.toFixed(1) : '--');
        
    const fuelRawEl = document.getElementById('fuelRaw');
    const fuelCalcEl = document.getElementById('fuelCalc');
    if (fuelRawEl) fuelRawEl.textContent = (data.fuel_raw ?? '--');
    if (fuelCalcEl) fuelCalcEl.textContent = (data.fuel_calculated != null ? data.fuel_calculated.toFixed(1) : '--');

    // Hydraulic pressure
    const hydRawEl = document.getElementById('hydRaw');
    const hydCalcEl = document.getElementById('hydCalc');
    if (hydRawEl) hydRawEl.textContent = (data.hydraulic_raw ?? '--');
    if (hydCalcEl) hydCalcEl.textContent = (data.hydraulic_calculated != null ? data.hydraulic_calculated.toFixed(0) : '--');
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
    'actualFuelLevel': 'Fuel Level',
    'actualHydPressure': 'Hydraulic Pressure'
    };
    return fieldNames[inputId] || inputId;
}

// Auto-calibrate a single sensor when actual value is entered - SIMPLIFIED VERSION
function autoCalibrateSingle(inputId, actualValue) {
    // Map input IDs to sensor types for C++ endpoint
    const sensorMap = {
        'actualVoltage': 'battery',
        'actualTemp': 'temperature', 
        'actualPressure': 'pressure',
    'actualFuelLevel': 'fuel',
    'actualHydPressure': 'hydraulic'
    };
    
    const sensorType = sensorMap[inputId];
    if (!sensorType) {
        window.settingsManager.showStatus('Unknown sensor type', 'error');
        return;
    }
    
    // Call C++ endpoint to handle all calibration logic
    const formData = new FormData();
    formData.append('sensor', sensorType);
    formData.append('actual_value', actualValue);
    
    fetch('/api/auto-calibrate', {
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(result => {
        if (result.status === 'success') {
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
        const response = await fetch('/status');
        const data = await response.json();

        // Update sleep mode status
        const sleepModeElement = document.getElementById('sleepStatus');
        if (sleepModeElement) {
            const enabled = !!data.sleep_mode_enabled;
            sleepModeElement.textContent = `Sleep mode: ${enabled ? 'Enabled' : 'Disabled'}`;
            sleepModeElement.classList.toggle('success', enabled);
            sleepModeElement.classList.toggle('error', !enabled);
        }

        // Update activity timer
        const activityElement = document.getElementById('activityTime');
        if (activityElement) {
            const since = Number(data.time_since_activity || 0);
            const minutes = Math.floor(since / 60);
            const seconds = Math.floor(since % 60);
            const sleepIn = Number(data.time_until_sleep || 0);
            const sleepMinutes = Math.floor(sleepIn / 60);
            const sleepSeconds = Math.floor(sleepIn % 60);
            activityElement.textContent = `Time since activity: ${minutes}m ${seconds}s (sleep in: ${sleepMinutes}m ${sleepSeconds}s)`;
        }
    } catch (error) {
        console.error('Error updating power status:', error);
    }
}

async function toggleSleepMode() {
    try {
        const response = await fetch('/control', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ action: 'toggle_sleep_mode' })
        });
        const result = await response.json();
        if (result.success) {
            window.settingsManager.showStatus(result.message || 'Sleep mode toggled', 'success');
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
            const response = await fetch('/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: 'sleep_now' })
            });
            const result = await response.json();
            if (result.success) {
                window.settingsManager.showStatus('Entering sleep mode...', 'info');
            } else {
                window.settingsManager.showStatus(result.message || 'Cannot sleep now', 'error');
            }
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
            if (response.ok) {
                alert('Factory reset initiated. Device will restart.');
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
    const apPasswordInput = document.getElementById('wifiPassword');
    const homePasswordInput = document.getElementById('homePassword');
    const checkbox = document.getElementById('showPassword');

    const type = checkbox.checked ? 'text' : 'password';
    if (apPasswordInput) apPasswordInput.type = type;
    if (homePasswordInput) homePasswordInput.type = type;
}

// Helper to trigger calibration explicitly from button clicks
// calibrateClick buttons removed; calibration is triggered by pressing Enter in actual value fields.
