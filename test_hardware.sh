#!/bin/bash
# Hardware test script for Waveshare Stepper Motor HAT (B)
# Tests GPIO connectivity and basic motor operation using lgpio

echo "=========================================="
echo "Waveshare HAT Hardware Test"
echo "=========================================="
echo

# Check if user is in gpio group
if ! groups | grep -q gpio; then
    echo "WARNING: You are not in the 'gpio' group!"
    echo "Add yourself with: sudo usermod -a -G gpio \$USER"
    echo "Then logout and login again."
    echo
fi

# Check if lgpio tools are available
if ! command -v rgpiod &> /dev/null; then
    echo "lgpio tools not found. Installing..."
    sudo apt install -y liblgpio-dev lgpio
fi

echo "Testing GPIO access..."
if [ ! -r /dev/gpiochip0 ]; then
    echo "ERROR: Cannot access /dev/gpiochip0"
    echo "Make sure you're in the gpio group and have logged out/in."
    exit 1
fi

echo "✓ GPIO access OK"
echo

# Test Motor X pins
echo "Testing Motor X (GPIO 12, 13, 19)..."

# Set pins as outputs
pigs m 12 w  # ENABLE as output
pigs m 13 w  # DIR as output
pigs m 19 w  # STEP as output

# Disable motor (active low)
pigs w 12 1

echo "✓ Motor X pins configured"
echo

# Ask user which motor to test
echo "Which motor would you like to test?"
echo "1) Motor X (GPIO 12, 13, 19)"
echo "2) Motor Y (GPIO 4, 24, 18)"
echo "3) Motor Z (GPIO 25, 21, 26)"
echo "4) Skip motor test"
read -p "Enter choice (1-4): " choice

case $choice in
    1)
        ENABLE_PIN=12
        DIR_PIN=13
        STEP_PIN=19
        MOTOR_NAME="X"
        ;;
    2)
        ENABLE_PIN=4
        DIR_PIN=24
        STEP_PIN=18
        MOTOR_NAME="Y"
        ;;
    3)
        ENABLE_PIN=25
        DIR_PIN=21
        STEP_PIN=26
        MOTOR_NAME="Z"
        ;;
    4)
        echo "Skipping motor test."
        exit 0
        ;;
    *)
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac

echo
echo "Testing Motor $MOTOR_NAME..."
echo "The motor should move a few steps in each direction."
echo "Make sure:"
echo "  - Motor is connected to Motor $MOTOR_NAME port"
echo "  - Power supply is connected to the HAT"
echo "  - Motor current limit is properly set"
echo
read -p "Press Enter to start test (Ctrl+C to cancel)..."

# Initialize pins
pigs m $ENABLE_PIN w
pigs m $DIR_PIN w
pigs m $STEP_PIN w

# Enable motor (active low)
pigs w $ENABLE_PIN 0

echo
echo "Moving forward 50 steps..."
pigs w $DIR_PIN 1  # Forward direction

for i in {1..50}; do
    pigs w $STEP_PIN 1
    sleep 0.001
    pigs w $STEP_PIN 0
    sleep 0.001
done

sleep 1

echo "Moving backward 50 steps..."
pigs w $DIR_PIN 0  # Reverse direction

for i in {1..50}; do
    pigs w $STEP_PIN 1
    sleep 0.001
    pigs w $STEP_PIN 0
    sleep 0.001
done

# Disable motor
pigs w $ENABLE_PIN 1

echo
echo "Test complete!"
echo
read -p "Did the motor move correctly? (y/n): " response

if [[ $response =~ ^[Yy]$ ]]; then
    echo
    echo "✓ Hardware test PASSED!"
    echo "Your Waveshare HAT is working correctly."
    echo "You can now use the INDI driver."
else
    echo
    echo "✗ Hardware test FAILED"
    echo
    echo "Troubleshooting:"
    echo "1. Check power supply connection"
    echo "2. Verify motor is connected to correct port"
    echo "3. Check motor current limit adjustment on HAT"
    echo "4. Ensure microstepping jumpers are set correctly"
    echo "5. Try a different motor to rule out motor issues"
    echo "6. Check for loose connections"
fi

echo
