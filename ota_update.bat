@echo off
REM Bobcat Ignition Controller - OTA Update Batch Script
REM Simple wrapper for PowerShell script

echo.
echo ========================================
echo Bobcat Ignition Controller OTA Update
echo ========================================
echo.

REM Check if PowerShell is available
powershell -Command "Get-Host" >nul 2>&1
if errorlevel 1 (
    echo ERROR: PowerShell is not available on this system
    pause
    exit /b 1
)

REM Run the PowerShell script
echo Running OTA update script...
echo.
powershell -ExecutionPolicy Bypass -File "ota_update.ps1"

echo.
echo Script completed. Press any key to exit...
pause >nul
