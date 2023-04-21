# TinyTerra

TinyTerra is a system designed to control the environmental conditions inside a greenhouse or terrarium to maintain optimal conditions for plants and animals. It measures the temperature and humidity inside the greenhouse/terrarium using a DHT22 sensor and uses this data to turn on/off different devices. The system allows the user to configure the optimal temperature and humidity ranges for different times of the day, as well as the intervals at which the pump should be activated.

## Try it on Wokwi

You can try TinyTerra on Wokwi using this [link](https://wokwi.com/projects/358678485846549505).

## Hardware

The following hardware components are used in the TinyTerra system:

- Arduino Uno R3
- LCD screen 1602
- DHT22 sensor
- DS3231 RTC module
- x5 Relay module
- Potentiometer
- Two push buttons

## Libraries

The following libraries are used in the TinyTerra system:

- LiquidCrystal_I2C.h
- Wire.h
- RTClib.h
- DHT.h
- EEPROM.h

## Pin Assignments

The following pins are assigned to the hardware components in the TinyTerra system:

- DHT22 sensor: pin 4
- Diaphragm pump: pin 7
- Fogger: pin 6
- Fan: pin 13
- Heater: pin 12
- Light: pin 11
- Potentiometer: pin A0
- Validation button: pin 2
- Back button: pin 3

## Functionality

The TinyTerra system provides the following functionality:

- Monitors the temperature and humidity inside the greenhouse using the DHT22 sensor.
- Uses the RTC module to keep track of the time and date.
- Controls the fogger, pump, fan, heater, and light relays to maintain optimal environmental conditions for plant growth.
- Allows the user to configure the optimal temperature and humidity ranges for different times of the day using the potentiometer and push buttons.
- Activates the pump at regular intervals (configurable by the user) to irrigate the plants.
- Displays the temperature, humidity, time, and date on the LCD screen and allows the user to switch between information using the push buttons.

## Configuration Mode

TinyTerra has a configuration mode that allows the user to adjust the following parameters:

- Night temperature limit (°C)
- Day temperature limit (°C)
- Night humidity limit (%)
- Day humidity limit (%)
- Pump intervals (hours)
- Pump duration (seconds)
- Sunrise hour
- Sunset hour

The configuration mode is accessed by pressing the back button for 3 seconds. In configuration mode, the LCD screen displays the name of the configurable item and its current value. The user can then use the potentiometer to adjust the value and the validation button to save the new value.

## EEPROM

TinyTerra uses the EEPROM library to store the values of the configurable parameters. If the EEPROM has not been initialized, the system initializes it with default values.
