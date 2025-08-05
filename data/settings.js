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
        this.showStatus('OTA firmware update feature is not implemented yet.', 'info');
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

// Initialize settings manager when page loads
document.addEventListener('DOMContentLoaded', () => {
    window.settingsManager = new SettingsManager();
});
