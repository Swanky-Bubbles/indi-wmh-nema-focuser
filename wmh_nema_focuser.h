/*******************************************************************************
  Copyright(c) 2025 Danny McGee. All rights reserved.

  Waveshare Stepper Motor HAT (B) NEMA Focuser Driver

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#pragma once

#include <indifocuser.h>
#include <pigpio.h>

class WMHNEMAFocuser : public INDI::Focuser
{
public:
    WMHNEMAFocuser();
    virtual ~WMHNEMAFocuser() override;

    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool saveConfigItems(FILE *fp) override;

protected:
    virtual bool Connect() override;
    virtual bool Disconnect() override;
    
    virtual IPState MoveAbsFocuser(uint32_t targetTicks) override;
    virtual IPState MoveRelFocuser(FocusDirection dir, uint32_t ticks) override;
    virtual bool AbortFocuser() override;
    virtual void TimerHit() override;

private:
    // GPIO Pins for Waveshare Stepper Motor HAT (B)
    // Motor 1 (X-axis)
    static constexpr int DIR_PIN_X = 13;
    static constexpr int STEP_PIN_X = 19;
    static constexpr int ENABLE_PIN_X = 12;
    
    // Motor 2 (Y-axis) - if needed for second motor
    static constexpr int DIR_PIN_Y = 24;
    static constexpr int STEP_PIN_Y = 18;
    static constexpr int ENABLE_PIN_Y = 4;
    
    // Motor 3 (Z-axis) - if needed
    static constexpr int DIR_PIN_Z = 21;
    static constexpr int STEP_PIN_Z = 26;
    static constexpr int ENABLE_PIN_Z = 25;

    // Stepper motor control
    bool initializeGPIO();
    void shutdownGPIO();
    void setDirection(bool forward);
    void enableMotor(bool enable);
    bool stepMotor();
    
    // Movement tracking
    uint32_t targetPosition { 0 };
    bool isMoving { false };
    
    // Motor configuration
    INumberVectorProperty StepDelayNP;
    INumber StepDelayN[1];
    
    INumberVectorProperty StepsPerRevolutionNP;
    INumber StepsPerRevolutionN[1];
    
    ISwitchVectorProperty MotorSelectSP;
    ISwitch MotorSelectS[3];
    
    ISwitchVectorProperty MicrosteppingSP;
    ISwitch MicroSteppingS[5]; // Full, 1/2, 1/4, 1/8, 1/16
    
    // Current motor selection
    int currentDirPin { DIR_PIN_X };
    int currentStepPin { STEP_PIN_X };
    int currentEnablePin { ENABLE_PIN_X };
    
    // Step delay in microseconds
    unsigned int stepDelay { 1000 };
    
    // Steps per revolution (200 for NEMA 17 with 1.8° step angle)
    unsigned int stepsPerRevolution { 200 };
    
    // Microstepping mode (1, 2, 4, 8, 16)
    unsigned int microsteppingMode { 1 };
    
    // Timer for movement
    int timerID { -1 };
};
