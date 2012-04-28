/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2011 Free Software Foundation, Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: <postmaster@poes-weather.com>
    Web: <http://www.poes-weather.com>
*/
//---------------------------------------------------------------------------
#ifndef JRK_PROTOCOL_H
#define JRK_PROTOCOL_H

#define JRK_IO_BUF_SIZE 64
#define JRK_VENDOR      0x1ffb
#define JRK_12v12       0x0085
#define JRK_21v3        0x0083

#define JRK_REQUEST_GET_TYPE        0xC0
#define JRK_REQUEST_SET_TYPE        0x40

#define JRK_REQUEST_GET_PARAMETER   0x81
#define JRK_REQUEST_SET_PARAMETER   0x82
#define JRK_REQUEST_GET_VARIABLES   0x83
#define JRK_REQUEST_SET_TARGET      0x84
#define JRK_REQUEST_CLEAR_ERRORS    0x86
#define JRK_REQUEST_MOTOR_OFF       0x87
#define JRK_REQUEST_REINITIALIZE    0x90

typedef struct jrk_variables_t
{
    unsigned short input;               // Offset 1
    unsigned short target;              // Offset 3
    unsigned short feedback;            // Offset 5
    unsigned short scaledFeedback;      // Offset 7
    short errorSum;                     // Offset 9
    short dutyCycleTarget;              // Offset 11
    short dutyCycle;                    // Offset 13
    unsigned char current;              // Offset 15
    unsigned char pidPeriodExceeded;    // Offset 16
    unsigned short pidPeriodCount;      // Offset 17
    unsigned short errorFlagBits;       // Offset 19
    unsigned short errorOccurredBits;   // Offset 21
} jrk_variables;

typedef struct jrk_pid_variables_t
{
    double cP, cI, cD; // user input coefficients
    double error;       // current error = scaled - target
    double integral;    // sum of all errors
    double derivative;  // current - previous error
} jrk_pid_variables;


#define JRK_PARAMETER_INPUT_MODE                        1  // 1 byte unsigned value.  Valid values are INPUT_MODE_*.  Init parameter.
#define JRK_PARAMETER_INPUT_MINIMUM                     2  // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_INPUT_MAXIMUM                     6  // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_OUTPUT_MINIMUM                    8  // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_OUTPUT_NEUTRAL                    10 // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_OUTPUT_MAXIMUM                    12 // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_INPUT_INVERT                      16 // 1 bit boolean value
#define JRK_PARAMETER_INPUT_SCALING_DEGREE              17 // 1 bit boolean value
#define JRK_PARAMETER_INPUT_POWER_WITH_AUX              18 // 1 bit boolean value
#define JRK_PARAMETER_INPUT_ANALOG_SAMPLES_EXPONENT     20 // 1 byte unsigned value, 0-8 - averages together 4 * 2^x samples
#define JRK_PARAMETER_INPUT_DISCONNECT_MINIMUM          22 // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_INPUT_DISCONNECT_MAXIMUM          24 // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_INPUT_NEUTRAL_MAXIMUM             26 // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_INPUT_NEUTRAL_MINIMUM             28 // 2 byte unsigned value (0-4095)

#define JRK_PARAMETER_SERIAL_MODE                       30 // 1 byte unsigned value.  Valid values are SERIAL_MODE_*.  MUST be SERIAL_MODE_USB_DUAL_PORT if INPUT_MODE!=INPUT_MODE_SERIAL.  Init parameter.
#define JRK_PARAMETER_SERIAL_FIXED_BAUD_RATE            31 // 2-byte unsigned value; 0 means autodetect.  Init parameter.
#define JRK_PARAMETER_SERIAL_TIMEOUT                    34 // 2-byte unsigned value
#define JRK_PARAMETER_SERIAL_ENABLE_CRC                 36 // 1 bit boolean value
#define JRK_PARAMETER_SERIAL_NEVER_SUSPEND              37 // 1 bit boolean value
#define JRK_PARAMETER_SERIAL_DEVICE_NUMBER              38 // 1 byte unsigned value, 0-127

#define JRK_PARAMETER_FEEDBACK_MODE                     50 // 1 byte unsigned value.  Valid values are FEEDBACK_MODE_*.  Init parameter.
#define JRK_PARAMETER_FEEDBACK_MINIMUM                  51 // 2 byte unsigned value
#define JRK_PARAMETER_FEEDBACK_MAXIMUM                  53 // 2 byte unsigned value
#define JRK_PARAMETER_FEEDBACK_INVERT                   55 // 1 bit boolean value
#define JRK_PARAMETER_FEEDBACK_POWER_WITH_AUX           57 // 1 bit boolean value
#define JRK_PARAMETER_FEEDBACK_DEAD_ZONE                58 // 1 byte unsigned value
#define JRK_PARAMETER_FEEDBACK_ANALOG_SAMPLES_EXPONENT  59 // 1 byte unsigned value, 0-8 - averages together 4 * 2^x samples
#define JRK_PARAMETER_FEEDBACK_DISCONNECT_MINIMUM       61 // 2 byte unsigned value (0-4095)
#define JRK_PARAMETER_FEEDBACK_DISCONNECT_MAXIMUM       63 // 2 byte unsigned value (0-4095)

#define JRK_PARAMETER_PROPORTIONAL_MULTIPLIER           70 // 2 byte unsigned value (0-1023)
#define JRK_PARAMETER_PROPORTIONAL_EXPONENT             72 // 1 byte unsigned value (0-15)
#define JRK_PARAMETER_INTEGRAL_MULTIPLIER               73 // 2 byte unsigned value (0-1023)
#define JRK_PARAMETER_INTEGRAL_EXPONENT                 75 // 1 byte unsigned value (0-15)
#define JRK_PARAMETER_DERIVATIVE_MULTIPLIER             76 // 2 byte unsigned value (0-1023)
#define JRK_PARAMETER_DERIVATIVE_EXPONENT               78 // 1 byte unsigned value (0-15)
#define JRK_PARAMETER_PID_PERIOD                        79 // 2 byte unsigned value
#define JRK_PARAMETER_PID_INTEGRAL_LIMIT                81 // 2 byte unsigned value
#define JRK_PARAMETER_PID_RESET_INTEGRAL                84 // 1 bit boolean value

#define JRK_PARAMETER_MOTOR_PWM_FREQUENCY               100 // 1 byte unsigned value.  Valid values are 0 = 20kHz, 1 = 5kHz.  Init parameter.
#define JRK_PARAMETER_MOTOR_INVERT                      101 // 1 bit boolean value

// WARNING: The EEPROM initialization assumes the 5 parameters below are consecutive!
#define JRK_PARAMETER_MOTOR_MAX_DUTY_CYCLE_WHILE_FEEDBACK_OUT_OF_RANGE 102 // 2 byte unsigned value (0-600)
#define JRK_PARAMETER_MOTOR_MAX_ACCELERATION_FORWARD    104 // 2 byte unsigned value (1-600)
#define JRK_PARAMETER_MOTOR_MAX_ACCELERATION_REVERSE    106 // 2 byte unsigned value (1-600)
#define JRK_PARAMETER_MOTOR_MAX_DUTY_CYCLE_FORWARD      108 // 2 byte unsigned value (0-600)
#define JRK_PARAMETER_MOTOR_MAX_DUTY_CYCLE_REVERSE      110 // 2 byte unsigned value (0-600)
// WARNING: The EEPROM initialization assumes the 5 parameters above are consecutive!

// WARNING: The EEPROM initialization assumes the 2 parameters below are consecutive!
#define JRK_PARAMETER_MOTOR_MAX_CURRENT_FORWARD         112 // 1 byte unsigned value (units of current_calibration_forward)
#define JRK_PARAMETER_MOTOR_MAX_CURRENT_REVERSE         113 // 1 byte unsigned value (units of current_calibration_reverse)
// WARNING: The EEPROM initialization assumes the 2 parameters above are consecutive!

// WARNING: The EEPROM initialization assumes the 2 parameters below are consecutive!
#define JRK_PARAMETER_MOTOR_CURRENT_CALIBRATION_FORWARD 114 // 1 byte unsigned value (units of mA)
#define JRK_PARAMETER_MOTOR_CURRENT_CALIBRATION_REVERSE 115 // 1 byte unsigned value (units of mA)
// WARNING: The EEPROM initialization assumes the 2 parameters above are consecutive!

#define JRK_PARAMETER_MOTOR_BRAKE_DURATION_FORWARD      116 // 1 byte unsigned value (units of 5 ms)
#define JRK_PARAMETER_MOTOR_BRAKE_DURATION_REVERSE      117 // 1 byte unsigned value (units of 5 ms)
#define JRK_PARAMETER_MOTOR_COAST_WHEN_OFF              118 // 1 bit boolean value (coast=1, brake=0)

#define JRK_PARAMETER_ERROR_ENABLE                      130 // 2 byte unsigned value.  See below for the meanings of the bits.
#define JRK_PARAMETER_ERROR_LATCH                       132 // 2 byte unsigned value.  See below for the meanings of the bits.


#endif // JRK_PROTOCOL_H
