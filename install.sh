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
sudo apt install -y libindi-dev cmake build-essential git liblgpio-dev pkg-config

echo
echo "Step 2: Verifying INDI installation..."

# Check if libindi-dev is installed
if ! dpkg -l | grep -q libindi-dev; then
    echo "ERROR: libindi-dev is not installed!"
    echo "Please install INDI first. This driver requires INDI version 2.1.6 or later."
    echo
    echo "For Raspberry Pi / StellarMate:"
    echo "  sudo add-apt-repository ppa:mutlaqja/ppa"
    echo "  sudo apt update"
    echo "  sudo apt install libindi-dev indi-bin"
    echo
    echo "Or build INDI from source:"
    echo "  https://github.com/indilib/indi"
    exit 1
fi

# Try to detect INDI version using multiple methods
INDI_VERSION=""

# Method 1: Try pkg-config
if command -v pkg-config &> /dev/null; then
    INDI_VERSION=$(pkg-config --modversion indi 2>/dev/null || true)
fi

# Method 2: Check dpkg for package version
if [ -z "$INDI_VERSION" ]; then
    INDI_VERSION=$(dpkg -l | grep libindi-dev | awk '{print $3}' | head -1 || true)
fi

# Method 3: Try indi_getprop if available
if [ -z "$INDI_VERSION" ] && command -v indi_getprop &> /dev/null; then
    INDI_VERSION=$(indi_getprop --version 2>/dev/null | grep -oP '\d+\.\d+\.\d+' | head -1 || true)
fi

# Method 4: Check for INDI header files
if [ -z "$INDI_VERSION" ]; then
    for header in /usr/include/libindi/indiapi.h /usr/local/include/libindi/indiapi.h; do
        if [ -f "$header" ]; then
            echo "✓ INDI development files found at $header"
            break
        fi
    done
fi

if [ -n "$INDI_VERSION" ]; then
    echo "✓ INDI version $INDI_VERSION detected"
    
    # Warn if version might be too old
    MAJOR=$(echo $INDI_VERSION | cut -d. -f1)
    MINOR=$(echo $INDI_VERSION | cut -d. -f2)
    if [ "$MAJOR" -lt 2 ] || ([ "$MAJOR" -eq 2 ] && [ "$MINOR" -lt 1 ]); then
        echo "⚠ WARNING: INDI version $INDI_VERSION may be too old (requires 2.1.6+)"
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
else
    echo "⚠ Could not detect INDI version, but libindi-dev is installed"
    echo "  Proceeding with build..."
fi

echo
echo "Step 3: Adding user to gpio group..."
sudo usermod -a -G gpio $USER

echo
echo "Step 4: Building driver..."

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
echo "Step 5: Installing driver..."
sudo make install

echo
echo "Step 6: Verifying installation..."

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
echo "Step 7: Adding user to GPIO group..."
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
