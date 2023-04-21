#include <LiquidCrystal_I2C.h>  // library for the LCD screen
#include <Wire.h>               // library for I2C communication
#include <RTClib.h>             // library for the real-time clock
#include <DHT.h>                // library for the DHT sensor
#include <EEPROM.h>

#define DHTPIN 4       // pin of the DHT sensor
#define DHTTYPE DHT22  // type of DHT sensor
#define FOGGER_PIN 6
#define PUMP_PIN 7
#define FAN_PIN 13
#define HEATER_PIN 12
#define LIGHT_PIN 11

#define VALIDATION_BUTTON_PIN 2  // pin of the validation button
#define BACK_BUTTON_PIN 3        // pin of the back button
#define POT_PIN A0               // pin of the analog potentiometer

// constants for optimal living conditions
int TEMPERATURE_LIMIT_NIGHT(20);  // HIGH temperature limit
int TEMPERATURE_LIMIT_DAY(24);    // LOW temperature limit
int HUMIDITY_LIMIT_NIGHT(80);     // HIGH humidity limit
int HUMIDITY_LIMIT_DAY(60);       // LOW humidity limit

// constants for light management
int SUNRISE_HOUR(8);  // hour of sunrise
int SUNSET_HOUR(18);  // hour of sunset

// constants for pump management
int PUMP_DURATION(7);   // in seconds
int PUMP_INTERVALS(8);  // in hours
int lastPumpTime;       // variable to store the time of the last use of the diaphragm pump

// constants for RTC and DHT
int hour, minute, day, month, year, time;
float humidity, temperature;

unsigned long lastUpdateTime(0);            // variable to store the time of the last update
const unsigned long updateInterval(60000);  // update interval in milliseconds (here, every minute)
bool displayTemperature(false);             // variable to store the display state (temperature/humidity or date/time)

DHT dht(DHTPIN, DHTTYPE);  // DHT object
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;  // RTC object

// constants for the configuration process
const int NUM_CONFIG_ITEMS(8);  // number of configurable items
const char* CONFIG_ITEMS[NUM_CONFIG_ITEMS] = {
  // names of the configurable items
  "NIGHT TEMP.", "DAY TEMPERATURE", "NIGHT HUMIDITY", "DAY HUMIDITY", "PUMP INTERVALS", "PUMP DURATION", "SUNRISE HOUR", "SUNSET HOUR",  // names of the configurable items
};

int configIndex(0);      // index of the currently selected configurable item
int configValue(0);      // value of the currently selected configurable item
bool configMode(false);  // flag to indicate if we are in config mode

void setup() {
  // initialize the LCD screen
  Serial.begin(9600);

  // initialize EEPROM with default values if not already done
  if (EEPROM.read(0) == 255) {
    EEPROM.update(0, TEMPERATURE_LIMIT_NIGHT);
    EEPROM.update(1, TEMPERATURE_LIMIT_DAY);
    EEPROM.update(2, HUMIDITY_LIMIT_NIGHT);
    EEPROM.update(3, HUMIDITY_LIMIT_DAY);
    EEPROM.update(4, SUNRISE_HOUR);
    EEPROM.update(5, SUNSET_HOUR);
    EEPROM.update(6, PUMP_DURATION);
    EEPROM.update(7, PUMP_INTERVALS);
  }

  // read values from EEPROM
  TEMPERATURE_LIMIT_NIGHT = EEPROM.read(0);
  TEMPERATURE_LIMIT_DAY = EEPROM.read(1);
  HUMIDITY_LIMIT_NIGHT = EEPROM.read(2);
  HUMIDITY_LIMIT_DAY = EEPROM.read(3);
  SUNRISE_HOUR = EEPROM.read(4);
  SUNSET_HOUR = EEPROM.read(5);
  PUMP_DURATION = EEPROM.read(6);
  PUMP_INTERVALS = EEPROM.read(7);

  lcd.init();
  lcd.clear();
  lcd.backlight();
  // initialize I2C communication
  Wire.begin();
  rtc.begin();
  rtc.adjust(DateTime(__DATE__, __TIME__));

  // initialize the DHT sensor
  dht.begin();
  // initialize the pins of the fogger, pump, potentiometer, and fan as outputs
  pinMode(FOGGER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT_PULLUP);
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(VALIDATION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACK_BUTTON_PIN, INPUT_PULLUP);
  lastUpdateTime = 60000;           // initialize the last update time variable
  lastPumpTime = rtc.now().hour();  // initialize the lastPumpTime variable with the current hour

  // initialize relays
  digitalWrite(FOGGER_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(HEATER_PIN, HIGH);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(LIGHT_PIN, HIGH);
  delay(1000);
  digitalWrite(FOGGER_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(2), interruptValidation, LOW);
  attachInterrupt(digitalPinToInterrupt(3), interruptBack, LOW);

  // display a message asking the user to press the validation button to enter config mode
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  TinyTerra v1  ");
  lcd.setCursor(0, 1);
  lcd.print("please wait...");
  delay(1000);

  // display a message asking the user to press the validation button to enter config mode
  Serial.println("asking for enter in config mode");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press VALIDATION");
  lcd.setCursor(0, 1);
  lcd.print("to enter config");
  unsigned long startTime = millis();    // store the current time
  while (millis() - startTime < 3000) {  // wait for 5 seconds
    // check if the validation button is pressed
    if (digitalRead(VALIDATION_BUTTON_PIN) == LOW) {
      configMode = true;
      configIndex = -1;
      break;  // exit the loop
    }
  }
  lcd.clear();
  Serial.println("config skipped");
}


unsigned long lastButtonPress(0);
unsigned long lastValidationPress(0);
unsigned long lastBackPress(0);

byte configIndexLCD(0);
bool dataDisplay(false);

void interruptValidation() {
  lastButtonPress = millis();

  if (digitalRead(VALIDATION_BUTTON_PIN) == LOW) {
    // Check if at least 100ms have passed since the last button press
    if (millis() - lastValidationPress > 100) {
      // Debounce successful, take action
      if (configMode == true) {
        // go to the next configurable item
        configIndex++;
        // if we reached the end of the list, exit config mode
        if (configIndex >= NUM_CONFIG_ITEMS) {
          configMode = false;
          // reset the config value to 0
          EEPROM.update(0, TEMPERATURE_LIMIT_NIGHT);
          EEPROM.update(1, TEMPERATURE_LIMIT_DAY);
          EEPROM.update(2, HUMIDITY_LIMIT_NIGHT);
          EEPROM.update(3, HUMIDITY_LIMIT_DAY);
          EEPROM.update(7, PUMP_INTERVALS);
          EEPROM.update(6, PUMP_DURATION);
          EEPROM.update(4, SUNRISE_HOUR);
          EEPROM.update(5, SUNSET_HOUR);
          return;
        }
      } else if (dataDisplay == true && configIndexLCD == 2) {
        configIndexLCD = 0;
      } else if (dataDisplay == true) {
        configIndexLCD++;
      }
      // Update lastValidationPress variable
      lastValidationPress = millis();
    }
  }
}

void interruptBack() {
  lastButtonPress = millis();

  if (digitalRead(BACK_BUTTON_PIN) == LOW) {
    // Check if at least 100ms have passed since the last button press
    if (millis() - lastBackPress > 100) {
      // Debounce successful, take action
      if (configMode == true) {
        configIndex--;
        // if we reached the beginning of the list, exit config mode
        if (configIndex < 0) {
          configMode = false;
        }
      } else if (dataDisplay == true && configIndexLCD == 0) {
        configIndexLCD = 2;
      } else if (dataDisplay == true) {
        configIndexLCD--;
      }
      // Update lastBackPress variable
      lastBackPress = millis();
    }
  }
}

void configure() {
  // display the name of the current configurable item
  Serial.println("in config mode");
  Serial.print("Temperature limit (night): ");
  Serial.println(TEMPERATURE_LIMIT_NIGHT);
  Serial.print("Temperature limit (day): ");
  Serial.println(TEMPERATURE_LIMIT_DAY);
  Serial.print("Humidity limit (night): ");
  Serial.println(HUMIDITY_LIMIT_NIGHT);
  Serial.print("Humidity limit (day): ");
  Serial.println(HUMIDITY_LIMIT_DAY);
  Serial.print("Sunrise hour: ");
  Serial.println(SUNRISE_HOUR);
  Serial.print("Sunset hour: ");
  Serial.println(SUNSET_HOUR);
  Serial.print("Pump duration: ");
  Serial.println(PUMP_DURATION);
  Serial.print("Pump intervals: ");
  Serial.println(PUMP_INTERVALS);

  lcd.setCursor(0, 0);
  lcd.print(CONFIG_ITEMS[configIndex]);
  lcd.print("               ");
  // display the current value of the configurable item
  lcd.setCursor(0, 1);
  lcd.print(configValue);
  lcd.print("               ");
  // check if the validation button is pressed

  // update the value of the configurable item based on its index
  switch (configIndex) {
    case 0:
      TEMPERATURE_LIMIT_NIGHT = configValue;
      break;
    case 1:
      TEMPERATURE_LIMIT_DAY = configValue;
      break;
    case 2:
      HUMIDITY_LIMIT_NIGHT = configValue;
      break;
    case 3:
      HUMIDITY_LIMIT_DAY = configValue;
      break;
    case 4:
      PUMP_INTERVALS = configValue;
      break;
    case 5:
      PUMP_DURATION = configValue;
      break;
    case 6:
      SUNRISE_HOUR = configValue;
      break;
    case 7:
      SUNSET_HOUR = configValue;
      break;
  }

  // check potentiometer position
  int potValue = analogRead(POT_PIN);
  if (configIndex == 4 || configIndex == 6 || configIndex == 7) {
    configValue = map(potValue, 0, 1023, 0, 24);  // hours
  } else if (configIndex == 5) {
    configValue = map(potValue, 0, 1023, 0, 120);  // secondes
  } else {
    configValue = map(potValue, 0, 1023, 0, 100);  // percents
  }
}

void loop() {
  time = hour * 60 + minute;
  // check if we are in config mode
  if (configMode) {
    // handle the configuration process
    configure();
    return;
  } else if (digitalRead(VALIDATION_BUTTON_PIN) == LOW && digitalRead(BACK_BUTTON_PIN) == LOW) {
    lcd.init();
    lcd.clear();
    lcd.backlight();
    configMode = true;
    configIndex = 0;
    return;
  }

  // get hour, minute and date from RTC
  DateTime now = rtc.now();
  hour = now.hour();
  minute = now.minute();
  day = now.day();
  month = now.month();
  year = now.year();

  // get the current temperature and humidity values from the DHT sensor
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  delay(200);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("NAN error with DHT sensor !");
  } else {
    Serial.print("Temp : ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("Hum  : ");
    Serial.print(humidity);
    Serial.println("%");
  }

  Serial.print("Time: ");
  if (hour < 10) {
    Serial.print("0");
  }
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) {
    Serial.print("0");
  }
  Serial.println(minute);
  Serial.print("Date: ");
  Serial.print(day);
  Serial.print("/");
  Serial.print(month);
  Serial.print("/");
  Serial.println(year);

  // constants for optimal living conditions
  Serial.print("Day temperature limit : ");
  Serial.print(TEMPERATURE_LIMIT_DAY - (TEMPERATURE_LIMIT_DAY * 0.025));
  Serial.print(" and ");
  Serial.println(TEMPERATURE_LIMIT_DAY + (TEMPERATURE_LIMIT_DAY * 0.025));
  Serial.print("Night temperature limit : ");
  Serial.print(TEMPERATURE_LIMIT_NIGHT - (TEMPERATURE_LIMIT_NIGHT * 0.025));
  Serial.print(" and ");
  Serial.println(TEMPERATURE_LIMIT_NIGHT + (TEMPERATURE_LIMIT_NIGHT * 0.025));
  Serial.print("Day humidity limit : ");
  Serial.print(HUMIDITY_LIMIT_DAY - (HUMIDITY_LIMIT_DAY * 0.025));
  Serial.print(" and ");
  Serial.println(HUMIDITY_LIMIT_DAY + (HUMIDITY_LIMIT_DAY * 0.025));
  Serial.print("Night humidity limit : ");
  Serial.print(HUMIDITY_LIMIT_NIGHT - (HUMIDITY_LIMIT_NIGHT * 0.025));
  Serial.print(" and ");
  Serial.println(HUMIDITY_LIMIT_NIGHT + (HUMIDITY_LIMIT_NIGHT * 0.025));

  // update the light
  updateLiving();
  displayLCD();
  delay(1000);
}

void updateLiving() {
  if (hour >= SUNRISE_HOUR && hour < SUNSET_HOUR) {
    // during the day, the light is ON
    digitalWrite(LIGHT_PIN, HIGH);
    Serial.println("☀");

    // check day temperature
    if (temperature < TEMPERATURE_LIMIT_DAY - (TEMPERATURE_LIMIT_DAY * 0.025)) {
      digitalWrite(HEATER_PIN, HIGH);
      Serial.print("Heater : ");
      Serial.println("✔");
    } else {
      digitalWrite(HEATER_PIN, LOW);
      Serial.print("Heater : ");
      Serial.println("✘");
    }

    // check day humidity
    if (humidity < HUMIDITY_LIMIT_DAY - (HUMIDITY_LIMIT_DAY * 0.025)) {
      digitalWrite(FOGGER_PIN, HIGH);
      Serial.print("Fogger : ");
      Serial.println("✔");
    } else {
      digitalWrite(FOGGER_PIN, LOW);
      Serial.print("Fogger : ");
      Serial.println("✘");
    }

    // check temperature and humidity limit
    if (temperature > TEMPERATURE_LIMIT_DAY + (TEMPERATURE_LIMIT_DAY * 0.025) || humidity > HUMIDITY_LIMIT_DAY + (HUMIDITY_LIMIT_DAY * 0.025)) {
      digitalWrite(FAN_PIN, HIGH);
      Serial.print("Fan : ");
      Serial.println("✔");
    } else {
      digitalWrite(FAN_PIN, LOW);
      Serial.print("Fan : ");
      Serial.println("✘");
    }

  } else {
    // during the night, the light is OFF
    digitalWrite(LIGHT_PIN, LOW);
    Serial.println("🌑");

    // check night temperature
    if (temperature < TEMPERATURE_LIMIT_NIGHT - (TEMPERATURE_LIMIT_NIGHT * 0.025)) {
      digitalWrite(HEATER_PIN, HIGH);
      Serial.print("Heater : ");
      Serial.println("✔");
    } else {
      digitalWrite(HEATER_PIN, LOW);
      Serial.print("Heater : ");
      Serial.println("✘");
    }

    // check night humidity
    if (humidity < HUMIDITY_LIMIT_NIGHT - (HUMIDITY_LIMIT_NIGHT * 0.025)) {
      digitalWrite(FOGGER_PIN, HIGH);
      Serial.print("Fogger : ");
      Serial.println("✔");
    } else {
      digitalWrite(FOGGER_PIN, LOW);
      Serial.print("Fogger : ");
      Serial.println("✘");
    }

    // check temperature and humidity limit
    if (temperature > TEMPERATURE_LIMIT_NIGHT + (TEMPERATURE_LIMIT_NIGHT * 0.025) || humidity > HUMIDITY_LIMIT_NIGHT + (HUMIDITY_LIMIT_NIGHT * 0.025)) {
      digitalWrite(FAN_PIN, HIGH);
      Serial.print("Fan : ");
      Serial.println("✔");
    } else {
      digitalWrite(FAN_PIN, LOW);
      Serial.print("Fan : ");
      Serial.println("✘");
    }
  }
  if (hour % PUMP_INTERVALS == 0 && (hour != lastPumpTime)) {
    lastPumpTime = hour;  // update the last pump time
                          // turn on the pump
    Serial.print("Pump : ");
    Serial.println("✔");
    digitalWrite(PUMP_PIN, HIGH);
    delay(PUMP_DURATION * 1000);
    digitalWrite(PUMP_PIN, LOW);
  }
  Serial.print("Pump : ");
  Serial.println("✘");
}

void displayLCD() {
  lcd.clear();
  if (millis() - lastButtonPress > 30000) {
    lcd.noBacklight();  // turn off backlight
    dataDisplay = false;
  } else if (millis() - lastButtonPress < 30000) {
    dataDisplay = true;
    lcd.backlight();  // turn on backlight
    // update the value of the configurable item based on its index
    switch (configIndexLCD) {
      case 0:
        // update the display with the temperature and humidity
        lcd.print("Temperature : ");
        lcd.setCursor(0, 1);
        lcd.print(temperature);
        lcd.print((char)223);  // degree symbol
        lcd.print("C");
        break;
      case 1:
        // update the display with the temperature and humidity
        lcd.print("Humidity : ");
        lcd.setCursor(0, 1);
        lcd.print(humidity);
        lcd.print("%");
        break;
      case 2:
        // update the display with the date and time
        lcd.print("Time: ");
        if (hour < 10) {
          lcd.print("0");
        }
        lcd.print(hour);
        lcd.print(":");
        if (minute < 10) {
          lcd.print("0");
        }
        lcd.print(minute);
        lcd.setCursor(0, 1);
        lcd.print("Date: ");
        if (day < 10) {
          lcd.print("0");
        }
        lcd.print(day);
        lcd.print("/");
        if (month < 10) {
          lcd.print("0");
        }
        lcd.print(month);
        lcd.print("/");
        lcd.print(year);
        break;
    }
  }
}
