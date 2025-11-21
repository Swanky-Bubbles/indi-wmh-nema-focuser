# Waveshare Stepper Motor HAT (B) NEMA Focuser Driver for INDI

INDI 2.1.6+ driver for controlling a NEMA stepper motor (such as NEMA 17) using the Waveshare Stepper Motor HAT (B) on a Raspberry Pi running StellarMate.

## Features

- **Full INDI Focuser Support**: Absolute and relative positioning, sync capability
- **Multi-Motor Support**: Control any of the 3 motor channels (X, Y, or Z) on the Waveshare HAT
- **Configurable Microstepping**: Software settings for microstepping modes (1, 1/2, 1/4, 1/8, 1/16)
- **Adjustable Speed**: Configurable step delay for different motor speeds
- **Power Efficient**: Motor automatically disabled when not in use
- **StellarMate Compatible**: Designed and tested for StellarMate on Raspberry Pi

## Hardware Requirements

- Raspberry Pi (tested on Pi 4, compatible with Pi 3/3+/4/5)
- [Waveshare Stepper Motor HAT (B)](https://www.waveshare.com/stepper-motor-hat-b.htm)
- NEMA 17 stepper motor (or similar)
- Power supply for the stepper motor (typically 12V)

## Waveshare HAT Pin Configuration

The driver uses the following GPIO pins for each motor channel:

### Motor X (Default)
- DIR: GPIO 13
- STEP: GPIO 19
- ENABLE: GPIO 12

### Motor Y
- DIR: GPIO 24
- STEP: GPIO 18
- ENABLE: GPIO 4

### Motor Z
- DIR: GPIO 21
- STEP: GPIO 26
- ENABLE: GPIO 25

## Installation on StellarMate

### 1. Install Prerequisites

```bash
# Update system
sudo apt update

# Install required dependencies
sudo apt install -y libindi-dev cmake build-essential git

# Install lgpio library for GPIO control (no daemon required!)
sudo apt install -y liblgpio-dev

# Add user to gpio group for permissions
sudo usermod -a -G gpio $USER
```

### 2. Build the Driver

```bash
# Clone or navigate to the driver directory
cd /path/to/indi-wmh-nema-focuser

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Install the driver
sudo make install
```

### 3. Restart INDI Server

```bash
# Restart INDI to recognize the new driver
sudo systemctl restart indiserver
```

Or restart from the StellarMate Web Manager.

## Usage

### Connecting from INDI Clients

1. **KStars/Ekos**: 
   - Open Ekos Profile Editor
   - Add "Waveshare NEMA Focuser" under Focusers
   - Start the profile and connect

2. **StellarMate App**:
   - Navigate to Devices
   - Select "Waveshare NEMA Focuser"
   - Connect

### Configuration Options

#### Main Control
- **Absolute Position**: Move to a specific step position
- **Relative Position**: Move inward/outward by a number of steps
- **Sync**: Set current position to a specific value
- **Step Delay**: Adjust the delay between steps (100-10000 μs)
  - Lower values = faster movement
  - Higher values = smoother, quieter operation

#### Options Tab
- **Motor Channel**: Select X, Y, or Z motor
- **Steps per Revolution**: Configure for your motor (typically 200 for NEMA 17)
- **Microstepping Mode**: Select microstepping ratio
  - Note: This is a software setting for position calculation
  - Hardware microstepping must be configured via jumpers on the HAT

### Hardware Microstepping Configuration

The Waveshare Stepper Motor HAT (B) uses DIP switches to configure microstepping for each motor channel:

| Mode | Steps/Rev | Switch 1 | Switch 2 | Switch 3 | Recommended For |
|------|-----------|----------|----------|----------|----------------|
| Full Step | 200 | OFF | OFF | OFF | Fast movements |
| Half Step | 400 | ON | OFF | OFF | - |
| Quarter Step | 800 | OFF | ON | OFF | - |
| Eighth Step | 1600 | ON | ON | OFF | - |
| **1/16 Step** | **3200** | **ON** | **ON** | **ON** | **Telescope focusing** |

**Switch positions:**
- **OFF** = Switch in down/off position
- **ON** = Switch in up/on position

**Recommended setting:** 1/16 microstepping (all three switches ON) for smooth, precise focusing.

**Important Notes:**
1. The driver's "Microstepping Mode" setting should match your hardware DIP switch configuration
2. DIP switches are located on the HAT, typically labeled SW1, SW2, SW3 for each motor
3. Motor X switches control Motor X (MA), Motor Y controls Motor Y (MB), etc.
4. Power off the HAT before changing DIP switch settings
5. After changing switches, adjust the "Steps per Revolution" in INDI (200 × microstepping factor)

Consult the [Waveshare documentation](https://www.waveshare.com/wiki/Stepper_Motor_HAT_(B)) for the exact DIP switch locations on your board revision.

## Troubleshooting

### Driver doesn't appear in device list
- Ensure the driver was installed: `ls /usr/bin/indi_wmh_nema_focuser`
- Check XML file exists: `ls /usr/share/indi/indi_wmh_nema_focuser.xml`
- Restart INDI server

### "Failed to initialize GPIO" error
```bash
# Check GPIO device permissions
ls -l /dev/gpiochip0

# Add user to gpio group if needed
sudo usermod -a -G gpio $USER

# Logout and login for group changes to take effect
# Or use: newgrp gpio

# Verify access
groups | grep gpio
```

### Motor not moving
**Most common issue: Current limit set too low!**

1. **Adjust current limit potentiometer:**
   - Locate the small potentiometer near the DRV8825 chip
   - Turn it clockwise (about 1/4 turn at a time)
   - Test after each adjustment
   - For NEMA 17: aim for 0.4-0.8V reference voltage

2. **Other checks:**
   - Verify 12V power supply is connected to the HAT
   - Check motor is connected to the correct channel (Motor X = MA)
   - Ensure all 4 motor wires are firmly connected
   - Try setting all DIP switches to OFF (full-step mode)
   - Run test script: `./test_gpio_simple.py`
   - Try increasing step delay for testing

### Permission issues
```bash
# Add user to gpio group
sudo usermod -a -G gpio $USER

# Logout and login again for changes to take effect
```

### Motor moves but loses steps
- Increase step delay (slower movement)
- Check power supply voltage and current
- Verify motor current limit settings on the HAT
- Reduce load on the focuser mechanism

## Technical Details

### Position Tracking
- Driver tracks position in steps (not millimeters)
- Maximum position: 100,000 steps (configurable in code)
- Position persists across connections (stored in INDI)

### Movement Control
- Absolute positioning with automatic direction control
- Batched step execution for smooth movement
- Motor automatically disabled when idle to reduce power and heat

### GPIO Control
- Uses lgpio library for direct GPIO access (no daemon required)
- Accesses /dev/gpiochip0 directly
- Step pulse width: 10 microseconds
- Configurable inter-step delay (100-10000 μs)
- Enable pin is active HIGH (motor locks when pin is HIGH)

## Development

### Building from Source

```bash
git clone <repository-url>
cd indi-wmh-nema-focuser
mkdir build && cd build
cmake ..
make
```

### Testing

```bash
# Test hardware first with Python script
./test_gpio_simple.py

# Run the driver directly for debugging
INDI_DEBUG=5 /usr/local/bin/indi_wmh_nema_focuser

# Connect with indi_getprop
indi_getprop "Waveshare NEMA Focuser.CONNECTION.CONNECT"
indi_setprop "Waveshare NEMA Focuser.CONNECTION.CONNECT=On"

# Move to position 100
indi_setprop "Waveshare NEMA Focuser.ABS_FOCUS_POSITION.FOCUS_ABSOLUTE_POSITION=100"
```

## License

This driver is released under the GNU General Public License v2.0 or later.

## Author

Danny McGee (2025)

## Acknowledgments

- INDI Library team for the excellent framework
- Waveshare for the Stepper Motor HAT hardware
- StellarMate team for Raspberry Pi astronomy platform

## Support

For issues, questions, or contributions, please open an issue on the project repository.

## Version History

### 1.0.0 (2025-11-20)
- Initial release
- Support for all 3 motor channels
- Configurable microstepping and speed
- Full INDI focuser capabilities
