# Bobcat Ignition Controller - Automated OTA Update Script
# PowerShell version for Windows users
# 
# This script automates the complete OTA update workflow:
# 1. Compile firmware using PlatformIO
# 2. Upload firmware via ElegantOTA (manual browser interaction)
# 3. Compile filesystem using PlatformIO  
# 4. Upload filesystem via ElegantOTA (manual browser interaction)
#
# Usage: .\ota_update.ps1

param(
    [string]$DeviceIP = "192.168.1.128",
    [string]$ProjectPath = "c:\Users\lpgn\gits_go_here\bobcat-ignition-controller",
    [string]$PlatformIOPath = "C:\.platformio\penv\Scripts\platformio.exe"
)

# Configuration
$CONFIG = @{
    DeviceIP = "http://$DeviceIP"
    ProjectPath = $ProjectPath
    PlatformIOPath = $PlatformIOPath
    FirmwarePath = ".pio\build\esp32dev\firmware.bin"
    FilesystemPath = ".pio\build\esp32dev\littlefs.bin"
}

Write-Host "🎯 Bobcat Ignition Controller - Automated OTA Update" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "📍 Target device: $($CONFIG.DeviceIP)" -ForegroundColor Yellow
Write-Host "📁 Project path: $($CONFIG.ProjectPath)" -ForegroundColor Yellow
Write-Host "⚙️  PlatformIO: $($CONFIG.PlatformIOPath)" -ForegroundColor Yellow
Write-Host ""

function Test-PlatformIO {
    if (!(Test-Path $CONFIG.PlatformIOPath)) {
        Write-Host "❌ PlatformIO not found at: $($CONFIG.PlatformIOPath)" -ForegroundColor Red
        Write-Host "Please install PlatformIO or update the path in this script." -ForegroundColor Red
        return $false
    }
    return $true
}

function Test-ProjectPath {
    if (!(Test-Path $CONFIG.ProjectPath)) {
        Write-Host "❌ Project path not found: $($CONFIG.ProjectPath)" -ForegroundColor Red
        return $false
    }
    if (!(Test-Path (Join-Path $CONFIG.ProjectPath "platformio.ini"))) {
        Write-Host "❌ platformio.ini not found in project directory" -ForegroundColor Red
        return $false
    }
    return $true
}

function Build-Firmware {
    Write-Host "📦 STEP 1: Compiling firmware..." -ForegroundColor Green
    
    try {
        Set-Location $CONFIG.ProjectPath
        $output = & $CONFIG.PlatformIOPath run 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Firmware compilation successful" -ForegroundColor Green
            
            # Check if firmware.bin exists
            $firmwarePath = Join-Path $CONFIG.ProjectPath $CONFIG.FirmwarePath
            if (Test-Path $firmwarePath) {
                $size = (Get-Item $firmwarePath).Length
                Write-Host "📄 Firmware size: $([math]::Round($size/1KB, 1)) KB" -ForegroundColor Cyan
                return $true
            } else {
                Write-Host "❌ Firmware binary not found after compilation" -ForegroundColor Red
                return $false
            }
        } else {
            Write-Host "❌ Firmware compilation failed" -ForegroundColor Red
            Write-Host $output -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host "❌ Error during firmware compilation: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

function Build-Filesystem {
    Write-Host "📦 STEP 3: Compiling filesystem..." -ForegroundColor Green
    
    try {
        Set-Location $CONFIG.ProjectPath
        $output = & $CONFIG.PlatformIOPath run --target buildfs 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Filesystem compilation successful" -ForegroundColor Green
            
            # Check if littlefs.bin exists
            $filesystemPath = Join-Path $CONFIG.ProjectPath $CONFIG.FilesystemPath
            if (Test-Path $filesystemPath) {
                $size = (Get-Item $filesystemPath).Length
                Write-Host "📄 Filesystem size: $([math]::Round($size/1KB, 1)) KB" -ForegroundColor Cyan
                return $true
            } else {
                Write-Host "❌ Filesystem binary not found after compilation" -ForegroundColor Red
                return $false
            }
        } else {
            Write-Host "❌ Filesystem compilation failed" -ForegroundColor Red
            Write-Host $output -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host "❌ Error during filesystem compilation: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

function Open-OTAInterface {
    Write-Host "🌐 Opening OTA interface in browser..." -ForegroundColor Green
    
    $otaUrl = "$($CONFIG.DeviceIP)/update"
    Write-Host "📍 URL: $otaUrl" -ForegroundColor Cyan
    
    try {
        Start-Process $otaUrl
        return $true
    } catch {
        Write-Host "❌ Failed to open browser: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Please manually navigate to: $otaUrl" -ForegroundColor Yellow
        return $false
    }
}

function Show-UploadInstructions {
    param([string]$Type, [string]$FilePath)
    
    Write-Host ""
    Write-Host "🚀 MANUAL UPLOAD INSTRUCTIONS FOR ${Type}:" -ForegroundColor Yellow
    Write-Host "═══════════════════════════════════════════" -ForegroundColor Yellow
    Write-Host "1. In the browser, select '${Type}' from the dropdown" -ForegroundColor White
    Write-Host "2. Click 'Select File' button" -ForegroundColor White
    Write-Host "3. Navigate to and select:" -ForegroundColor White
    Write-Host "   $FilePath" -ForegroundColor Cyan
    Write-Host "4. Wait for upload to complete (shows 'Update Successful')" -ForegroundColor White
    if ($Type -eq "Firmware") {
        Write-Host "5. Click 'Go Back' when done" -ForegroundColor White
    } else {
        Write-Host "5. Device will restart automatically when done" -ForegroundColor White
    }
    Write-Host ""
}

function Wait-ForUserConfirmation {
    param([string]$Message)
    
    Write-Host $Message -ForegroundColor Yellow
    Read-Host "Press Enter to continue when ready"
}

# Main execution
function Main {
    # Validate prerequisites
    if (!(Test-PlatformIO)) { return }
    if (!(Test-ProjectPath)) { return }
    
    Write-Host "✅ Prerequisites validated" -ForegroundColor Green
    Write-Host ""
    
    # Step 1: Compile firmware
    if (!(Build-Firmware)) {
        Write-Host "❌ Build process failed at firmware compilation" -ForegroundColor Red
        return
    }
    
    # Step 2: Upload firmware
    Write-Host "🚀 STEP 2: Upload firmware via OTA..." -ForegroundColor Green
    Open-OTAInterface | Out-Null
    
    $firmwareFullPath = Join-Path $CONFIG.ProjectPath $CONFIG.FirmwarePath
    Show-UploadInstructions "Firmware" $firmwareFullPath
    Wait-ForUserConfirmation "⏳ Please complete firmware upload in browser..."
    
    # Step 3: Compile filesystem
    if (!(Build-Filesystem)) {
        Write-Host "❌ Build process failed at filesystem compilation" -ForegroundColor Red
        return
    }
    
    # Step 4: Upload filesystem
    Write-Host "🚀 STEP 4: Upload filesystem via OTA..." -ForegroundColor Green
    
    $filesystemFullPath = Join-Path $CONFIG.ProjectPath $CONFIG.FilesystemPath
    Show-UploadInstructions "LittleFS / SPIFFS" $filesystemFullPath
    Wait-ForUserConfirmation "⏳ Please complete filesystem upload in browser..."
    
    # Step 5: Verification
    Write-Host "🔍 STEP 5: Verification..." -ForegroundColor Green
    Write-Host "Please wait 15-30 seconds for device to restart, then test:" -ForegroundColor Yellow
    Write-Host "• Main interface: $($CONFIG.DeviceIP)/" -ForegroundColor Cyan
    Write-Host "• Settings page: $($CONFIG.DeviceIP)/settings.html" -ForegroundColor Cyan
    Write-Host ""
    
    Write-Host "🎉 OTA UPDATE PROCESS COMPLETED!" -ForegroundColor Green
    Write-Host "═══════════════════════════════════════" -ForegroundColor Green
    Write-Host "✅ Firmware compiled and ready for upload" -ForegroundColor Green
    Write-Host "✅ Filesystem compiled and ready for upload" -ForegroundColor Green
    Write-Host "📱 Manual upload steps completed via browser" -ForegroundColor Green
    Write-Host ""
    Write-Host "🌐 Your device should now be updated and accessible at:" -ForegroundColor Cyan
    Write-Host "   $($CONFIG.DeviceIP)" -ForegroundColor Cyan
}

# Execute main function
Main
