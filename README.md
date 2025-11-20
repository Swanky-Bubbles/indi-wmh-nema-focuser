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

# Install pigpio library for GPIO control
sudo apt install -y pigpio libpigpio-dev

# Enable and start pigpiod daemon
sudo systemctl enable pigpiod
sudo systemctl start pigpiod

# Verify pigpiod is running
sudo systemctl status pigpiod

# Alternative: Start pigpiod manually if systemctl doesn't work
# sudo pigpiod
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

The Waveshare Stepper Motor HAT (B) uses DIP switches or jumpers to configure microstepping. Consult the [Waveshare documentation](https://www.waveshare.com/wiki/Stepper_Motor_HAT_(B)) for your specific board revision.

Typical configuration:
- All switches OFF = Full step
- Switch pattern per Waveshare documentation for 1/2, 1/4, 1/8, 1/16 steps

**Important**: The driver's microstepping setting should match your hardware configuration.

## Troubleshooting

### Driver doesn't appear in device list
- Ensure the driver was installed: `ls /usr/bin/indi_wmh_nema_focuser`
- Check XML file exists: `ls /usr/share/indi/indi_wmh_nema_focuser.xml`
- Restart INDI server

### "Failed to initialize GPIO" error
```bash
# Check if pigpiod is running
sudo systemctl status pigpiod

# If not running, start it
sudo systemctl start pigpiod

# Enable it to start on boot
sudo systemctl enable pigpiod

# If systemctl method doesn't work, try starting manually:
sudo killall pigpiod  # Kill any existing instance
sudo pigpiod          # Start daemon manually

# Verify it's running
pigs hwver            # Should return hardware version number
```

### Motor not moving
- Verify power supply is connected to the Waveshare HAT
- Check motor is properly connected to the correct channel
- Verify GPIO pins are not being used by other services
- Check motor enable switch on the HAT (if applicable)
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
- Uses pigpio library for precise timing
- Step pulse width: 2 microseconds
- Configurable inter-step delay

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
# Run the driver directly for debugging
INDI_DEBUG=5 /usr/bin/indi_wmh_nema_focuser

# Connect with indi_getprop
indi_getprop "Waveshare NEMA Focuser.CONNECTION.CONNECT"
indi_setprop "Waveshare NEMA Focuser.CONNECTION.CONNECT=On"
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
