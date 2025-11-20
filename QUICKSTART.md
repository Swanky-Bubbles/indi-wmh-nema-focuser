# Quick Reference Card

## Installation (One-time setup)

```bash
./install.sh
# Then logout and login again
sudo systemctl restart indiserver
```

## Hardware Test

```bash
./test_hardware.sh
```

## Usage in KStars/Ekos

1. **Profile Editor** → Add "Waveshare NEMA Focuser" 
2. **Start Profile** → Connect to focuser
3. **Focus Module** → Use absolute/relative controls

## Common Settings

### Speed Configuration
- **Slow/Precise**: Step Delay = 5000 μs
- **Medium**: Step Delay = 2000 μs  
- **Fast**: Step Delay = 1000 μs

### Motor Selection
- **Motor X**: Default (GPIO 12, 13, 19)
- **Motor Y**: Alternative (GPIO 4, 24, 18)
- **Motor Z**: Alternative (GPIO 25, 21, 26)

### Microstepping
Match software setting to HAT jumper configuration:
- Full Step: 200 steps/rev
- 1/2 Step: 400 steps/rev
- 1/4 Step: 800 steps/rev
- 1/8 Step: 1600 steps/rev
- 1/16 Step: 3200 steps/rev

## GPIO Pin Reference

| Motor | DIR | STEP | ENABLE |
|-------|-----|------|--------|
| X     | 13  | 19   | 12     |
| Y     | 24  | 18   | 4      |
| Z     | 21  | 26   | 25     |

## Troubleshooting Quick Fixes

### Driver not listed
```bash
sudo systemctl restart indiserver
```

### GPIO errors
```bash
sudo systemctl start pigpiod
sudo systemctl enable pigpiod
```

### Permission denied
```bash
sudo usermod -a -G gpio $USER
# Logout and login
```

### Motor doesn't move
1. Check power supply
2. Verify correct motor channel selected
3. Increase step delay
4. Run `./test_hardware.sh`

## Manual Driver Start (for debugging)

```bash
# With verbose logging
INDI_DEBUG=5 /usr/bin/indi_wmh_nema_focuser

# Normal operation
/usr/bin/indi_wmh_nema_focuser
```

## File Locations

- Binary: `/usr/bin/indi_wmh_nema_focuser`
- XML: `/usr/share/indi/indi_wmh_nema_focuser.xml`
- Config: `~/.indi/Waveshare_NEMA_Focuser_config.xml`

## Support

See README.md for detailed documentation.
