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

#include "wmh_nema_focuser.h"
#include <cstring>
#include <cmath>
#include <memory>
#include <unistd.h>

// Maximum position (can be adjusted via settings)
#define MAX_STEPS 100000

std::unique_ptr<WMHNEMAFocuser> wmhNemaFocuser(new WMHNEMAFocuser());

WMHNEMAFocuser::WMHNEMAFocuser()
{
    setVersion(1, 0);
    FI::SetCapability(FOCUSER_CAN_ABS_MOVE |
                      FOCUSER_CAN_REL_MOVE |
                      FOCUSER_CAN_ABORT |
                      FOCUSER_CAN_SYNC);
}

WMHNEMAFocuser::~WMHNEMAFocuser()
{
    shutdownGPIO();
}

const char *WMHNEMAFocuser::getDefaultName()
{
    return "Waveshare NEMA Focuser";
}

bool WMHNEMAFocuser::initProperties()
{
    INDI::Focuser::initProperties();

    // Step delay in microseconds
    IUFillNumber(&StepDelayN[0], "STEP_DELAY", "Delay (μs)", "%.0f", 100, 10000, 100, 1000);
    IUFillNumberVector(&StepDelayNP, StepDelayN, 1, getDeviceName(), "STEP_SETTINGS", 
                       "Step Delay", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

    // Steps per revolution
    IUFillNumber(&StepsPerRevolutionN[0], "STEPS_PER_REV", "Steps/Rev", "%.0f", 100, 400, 1, 200);
    IUFillNumberVector(&StepsPerRevolutionNP, StepsPerRevolutionN, 1, getDeviceName(), 
                       "MOTOR_STEPS", "Motor Configuration", OPTIONS_TAB, IP_RW, 60, IPS_IDLE);

    // Motor selection
    IUFillSwitch(&MotorSelectS[0], "MOTOR_X", "Motor X", ISS_ON);
    IUFillSwitch(&MotorSelectS[1], "MOTOR_Y", "Motor Y", ISS_OFF);
    IUFillSwitch(&MotorSelectS[2], "MOTOR_Z", "Motor Z", ISS_OFF);
    IUFillSwitchVector(&MotorSelectSP, MotorSelectS, 3, getDeviceName(), "MOTOR_SELECT",
                       "Motor Channel", OPTIONS_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    // Microstepping mode
    IUFillSwitch(&MicroSteppingS[0], "FULL_STEP", "Full Step (1)", ISS_ON);
    IUFillSwitch(&MicroSteppingS[1], "HALF_STEP", "Half Step (1/2)", ISS_OFF);
    IUFillSwitch(&MicroSteppingS[2], "QUARTER_STEP", "Quarter Step (1/4)", ISS_OFF);
    IUFillSwitch(&MicroSteppingS[3], "EIGHTH_STEP", "Eighth Step (1/8)", ISS_OFF);
    IUFillSwitch(&MicroSteppingS[4], "SIXTEENTH_STEP", "Sixteenth Step (1/16)", ISS_OFF);
    IUFillSwitchVector(&MicrosteppingSP, MicroSteppingS, 5, getDeviceName(), "MICROSTEPPING",
                       "Microstepping", OPTIONS_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    // Set focuser parameters
    FocusMaxPosN[0].value = MAX_STEPS;
    FocusAbsPosN[0].min = 0;
    FocusAbsPosN[0].max = MAX_STEPS;
    FocusAbsPosN[0].value = 0;
    FocusAbsPosN[0].step = 100;

    FocusRelPosN[0].min = 0;
    FocusRelPosN[0].max = MAX_STEPS / 10;
    FocusRelPosN[0].value = 100;
    FocusRelPosN[0].step = 100;

    FocusSyncN[0].min = 0;
    FocusSyncN[0].max = MAX_STEPS;
    FocusSyncN[0].value = 0;
    FocusSyncN[0].step = 100;

    addDebugControl();

    return true;
}

bool WMHNEMAFocuser::updateProperties()
{
    INDI::Focuser::updateProperties();

    if (isConnected())
    {
        defineProperty(&StepDelayNP);
        defineProperty(&StepsPerRevolutionNP);
        defineProperty(&MotorSelectSP);
        defineProperty(&MicrosteppingSP);
    }
    else
    {
        deleteProperty(StepDelayNP.name);
        deleteProperty(StepsPerRevolutionNP.name);
        deleteProperty(MotorSelectSP.name);
        deleteProperty(MicrosteppingSP.name);
    }

    return true;
}

bool WMHNEMAFocuser::Connect()
{
    if (!initializeGPIO())
    {
        LOG_ERROR("Failed to initialize GPIO. Make sure pigpiod daemon is running.");
        return false;
    }

    enableMotor(false); // Keep motor disabled when not moving to save power
    
    LOG_INFO("Waveshare NEMA Focuser connected successfully.");
    return true;
}

bool WMHNEMAFocuser::Disconnect()
{
    enableMotor(false);
    shutdownGPIO();
    LOG_INFO("Waveshare NEMA Focuser disconnected.");
    return true;
}

bool WMHNEMAFocuser::initializeGPIO()
{
    // Initialize pigpio library
    if (gpioInitialise() < 0)
    {
        LOGF_ERROR("Failed to initialize pigpio library: %d", gpioInitialise());
        return false;
    }

    // Set up Motor X pins
    gpioSetMode(DIR_PIN_X, PI_OUTPUT);
    gpioSetMode(STEP_PIN_X, PI_OUTPUT);
    gpioSetMode(ENABLE_PIN_X, PI_OUTPUT);
    gpioWrite(ENABLE_PIN_X, 1); // Disable motor initially (active low)

    // Set up Motor Y pins
    gpioSetMode(DIR_PIN_Y, PI_OUTPUT);
    gpioSetMode(STEP_PIN_Y, PI_OUTPUT);
    gpioSetMode(ENABLE_PIN_Y, PI_OUTPUT);
    gpioWrite(ENABLE_PIN_Y, 1);

    // Set up Motor Z pins
    gpioSetMode(DIR_PIN_Z, PI_OUTPUT);
    gpioSetMode(STEP_PIN_Z, PI_OUTPUT);
    gpioSetMode(ENABLE_PIN_Z, PI_OUTPUT);
    gpioWrite(ENABLE_PIN_Z, 1);

    LOG_DEBUG("GPIO initialized successfully");
    return true;
}

void WMHNEMAFocuser::shutdownGPIO()
{
    // Disable all motors
    gpioWrite(ENABLE_PIN_X, 1);
    gpioWrite(ENABLE_PIN_Y, 1);
    gpioWrite(ENABLE_PIN_Z, 1);
    
    // Terminate pigpio
    gpioTerminate();
}

void WMHNEMAFocuser::setDirection(bool forward)
{
    // Set direction: HIGH = forward, LOW = reverse
    gpioWrite(currentDirPin, forward ? 1 : 0);
}

void WMHNEMAFocuser::enableMotor(bool enable)
{
    // Enable pin is active LOW
    gpioWrite(currentEnablePin, enable ? 0 : 1);
}

bool WMHNEMAFocuser::stepMotor()
{
    // Generate step pulse
    gpioWrite(currentStepPin, 1);
    gpioDelay(2); // 2 microsecond pulse
    gpioWrite(currentStepPin, 0);
    gpioDelay(stepDelay);
    
    return true;
}

IPState WMHNEMAFocuser::MoveAbsFocuser(uint32_t targetTicks)
{
    if (targetTicks == FocusAbsPosN[0].value)
    {
        return IPS_OK;
    }

    targetPosition = targetTicks;
    isMoving = true;

    // Set direction
    bool forward = (targetTicks > FocusAbsPosN[0].value);
    setDirection(forward);
    
    enableMotor(true);

    LOGF_DEBUG("Moving to absolute position: %u (current: %.0f)", 
               targetPosition, FocusAbsPosN[0].value);

    SetTimer(1); // Update every 1ms
    return IPS_BUSY;
}

IPState WMHNEMAFocuser::MoveRelFocuser(FocusDirection dir, uint32_t ticks)
{
    uint32_t newPosition;
    
    if (dir == FOCUS_INWARD)
    {
        newPosition = FocusAbsPosN[0].value - ticks;
        if ((int32_t)newPosition < 0)
            newPosition = 0;
    }
    else
    {
        newPosition = FocusAbsPosN[0].value + ticks;
        if (newPosition > FocusMaxPosN[0].value)
            newPosition = FocusMaxPosN[0].value;
    }

    return MoveAbsFocuser(newPosition);
}

bool WMHNEMAFocuser::AbortFocuser()
{
    isMoving = false;
    enableMotor(false);
    LOG_INFO("Focuser movement aborted");
    return true;
}

void WMHNEMAFocuser::TimerHit()
{
    if (!isMoving)
    {
        SetTimer(getCurrentPollingPeriod());
        return;
    }

    // Calculate how many steps we need to take
    uint32_t currentPos = static_cast<uint32_t>(FocusAbsPosN[0].value);
    
    if (currentPos == targetPosition)
    {
        // We've reached the target
        isMoving = false;
        enableMotor(false);
        FocusAbsPosNP.s = IPS_OK;
        IDSetNumber(&FocusAbsPosNP, nullptr);
        LOG_DEBUG("Target position reached");
        return;
    }

    // Take steps (batch multiple steps per timer hit for smoother movement)
    int stepsToTake = 10; // Take multiple steps per timer hit
    bool forward = (targetPosition > currentPos);
    
    for (int i = 0; i < stepsToTake; i++)
    {
        if (currentPos == targetPosition)
            break;
            
        stepMotor();
        
        if (forward)
            currentPos++;
        else
            currentPos--;
    }

    FocusAbsPosN[0].value = currentPos;
    IDSetNumber(&FocusAbsPosNP, nullptr);

    SetTimer(1);
}

bool WMHNEMAFocuser::ISNewNumber(const char *dev, const char *name, double values[], 
                                  char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Step delay setting
        if (strcmp(name, StepDelayNP.name) == 0)
        {
            IUUpdateNumber(&StepDelayNP, values, names, n);
            stepDelay = static_cast<unsigned int>(StepDelayN[0].value);
            StepDelayNP.s = IPS_OK;
            IDSetNumber(&StepDelayNP, nullptr);
            LOGF_INFO("Step delay set to %u μs", stepDelay);
            return true;
        }

        // Steps per revolution
        if (strcmp(name, StepsPerRevolutionNP.name) == 0)
        {
            IUUpdateNumber(&StepsPerRevolutionNP, values, names, n);
            stepsPerRevolution = static_cast<unsigned int>(StepsPerRevolutionN[0].value);
            StepsPerRevolutionNP.s = IPS_OK;
            IDSetNumber(&StepsPerRevolutionNP, nullptr);
            LOGF_INFO("Steps per revolution set to %u", stepsPerRevolution);
            return true;
        }
    }

    return INDI::Focuser::ISNewNumber(dev, name, values, names, n);
}

bool WMHNEMAFocuser::ISNewSwitch(const char *dev, const char *name, ISState *states, 
                                  char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Motor selection
        if (strcmp(name, MotorSelectSP.name) == 0)
        {
            IUUpdateSwitch(&MotorSelectSP, states, names, n);
            
            int selectedMotor = IUFindOnSwitchIndex(&MotorSelectSP);
            
            switch (selectedMotor)
            {
                case 0: // Motor X
                    currentDirPin = DIR_PIN_X;
                    currentStepPin = STEP_PIN_X;
                    currentEnablePin = ENABLE_PIN_X;
                    LOG_INFO("Selected Motor X");
                    break;
                case 1: // Motor Y
                    currentDirPin = DIR_PIN_Y;
                    currentStepPin = STEP_PIN_Y;
                    currentEnablePin = ENABLE_PIN_Y;
                    LOG_INFO("Selected Motor Y");
                    break;
                case 2: // Motor Z
                    currentDirPin = DIR_PIN_Z;
                    currentStepPin = STEP_PIN_Z;
                    currentEnablePin = ENABLE_PIN_Z;
                    LOG_INFO("Selected Motor Z");
                    break;
            }
            
            MotorSelectSP.s = IPS_OK;
            IDSetSwitch(&MotorSelectSP, nullptr);
            return true;
        }

        // Microstepping mode
        if (strcmp(name, MicrosteppingSP.name) == 0)
        {
            IUUpdateSwitch(&MicrosteppingSP, states, names, n);
            
            int selectedMode = IUFindOnSwitchIndex(&MicrosteppingSP);
            
            switch (selectedMode)
            {
                case 0: microsteppingMode = 1; break;
                case 1: microsteppingMode = 2; break;
                case 2: microsteppingMode = 4; break;
                case 3: microsteppingMode = 8; break;
                case 4: microsteppingMode = 16; break;
            }
            
            LOGF_INFO("Microstepping mode set to 1/%u", microsteppingMode);
            MicrosteppingSP.s = IPS_OK;
            IDSetSwitch(&MicrosteppingSP, nullptr);
            return true;
        }
    }

    return INDI::Focuser::ISNewSwitch(dev, name, states, names, n);
}

bool WMHNEMAFocuser::saveConfigItems(FILE *fp)
{
    INDI::Focuser::saveConfigItems(fp);

    IUSaveConfigNumber(fp, &StepDelayNP);
    IUSaveConfigNumber(fp, &StepsPerRevolutionNP);
    IUSaveConfigSwitch(fp, &MotorSelectSP);
    IUSaveConfigSwitch(fp, &MicrosteppingSP);

    return true;
}
