#!/usr/bin/env python3
"""
Simple GPIO test for Waveshare Stepper Motor HAT (B)
Tests basic motor movement using lgpio library
"""

import sys
import time

try:
    import lgpio
except ImportError:
    print("ERROR: lgpio library not found!")
    print("Install with: sudo apt install python3-lgpio")
    sys.exit(1)

# GPIO pins for Motor X
DIR_PIN = 13
STEP_PIN = 19
ENABLE_PIN = 12

def main():
    print("=" * 50)
    print("Waveshare Stepper HAT - Simple GPIO Test")
    print("=" * 50)
    print()
    
    # Open GPIO chip
    try:
        h = lgpio.gpiochip_open(0)
        print("✓ Opened GPIO chip successfully")
    except Exception as e:
        print(f"✗ Failed to open GPIO chip: {e}")
        print("Make sure you're in the gpio group:")
        print("  sudo usermod -a -G gpio $USER")
        print("Then logout and login again.")
        return 1
    
    try:
        # Configure pins as outputs
        lgpio.gpio_claim_output(h, DIR_PIN)
        lgpio.gpio_claim_output(h, STEP_PIN)
        lgpio.gpio_claim_output(h, ENABLE_PIN)
        print("✓ Claimed GPIO pins for Motor X")
        
        # Enable motor (active low)
        lgpio.gpio_write(h, ENABLE_PIN, 0)
        print("✓ Motor enabled")
        print()
        
        input("Press Enter to test motor movement (Ctrl+C to cancel)...")
        print()
        
        # Move forward 100 steps slowly
        print("Moving forward 100 steps...")
        lgpio.gpio_write(h, DIR_PIN, 1)  # Forward
        time.sleep(0.01)  # Let direction settle
        
        for i in range(100):
            lgpio.gpio_write(h, STEP_PIN, 1)
            time.sleep(0.005)  # 5ms high
            lgpio.gpio_write(h, STEP_PIN, 0)
            time.sleep(0.005)  # 5ms low (slow enough to see/hear)
            if (i + 1) % 20 == 0:
                print(f"  Step {i + 1}/100")
        
        time.sleep(1)
        
        # Move backward 100 steps slowly
        print("Moving backward 100 steps...")
        lgpio.gpio_write(h, DIR_PIN, 0)  # Reverse
        time.sleep(0.01)  # Let direction settle
        
        for i in range(100):
            lgpio.gpio_write(h, STEP_PIN, 1)
            time.sleep(0.005)  # 5ms high
            lgpio.gpio_write(h, STEP_PIN, 0)
            time.sleep(0.005)  # 5ms low
            if (i + 1) % 20 == 0:
                print(f"  Step {i + 1}/100")
        
        print()
        print("✓ Test complete!")
        print()
        response = input("Did the motor move correctly? (y/n): ")
        
        if response.lower() == 'y':
            print()
            print("✓ Hardware test PASSED!")
            print("Your Waveshare HAT is working correctly.")
        else:
            print()
            print("✗ Hardware test FAILED")
            print()
            print("Troubleshooting:")
            print("1. Check power supply connection")
            print("2. Verify motor is connected to Motor X port")
            print("3. Check motor current limit adjustment on HAT")
            print("4. Ensure microstepping jumpers are set correctly")
        
    finally:
        # Disable motor and clean up
        lgpio.gpio_write(h, ENABLE_PIN, 1)
        lgpio.gpio_free(h, DIR_PIN)
        lgpio.gpio_free(h, STEP_PIN)
        lgpio.gpio_free(h, ENABLE_PIN)
        lgpio.gpiochip_close(h)
        print()
        print("GPIO cleaned up")
    
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nTest cancelled by user")
        sys.exit(1)
