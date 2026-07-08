#!/bin/sh
# Build + asset validation script for the Bobcat 743 controller.
# Runs node syntax checks on data/*.js and then builds the esp32dev environment.

set -u

exit_code=0

# Check every JS file in data/ for syntax errors.
if [ -d "data" ]; then
    for js in data/*.js; do
        [ -f "$js" ] || continue
        if node --check "$js"; then
            printf "  OK    JS syntax: %s\n" "$js"
        else
            printf "  FAIL  JS syntax: %s\n" "$js"
            exit_code=1
        fi
    done
fi

# Build the default esp32dev environment.
if pio run -e esp32dev; then
    printf "  OK    pio run -e esp32dev\n"
else
    printf "  FAIL  pio run -e esp32dev\n"
    exit_code=1
fi

printf "\n"
if [ "$exit_code" -eq 0 ]; then
    printf "PASS: all validation checks succeeded.\n"
else
    printf "FAIL: one or more validation checks failed.\n"
fi

exit "$exit_code"
