# TinyTerra
Try it on Wokwi : https://wokwi.com/projects/358678485846549505

                                  .#
                            ##   #
                              %###   (         %#####
                              ,###        ############
                          ##    ###      %#############
                          .#   # ##%%#################%
                      %.  ##%#        %###############  #    %#
                          ##.%##     ##########%####   %%.%%
                          /%%#%%% /%(%#((%#%(#(%( (   %(##%##
                          (##%##%(##(##(%((##%#  %%#%#
                          ####*##############%
                          %### #############.
                           ### ###########
                            ### ######% ##########( %
                                         ######,/####%#%#
                                    ######## %#######
                                  #########%#         %

                         ---<< WELCOME TO TINYTERRA >>---
                         
The system constantly measures the temperature and humidity inside the greenhouse/terrarium and uses this data to determine
when to turn on/off the different devices. It also allows the user to configure the optimal temperature and humidity
ranges for different times of the day, as well as the intervals at which the pump should be activated.

---> Hardware
  Arduino Uno R3
  LCD screen 1602
  DHT22 sensor
  DS3231 RTC module
  x5 Relay module
  Potentiometer
  Two push buttons

---> Libraries
  LiquidCrystal_I2C.h
  Wire.h
  RTClib.h
  DHT.h
  EEPROM.h
  
---> Pin Assignments
  DHT22 sensor: pin 4
  Diaphragm pump: pin 7
  Fogger: pin 6
  Fan: pin 13
  Heater: pin 12
  Light: pin 11
  Potentiometer: pin A0
  Validation button: pin 2
  Back button: pin 3

---> Functionality
  Monitors the temperature and humidity inside the greenhouse using the DHT22 sensor.
  Uses the RTC module to keep track of the time and date.
  Controls the fogger, pump, fan, heater, and light relays to maintain optimal environmental conditions for plant growth.
  Allows the user to configure the optimal temperature and humidity ranges for different times of the day using the potentiometer and push buttons.
  Activates the pump at regular intervals (configurable by the user) to irrigate the plants.
  Displays the temperature, humidity, time, and date on the LCD screen and allows the user to switch between information using the push buttons.

---> Configuration Mode
  Night temperature limit (°C)
  Day temperature limit (°C)
  Night humidity limit (%)
  Day humidity limit (%)
  Pump intervals (hours)
  Pump duration (seconds)
  Sunrise hour
  Sunset hour
  The configuration mode is accessed by pressing the back button for 3 seconds. In configuration mode, the LCD screen displays the name of the configurable item and its current value. The user can then use the potentiometer to adjust the value and the validation button to save the new value.

---> EEPROM
The system uses the EEPROM library to store the values of the configurable parameters.
If the EEPROM has not been initialized, the system initializes it with default values.
