#include <avr/sleep.h>  // Including the avr/sleep.h library for the sleep function.
#include "avr/wdt.h"  // Including the avr/wdt.h library for watchdog timer.

#define SM_Pin A0  // Defines which analog input pin you are using.
#define XBee_Pin 3  // Defines which digital pin the XBee is connected to for sleeping.
#define SM_Power_Pin 5  // Defines which digital pin will be used to sleep the soil moisture sensor.

int Arduino_Number = 2;  // Defines the Ardunio number.
int SM_Air_Value = 845;  // Initiates a constant for the calibrated fresh air value.
int SM_Water_Value = 395;  // Initiates a constant for the calibrated full water value.
int SM_Calibrated_Difference = SM_Air_Value - SM_Water_Value;  // Computes the difference between the two calibrated values.
int SM_Analog_Value;  // Defines the soil moisture analog value variable.
int SM_Difference;  // Defines the soil moisture difference value variable.
int SM_Percent;  // Defines the soil moisture percent value variable.
float SM_Difference_Abs;  // Defines the soil moisture absolute difference value variable.
float SM_Decimal;  // Defines the soil moisture decimal value variable.
char Value_Text[50];  // Defines the character variable used for the sprintf.

ISR(WDT_vect) {  // An ISR function for watchdogs.
  wdt_reset();  // Resets the watchdog timer when called upon.
}

void setup() {  // Setup code that only runs once.
  Serial.begin(9600);  // Starts running the serial monitor.
  pinMode(XBee_Pin, OUTPUT);  // Sets the XBee pin as an output for signaling the sleep mode.
  pinMode(SM_Power_Pin, OUTPUT);  // Sets the soil moisture power pin to output for signaling sleep.
  wdt_disable();  // Disables the watchdog timer at the beginning to ensure that it is not enabled to mess with the rest of the code.
}

void loop() {  // Loop code that runs on a loop continuously.
  int sleep_time = 60;  // The amount of minutes you would like to be powered down is defined here.
  XBee_Wake_Function();  // Calls the XBee wake function to turn the XBee on.
  Sensors_Wake_Function();  // Calls the sensors wake function below to wake the XBee up.
  SM_Analog_Value = analogRead(SM_Pin);  // Collects the analog reading value of 0-1023 for the sensor.
  Sensors_Sleep_Function();  // Calls the sensors sleep function from below to make the XBee go to sleep.
  SM_Difference = SM_Analog_Value - SM_Air_Value;  // Computes the difference between the SoilMoistureValue and the fresh air calibrated value.
  SM_Difference_Abs = abs(SM_Difference);  // Callculates the absolute value of the value computed above.
  SM_Decimal = SM_Difference_Abs / SM_Calibrated_Difference;  // Takes the SoilDifference and makes it a decimal from 0-1.
  SM_Percent = SM_Decimal * 100;  // Turns the Decimal value above and makes it a percentage.
  sprintf(Value_Text, "id,%04d,sm,%d", Arduino_Number, SM_Percent);  // Collects all the data and makes it a single string for sending.
  Serial.println(Value_Text);  // Prints the data from the sprintf data string all at once.
  XBee_Sleep_Function();  // Calls the XBee sleep function from below to make the XBee go to sleep.
  Sleep_Function(sleep_time);  // Calling the sleep function to start the power down process.
}

void Sleep_Function(unsigned long sleep_time) {  // Sleep function defined with the sleep time inputed above in minutes.
  sleep_time = 60 * sleep_time;  // Turning the sleep time from minutes to seconds.
  unsigned long counter = 0;  // Starting a counter for the watchdog timer to use to determine which watchdog function to call on.
  wdt_enable_8s();  // Enable the 8 second watchdog timer.
  unsigned long prevtime = millis();  // Determine the amount of time passed since the code has stopped running.
  for (int i = 0; i < round((float)sleep_time / 8); i++) {  // Find the number of times to run the watchdog function by dividing the sleep time in seconds by the 8 seconds that the watchdog timer can run for.
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Start up the sleep mode.
    sleep_enable();  // Enable the power down mode to shut the Arduino down,
    sleep_cpu();  // Run the sleep mode.
    counter += 8;  // Increase the counter by 8 seconds.
    if (sleep_time - counter == 4) {  // "if" statement to determine if the remaining time is not 8 seconds and instead is 4 seconds.
      wdt_disable();  // Disables the 8 second watchdog timer.
      wdt_enable_4s();  // Enables the 4 second watchdog timer for the remaing 4 seconds.
    }
  }
  wdt_disable();  // Disables the 4 second watchdog timer.
  sleep_disable();  // Disable the sleep mode.
}

void wdt_enable_8s() {  // The 8 second watchdog timer function.
  cli();  // Disables the int variables.
  MCUSR = 0;  // Clears the reset status of the watchdog timer.
  WDTCSR |= 0b00011000;  // Configures the WDCE and WDE bites to a 1 (on) instead of a 0 (off).
  WDTCSR = 0b01000000 | 0b100001;  // Sets the WDIE (enables the interrupt mode), clears WDE (disables the watchdog reset mode), and sets the timer interval to 8 seconds.
  sei();  // Enables the int variables.
}

void wdt_enable_4s() {  // The 4 second watchdog timer function.
  cli();  // Disables the int variables.
  MCUSR = 0;  // Clears the reset status of the watchdog timer.
  WDTCSR |= 0b00011000;  // Configures the WDCE and WDE bites to a 1 (on) instead of a 0 (off).
  WDTCSR = 0b01000000 | 0b100000;  // Sets the WDIE (enables the interrupt mode), clears WDE (disables the watchdog reset mode), and sets the timer interval to 4 seconds.
  sei();  // Enables the int variables.
}

void Sensors_Sleep_Function() {  // Function to make the sensors go to sleep after it sends the data.
  digitalWrite(SM_Power_Pin, LOW);  // Sets the soil moisture power pin to low to go to sleep.
  delay(50);  // Makes sure that sensors are ready.
}

void Sensors_Wake_Function() {  // Function for waking the sensors up after it has been sleeping.
  digitalWrite(SM_Power_Pin, HIGH);  // Sets the soil moisture power pin to high to wake up.
  delay(500);  // Delay to ensure the soil moisture is on and set back down to the correct value before reading the value.
}

void XBee_Sleep_Function() {  // Function for sleeping the XBee after the sensor data has been sent.
  delay(100);  // Delays to ensure that the data has sent via the XBee before sleeping the sensors.
  digitalWrite(XBee_Pin, HIGH);  // Sets the XBee output pin to high to make the XBee go to sleep.
}

void XBee_Wake_Function() {  // Function for waking up the XBee to send the data.
  digitalWrite(XBee_Pin, LOW);  // Sets the XBee output pin to low to wake the XBee up.
}
