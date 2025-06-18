/*
 * Bobcat Ignition Controller
 * ESP32-based system to control Bobcat ignition sequence
 * 
 * Features:
 * - Glow plug preheating (20 seconds)
 * - Main ignition control
 * - Safety interlocks
 * - Status monitoring
 * 
 * Author: Generated for Bobcat Controller Project
 * Date: June 2025
 */

#include <Arduino.h>

// Pin definitions
const int GLOW_PLUG_RELAY_PIN = 2;    // GPIO2 for glow plug relay control
const int IGNITION_RELAY_PIN = 4;     // GPIO4 for main ignition relay
const int START_BUTTON_PIN = 18;      // GPIO18 for start button input
const int STOP_BUTTON_PIN = 19;       // GPIO19 for stop button input
const int STATUS_LED_PIN = 5;         // GPIO5 for status LED
const int BUZZER_PIN = 21;            // GPIO21 for buzzer/alarm

// Safety and monitoring pins
const int ENGINE_TEMP_PIN = 34;       // GPIO34 (ADC) for engine temperature sensor
const int OIL_PRESSURE_PIN = 35;      // GPIO35 (ADC) for oil pressure sensor
const int BATTERY_VOLTAGE_PIN = 32;   // GPIO32 (ADC) for battery voltage monitoring

// Timing constants
const unsigned long GLOW_PLUG_DURATION = 20000;  // 20 seconds in milliseconds
const unsigned long IGNITION_TIMEOUT = 5000;     // 5 seconds max cranking time
const unsigned long DEBOUNCE_DELAY = 50;         // Button debounce delay

// System states
enum SystemState {
  IDLE,
  GLOW_PLUG_HEATING,
  READY_TO_START,
  STARTING,
  RUNNING,
  SHUTDOWN,
  ERROR
};

// Global variables
SystemState currentState = IDLE;
unsigned long glowPlugStartTime = 0;
unsigned long ignitionStartTime = 0;
bool startButtonPressed = false;
bool stopButtonPressed = false;
unsigned long lastButtonCheck = 0;

// Function prototypes
void initializePins();
void checkButtons();
void updateSystem();
void controlGlowPlugs(bool enable);
void controlIgnition(bool enable);
void updateStatusLED();
void checkSafetyInputs();
void handleError(const char* errorMessage);

void setup() {
  Serial.begin(115200);
  Serial.println("Bobcat Ignition Controller Starting...");
  
  initializePins();
  currentState = IDLE;
  
  Serial.println("System initialized - Ready for operation");
  Serial.println("Press START button to begin ignition sequence");
}

void loop() {
  checkButtons();
  updateSystem();
  updateStatusLED();
  checkSafetyInputs();
  
  delay(10); // Small delay for system stability
}

void initializePins() {
  // Output pins
  pinMode(GLOW_PLUG_RELAY_PIN, OUTPUT);
  pinMode(IGNITION_RELAY_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Input pins
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  
  // ADC pins for sensors (no pinMode needed for ADC)
  
  // Initialize all outputs to safe state
  digitalWrite(GLOW_PLUG_RELAY_PIN, LOW);
  digitalWrite(IGNITION_RELAY_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void checkButtons() {
  if (millis() - lastButtonCheck < DEBOUNCE_DELAY) {
    return; // Debounce protection
  }
  
  // Check start button (active low with pullup)
  bool startButton = !digitalRead(START_BUTTON_PIN);
  bool stopButton = !digitalRead(STOP_BUTTON_PIN);
  
  if (startButton && !startButtonPressed) {
    startButtonPressed = true;
    Serial.println("START button pressed");
  } else if (!startButton) {
    startButtonPressed = false;
  }
  
  if (stopButton && !stopButtonPressed) {
    stopButtonPressed = true;
    Serial.println("STOP button pressed");
  } else if (!stopButton) {
    stopButtonPressed = false;
  }
  
  lastButtonCheck = millis();
}

void updateSystem() {
  switch (currentState) {
    case IDLE:
      if (startButtonPressed) {
        Serial.println("Starting ignition sequence...");
        Serial.println("Phase 1: Glow plug heating (20 seconds)");
        currentState = GLOW_PLUG_HEATING;
        glowPlugStartTime = millis();
        controlGlowPlugs(true);
      }
      break;
      
    case GLOW_PLUG_HEATING:
      if (stopButtonPressed) {
        Serial.println("Ignition sequence aborted");
        currentState = SHUTDOWN;
      } else if (millis() - glowPlugStartTime >= GLOW_PLUG_DURATION) {
        Serial.println("Glow plug heating complete");
        Serial.println("Ready to start engine - Press START again to crank");
        currentState = READY_TO_START;
        controlGlowPlugs(false);
      }
      break;
      
    case READY_TO_START:
      if (stopButtonPressed) {
        Serial.println("Start sequence cancelled");
        currentState = SHUTDOWN;
      } else if (startButtonPressed) {
        Serial.println("Phase 2: Engine cranking");
        currentState = STARTING;
        ignitionStartTime = millis();
        controlIgnition(true);
      }
      // Timeout after 30 seconds of waiting
      else if (millis() - glowPlugStartTime >= (GLOW_PLUG_DURATION + 30000)) {
        Serial.println("Start timeout - returning to idle");
        currentState = IDLE;
      }
      break;
      
    case STARTING:
      if (stopButtonPressed) {
        Serial.println("Engine start aborted");
        currentState = SHUTDOWN;
      } else if (millis() - ignitionStartTime >= IGNITION_TIMEOUT) {
        Serial.println("Engine start timeout - stopping cranking");
        controlIgnition(false);
        currentState = RUNNING; // Assume engine started or ready to retry
      }
      // In a real implementation, you'd check engine RPM or oil pressure
      // to determine if engine actually started
      break;
      
    case RUNNING:
      if (stopButtonPressed) {
        Serial.println("Engine shutdown requested");
        currentState = SHUTDOWN;
      }
      // Monitor engine parameters here
      break;
      
    case SHUTDOWN:
      Serial.println("Shutting down system...");
      controlGlowPlugs(false);
      controlIgnition(false);
      delay(1000); // Brief delay for relay settling
      currentState = IDLE;
      Serial.println("System ready for next start cycle");
      break;
      
    case ERROR:
      // Handle error state - flash LED, sound buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      if (stopButtonPressed) {
        currentState = SHUTDOWN;
      }
      break;
  }
}

void controlGlowPlugs(bool enable) {
  digitalWrite(GLOW_PLUG_RELAY_PIN, enable ? HIGH : LOW);
  Serial.print("Glow plugs: ");
  Serial.println(enable ? "ON" : "OFF");
}

void controlIgnition(bool enable) {
  digitalWrite(IGNITION_RELAY_PIN, enable ? HIGH : LOW);
  Serial.print("Ignition/Starter: ");
  Serial.println(enable ? "ON" : "OFF");
}

void updateStatusLED() {
  static unsigned long lastLEDUpdate = 0;
  static bool ledState = false;
  
  unsigned long interval = 1000; // Default slow blink
  
  switch (currentState) {
    case IDLE:
      interval = 2000; // Very slow blink
      break;
    case GLOW_PLUG_HEATING:
      interval = 500; // Medium blink
      break;
    case READY_TO_START:
      digitalWrite(STATUS_LED_PIN, HIGH); // Solid on
      return;
    case STARTING:
      interval = 100; // Fast blink
      break;
    case RUNNING:
      digitalWrite(STATUS_LED_PIN, HIGH); // Solid on
      return;
    case ERROR:
      interval = 200; // Very fast blink
      break;
  }
  
  if (millis() - lastLEDUpdate >= interval) {
    ledState = !ledState;
    digitalWrite(STATUS_LED_PIN, ledState);
    lastLEDUpdate = millis();
  }
}

void checkSafetyInputs() {
  // Read analog sensors
  int batteryVoltage = analogRead(BATTERY_VOLTAGE_PIN);
  int engineTemp = analogRead(ENGINE_TEMP_PIN);
  int oilPressure = analogRead(OIL_PRESSURE_PIN);
  
  // Convert ADC readings to meaningful values
  // Battery voltage: assuming voltage divider for 12V system
  float batteryVolts = (batteryVoltage * 3.3 / 4095.0) * (12.0 / 3.3); // Adjust divider ratio as needed
  
  // Check for low battery voltage
  if (batteryVolts < 10.5 && currentState != IDLE) {
    handleError("Low battery voltage");
    return;
  }
  
  // Add more safety checks as needed
  // - Engine temperature limits
  // - Oil pressure monitoring
  // - Emergency stop conditions
}

void handleError(const char* errorMessage) {
  Serial.print("ERROR: ");
  Serial.println(errorMessage);
  
  // Immediately shut off all systems
  controlGlowPlugs(false);
  controlIgnition(false);
  
  currentState = ERROR;
}
