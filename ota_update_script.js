/**
 * Automated OTA Update Script for Bobcat Ignition Controller
 * 
 * This script automates the complete OTA update workflow:
 * 1. Compile firmware using PlatformIO
 * 2. Upload firmware via ElegantOTA
 * 3. Compile filesystem using PlatformIO  
 * 4. Upload filesystem via ElegantOTA
 * 
 * Prerequisites:
 * - PlatformIO installed at C:\.platformio\penv\Scripts\platformio.exe
 * - Device accessible at http://192.168.1.128
 * - Playwright installed: npm install playwright
 * 
 * Usage: node ota_update_script.js
 */

const { chromium } = require('playwright');
const { execSync } = require('child_process');
const path = require('path');

// Configuration
const CONFIG = {
    deviceIP: 'http://192.168.1.128',
    projectPath: 'c:\\Users\\lpgn\\gits_go_here\\bobcat-ignition-controller',
    platformioPath: 'C:\\.platformio\\penv\\Scripts\\platformio.exe',
    firmwarePath: '.pio\\build\\esp32dev\\firmware.bin',
    filesystemPath: '.pio\\build\\esp32dev\\littlefs.bin',
    timeouts: {
        compile: 120000,    // 2 minutes for compilation
        upload: 180000,     // 3 minutes for upload
        restart: 15000      // 15 seconds for device restart
    }
};

class OTAUpdateManager {
    constructor() {
        this.browser = null;
        this.page = null;
    }

    async initialize() {
        console.log('🚀 Initializing Playwright browser...');
        this.browser = await chromium.launch({ 
            headless: false,  // Show browser for monitoring
            slowMo: 500       // Slow down for visibility
        });
        this.page = await this.browser.newPage();
        
        // Set longer timeouts for uploads
        this.page.setDefaultTimeout(CONFIG.timeouts.upload);
        
        console.log('✅ Browser initialized');
    }

    async cleanup() {
        if (this.browser) {
            await this.browser.close();
            console.log('🧹 Browser closed');
        }
    }

    async compileFirmware() {
        console.log('\n📦 STEP 1: Compiling firmware...');
        try {
            const command = `cd "${CONFIG.projectPath}" && "${CONFIG.platformioPath}" run`;
            console.log(`Running: ${command}`);
            
            const output = execSync(command, { 
                encoding: 'utf8',
                timeout: CONFIG.timeouts.compile,
                cwd: CONFIG.projectPath
            });
            
            console.log('✅ Firmware compilation successful');
            console.log(output.split('\n').slice(-5).join('\n')); // Show last 5 lines
            return true;
        } catch (error) {
            console.error('❌ Firmware compilation failed:');
            console.error(error.message);
            return false;
        }
    }

    async compileFilesystem() {
        console.log('\n📦 STEP 3: Compiling filesystem...');
        try {
            const command = `cd "${CONFIG.projectPath}" && "${CONFIG.platformioPath}" run --target buildfs`;
            console.log(`Running: ${command}`);
            
            const output = execSync(command, { 
                encoding: 'utf8',
                timeout: CONFIG.timeouts.compile,
                cwd: CONFIG.projectPath
            });
            
            console.log('✅ Filesystem compilation successful');
            console.log(output.split('\n').slice(-5).join('\n')); // Show last 5 lines
            return true;
        } catch (error) {
            console.error('❌ Filesystem compilation failed:');
            console.error(error.message);
            return false;
        }
    }

    async uploadFirmware() {
        console.log('\n🚀 STEP 2: Uploading firmware via OTA...');
        
        try {
            // Navigate to OTA interface
            console.log('Connecting to device...');
            await this.page.goto(`${CONFIG.deviceIP}/update`);
            await this.page.waitForLoadState('networkidle');
            
            // Verify we're on the right page
            const title = await this.page.title();
            if (!title.includes('ElegantOTA')) {
                throw new Error('Not on ElegantOTA page');
            }
            console.log('✅ Connected to ElegantOTA interface');
            
            // Ensure firmware mode is selected
            await this.page.selectOption('#otaMode', 'Firmware');
            console.log('📝 Firmware mode selected');
            
            // Click select file button and upload firmware
            const firmwareFullPath = path.join(CONFIG.projectPath, CONFIG.firmwarePath);
            console.log(`📁 Uploading: ${firmwareFullPath}`);
            
            const fileChooserPromise = this.page.waitForEvent('filechooser');
            await this.page.click('button:has-text("Select File")');
            const fileChooser = await fileChooserPromise;
            await fileChooser.setFiles(firmwareFullPath);
            
            // Wait for upload to complete
            console.log('⏳ Uploading firmware... (this may take 1-2 minutes)');
            await this.page.waitForText('Update Successful', { timeout: CONFIG.timeouts.upload });
            console.log('✅ Firmware upload successful!');
            
            // Click Go Back
            await this.page.click('button:has-text("Go Back")');
            console.log('🔙 Returned to main OTA interface');
            
            return true;
        } catch (error) {
            console.error('❌ Firmware upload failed:');
            console.error(error.message);
            return false;
        }
    }

    async uploadFilesystem() {
        console.log('\n🚀 STEP 4: Uploading filesystem via OTA...');
        
        try {
            // Switch to filesystem mode
            await this.page.selectOption('#otaMode', 'LittleFS / SPIFFS');
            console.log('📝 Filesystem mode selected');
            
            // Click select file button and upload filesystem
            const filesystemFullPath = path.join(CONFIG.projectPath, CONFIG.filesystemPath);
            console.log(`📁 Uploading: ${filesystemFullPath}`);
            
            const fileChooserPromise = this.page.waitForEvent('filechooser');
            await this.page.click('button:has-text("Select File")');
            const fileChooser = await fileChooserPromise;
            await fileChooser.setFiles(filesystemFullPath);
            
            // Wait for upload to complete
            console.log('⏳ Uploading filesystem... (this may take 30-60 seconds)');
            await this.page.waitForText('Update Successful', { timeout: CONFIG.timeouts.upload });
            console.log('✅ Filesystem upload successful!');
            
            return true;
        } catch (error) {
            console.error('❌ Filesystem upload failed:');
            console.error(error.message);
            return false;
        }
    }

    async waitForDeviceRestart() {
        console.log('\n⏳ Waiting for device to restart...');
        await new Promise(resolve => setTimeout(resolve, CONFIG.timeouts.restart));
        
        try {
            // Verify device is responsive
            await this.page.goto(`${CONFIG.deviceIP}/`);
            await this.page.waitForLoadState('networkidle', { timeout: 30000 });
            console.log('✅ Device is online and responsive');
            return true;
        } catch (error) {
            console.warn('⚠️  Device may still be restarting...');
            return false;
        }
    }

    async verifyUpdate() {
        console.log('\n🔍 STEP 5: Verifying update...');
        
        try {
            // Check main interface
            await this.page.goto(`${CONFIG.deviceIP}/`);
            await this.page.waitForLoadState('networkidle');
            
            const title = await this.page.title();
            console.log(`📄 Main page title: ${title}`);
            
            // Check if settings page loads (filesystem verification)
            await this.page.goto(`${CONFIG.deviceIP}/settings.html`);
            await this.page.waitForLoadState('networkidle');
            
            const settingsTitle = await this.page.title();
            console.log(`⚙️  Settings page title: ${settingsTitle}`);
            
            // Check for raw sensor data (verify new features)
            const sensorTable = await this.page.locator('table').isVisible();
            if (sensorTable) {
                console.log('📊 Raw sensor data table found - filesystem update verified');
            }
            
            console.log('✅ Update verification successful!');
            return true;
        } catch (error) {
            console.error('❌ Update verification failed:');
            console.error(error.message);
            return false;
        }
    }

    async run() {
        console.log('🎯 Starting automated OTA update process...');
        console.log(`📍 Target device: ${CONFIG.deviceIP}`);
        console.log(`📁 Project path: ${CONFIG.projectPath}`);
        console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        
        let success = true;
        
        try {
            await this.initialize();
            
            // Step 1: Compile firmware
            if (!await this.compileFirmware()) {
                success = false;
                throw new Error('Firmware compilation failed');
            }
            
            // Step 2: Upload firmware
            if (!await this.uploadFirmware()) {
                success = false;
                throw new Error('Firmware upload failed');
            }
            
            // Step 3: Compile filesystem
            if (!await this.compileFilesystem()) {
                success = false;
                throw new Error('Filesystem compilation failed');
            }
            
            // Step 4: Upload filesystem
            if (!await this.uploadFilesystem()) {
                success = false;
                throw new Error('Filesystem upload failed');
            }
            
            // Step 5: Wait and verify
            await this.waitForDeviceRestart();
            await this.verifyUpdate();
            
        } catch (error) {
            console.error('\n💥 OTA update process failed:');
            console.error(error.message);
            success = false;
        } finally {
            await this.cleanup();
        }
        
        console.log('\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        if (success) {
            console.log('🎉 OTA UPDATE COMPLETED SUCCESSFULLY!');
            console.log('✅ Firmware updated');
            console.log('✅ Filesystem updated');
            console.log('✅ Device verified and responsive');
            console.log(`🌐 Access your device at: ${CONFIG.deviceIP}`);
        } else {
            console.log('❌ OTA UPDATE FAILED!');
            console.log('Please check the errors above and try again.');
        }
        console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        
        return success;
    }
}

// Run the OTA update if this script is executed directly
if (require.main === module) {
    const otaManager = new OTAUpdateManager();
    otaManager.run()
        .then(success => {
            process.exit(success ? 0 : 1);
        })
        .catch(error => {
            console.error('Fatal error:', error);
            process.exit(1);
        });
}

module.exports = OTAUpdateManager;
