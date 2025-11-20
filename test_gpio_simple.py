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
        print(f"  DIR_PIN={DIR_PIN}, STEP_PIN={STEP_PIN}, ENABLE_PIN={ENABLE_PIN}")
        print()
        
        print("IMPORTANT CHECKS:")
        print("1. Is the 12V power supply connected to the HAT?")
        print("2. Is the motor connected to Motor X (MA) port?")
        print("3. Are all 4 motor wires firmly connected?")
        print("4. Is the current limit potentiometer adjusted? (turn clockwise)")
        print()
        
        input("Press Enter to enable motor and check if it locks...")
        
        # Enable motor (active HIGH for this HAT - opposite of typical DRV8825)
        lgpio.gpio_write(h, ENABLE_PIN, 1)
        print("✓ Motor enabled (should feel resistance/lock)")
        time.sleep(0.5)
        
        response = input("Does the motor feel locked/have resistance? (y/n): ")
        if response.lower() != 'y':
            print()
            print("✗ Motor not locking - possible issues:")
            print("  - No power to HAT (check 12V supply)")
            print("  - Current limit set too low (adjust potentiometer)")
            print("  - Bad motor connection")
            print("  - Faulty driver chip")
            return 1
        
        print()
        input("Press Enter to test stepping (watch/listen carefully)...")
        print()
        
        # Set direction
        lgpio.gpio_write(h, DIR_PIN, 1)
        time.sleep(0.01)
        
        # Move forward 100 steps slowly
        print("Sending 100 step pulses (this will take ~2 seconds)...")
        print("You should hear clicking/buzzing and see slight movement...")
        
        for i in range(100):
            lgpio.gpio_write(h, STEP_PIN, 1)
            time.sleep(0.010)  # 10ms high
            lgpio.gpio_write(h, STEP_PIN, 0)
            time.sleep(0.010)  # 10ms low
            if (i + 1) % 25 == 0:
                print(f"  Sent {i + 1}/100 pulses...")
        
        print()
        print("✓ Step pulses sent")
        print()
        
        response = input("Did you hear any clicking or see any movement? (y/n): ")
        
        if response.lower() != 'y':
            print()
            print("✗ No movement detected - likely causes:")
            print("  1. CURRENT LIMIT TOO LOW - This is the most common issue!")
            print("     Solution: Turn the small potentiometer on the HAT clockwise")
            print("     (It's near the DRV8825 chip for Motor X)")
            print()
            print("  2. Wrong microstepping jumpers")
            print("     Try removing ALL jumpers (MS1, MS2, MS3) for full-step mode")
            print()
            print("  3. Incorrect motor wiring")
            print("     Motor should be on Motor X (MA) connector")
            print()
            print("  4. Power supply issue")
            print("     Verify 12V is present on the HAT power input")
        else:
            print()
            print("✓ Movement detected! Testing reverse...")
            
            # Reverse direction
            lgpio.gpio_write(h, DIR_PIN, 0)
            time.sleep(0.01)
            
            for i in range(100):
                lgpio.gpio_write(h, STEP_PIN, 1)
                time.sleep(0.010)
                lgpio.gpio_write(h, STEP_PIN, 0)
                time.sleep(0.010)
            
            print()
            print("✓✓✓ Hardware test PASSED! ✓✓✓")
            print("Your setup is working correctly.")
        
    finally:
        # Disable motor and clean up
        print()
        print("Cleaning up GPIO...")
        try:
            lgpio.gpio_write(h, ENABLE_PIN, 0)  # Disable motor (reverse of enable)
            time.sleep(0.1)
            lgpio.gpio_free(h, DIR_PIN)
            lgpio.gpio_free(h, STEP_PIN)
            lgpio.gpio_free(h, ENABLE_PIN)
            lgpio.gpiochip_close(h)
            print("✓ GPIO cleaned up, motor disabled")
        except Exception as e:
            print(f"Warning during cleanup: {e}")
    
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\nTest cancelled by user")
        sys.exit(1)
