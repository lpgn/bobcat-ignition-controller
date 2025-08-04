# Refactoring Plan: Centralized Backend State

This document outlines the concrete steps to refactor the Bobcat Ignition Controller to a centralized backend state model.

## Phase 1: Backend Refactoring (C++)

-   [x] **1.1: Centralize System State (`system_state.h`)**
    -   [x] Define a new `struct` named `SystemState_t`.
    -   [x] Move all global state variables (e.g., `keyPosition`, `lightsOn`, sensor values, timers) into this struct.
    -   [x] Create a single, global instance of `SystemState_t`.

-   [x] **1.2: Implement Authoritative Web API (`web_interface.cpp`, `web_interface.h`)**
    -   [x] Add `ArduinoJson` library to `platformio.ini` for robust JSON handling.
    -   [x] Define a new `SystemEvent_t` struct and a thread-safe event queue (e.g., using `xQueueCreate`).
    -   [x] Create the `/status` GET endpoint.
        -   [x] It must read the global `SystemState_t` instance.
        -   [x] It must serialize the state struct into a JSON object and send it to the client.
    -   [x] Create the `/control` POST endpoint.
        -   [x] It must parse incoming JSON commands.
        -   [x] It must create a `SystemEvent_t` based on the command.
        -   [x] It must add the event to the central event queue.
        -   [x] It must **not** directly change any state or call any hardware functions.

-   [x] **1.3: Refactor State Machine (`system_state.cpp`, `system_state.h`)**
    -   [x] Modify all functions to accept a pointer to the `SystemState_t` struct instead of using global variables.
    -   [x] Create a new `process_system_events()` function.
    -   [x] This function will read from the event queue in a loop.
    -   [x] Implement a `switch` statement to handle different event types.
    -   [x] All state transitions, timer management, and hardware calls will be managed from this function based on the incoming events.

-   [x] **1.4: Update Main Loop (`main.cpp`)**
    -   [x] Initialize the event queue.
    -   [x] The `loop()` function should only call `process_system_events()` and any other non-state-machine handlers (like `server.handleClient()`).
    -   [x] Remove all other logic from the main loop.

## Phase 2: Frontend Simplification (JavaScript)

-   [x] **2.1: Eliminate Client-Side State (`data/script.js`)**
    -   [x] Delete global state variables: `currentKeyPosition`, `isCranking`, `startKeyHeld`.

-   [x] **2.2: Simplify Event Handlers (`data/script.js`)**
    -   [x] Remove all DOM manipulation (changing styles, text content) from `setKeyPosition`, `holdStartPosition`, `releaseStartPosition`, and other input handlers.
    -   [x] Ensure these functions *only* call `sendCommand()` with the appropriate action.

-   [x] **2.3: Consolidate UI Updates (`data/script.js`)**
    -   [x] Ensure the `updateDashboard(status)` function is the *only* place where UI elements are updated.
    -   [x] Verify this function is called only from the success callback of the `fetch('/status')` poll.
    -   [x] Update `updateDashboard` to handle the new, comprehensive state object from the backend, including key position, lights, etc.

## Phase 3: Documentation

-   [ ] **3.1: Update `README.md`**
    -   [ ] Revise the "Operation Sequence" section to reflect the new event-driven architecture.
    -   [ ] Add a new "API Endpoints" section detailing the `/status` (GET) and `/control` (POST) endpoints and their JSON schemas.

-   [ ] **3.2: Update `docs/refactoring_plan.md`**
    -   [ ] Mark all checkboxes as complete.
