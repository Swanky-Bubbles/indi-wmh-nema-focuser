#!/bin/bash
# Installation script for Waveshare NEMA Focuser INDI Driver
# For StellarMate / Raspberry Pi

set -e

echo "=========================================="
echo "Waveshare NEMA Focuser Driver Installer"
echo "=========================================="
echo

# Check if running on Raspberry Pi
if [ ! -f /proc/device-tree/model ] || ! grep -q "Raspberry Pi" /proc/device-tree/model 2>/dev/null; then
    echo "WARNING: This doesn't appear to be a Raspberry Pi."
    echo "The driver requires GPIO access and may not work on other platforms."
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check if running as root for installation
if [ "$EUID" -eq 0 ]; then 
    echo "Please don't run this script as root/sudo."
    echo "You'll be prompted for sudo password when needed."
    exit 1
fi

echo "Step 1: Installing dependencies..."
sudo apt update
sudo apt install -y libindi-dev cmake build-essential git liblgpio-dev

echo
echo "Step 2: Adding user to gpio group..."
sudo usermod -a -G gpio $USER

echo
echo "Step 3: Building driver..."

# Create build directory
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

mkdir build
cd build

# Configure
echo "Running cmake..."
cmake ..

# Build
echo "Compiling..."
make -j$(nproc)

echo
echo "Step 4: Installing driver..."
sudo make install

echo
echo "Step 5: Verifying installation..."

if [ -f "/usr/local/bin/indi_wmh_nema_focuser" ] || [ -f "/usr/bin/indi_wmh_nema_focuser" ]; then
    echo "✓ Driver binary installed successfully"
    if [ -f "/usr/local/bin/indi_wmh_nema_focuser" ]; then
        echo "  Location: /usr/local/bin/indi_wmh_nema_focuser"
    else
        echo "  Location: /usr/bin/indi_wmh_nema_focuser"
    fi
else
    echo "✗ Driver binary not found!"
    exit 1
fi

if [ -f "/usr/share/indi/indi_wmh_nema_focuser.xml" ]; then
    echo "✓ Driver XML file installed successfully"
else
    echo "✗ Driver XML file not found!"
    exit 1
fi

echo
echo "Step 6: Adding user to GPIO group..."
sudo usermod -a -G gpio $USER

echo
echo "=========================================="
echo "Installation completed successfully!"
echo "=========================================="
echo
echo "IMPORTANT: You need to log out and log back in for GPIO group"
echo "permissions to take effect."
echo
echo "After logging back in:"
echo "1. Restart INDI server: sudo systemctl restart indiserver"
echo "   (or restart from StellarMate Web Manager)"
echo "2. The driver should appear as 'Waveshare NEMA Focuser'"
echo "   in your INDI client (KStars/Ekos/StellarMate)"
echo
echo "Hardware setup:"
echo "- Connect your NEMA motor to the Waveshare HAT"
echo "- Ensure the HAT has proper power supply connected"
echo "- Configure microstepping jumpers on the HAT if needed"
echo
echo "See README.md for detailed usage instructions."
echo
