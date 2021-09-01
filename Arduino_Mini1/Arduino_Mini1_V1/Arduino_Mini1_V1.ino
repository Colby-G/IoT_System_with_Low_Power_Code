#include <avr/sleep.h>  // Including the avr/sleep.h library for the sleep function.
#include "avr/wdt.h"  // Including the avr/wdt.h library for watchdog timer.
#include "DHT.h"  // Including the DHT.h library for the DHT11 sensor.

#define DHT11_Pin 10  // Defines the pin used for the DHT sensor.
#define DHT_Type DHT11  // Defines the type of DHT sensor you are using. Here we have the DHT11.
#define CO2_Pin A0  // Defines which analog input pin you are using.
#define CO2_Zero_Point_Voltage_Reading 1.57  // Defines the output of the sensor in volts when the concentration of CO2 is 400ppm (calibrated voltage value).
#define CO2_Reaction_Voltage 0.030  // Defines the voltage drop of the sensor when moved from air into 1000ppm CO2.
#define Light_Pin A1  // Defines which analog input pin you are using.
#define Transistor_Pin 6 // Defines which pin is used to go to the transistor.
#define XBee_Pin 3  // Defines which digital pin the XBee is connected to for sleeping.
#define DHT11_Power_Pin 9  // Defines which pin is used to turn the power on and off for the DHT11 sensor.
#define Light_Power_Pin 5  // Defines which pin is used to turn the power on and off for the light sensor.

int Arduino_Number = 1;  // Defines the Ardunio number.
float CO2_Zero_Point_Voltage = CO2_Zero_Point_Voltage_Reading / 8.5;  // Takes the output voltage value and divides it by the DC gain of the amlifier (8.5).
float CO2_Curve[3] = {2.602059991, CO2_Zero_Point_Voltage, (CO2_Reaction_Voltage / (2.602059991 - 3))};  // Two points are taken from the curve. With these two points, a line is formed which is "approximately equivalent" to the original curve. Data Format: (x, y, slope); point1: (log400, 0.324), point2: (log4000, 0.280), slope: (Reaction_Voltage)/(log(400)–log(1000))
float CO2_Voltage;  // Defines the CO2 collection variable for the voltage read.
float Light_Difference_Abs;  // Defines the variable for the absolute difference between the light values.
float Light_Decimal;  // Defines the evariable for the light sensor in decimal form.
int CO2_Percent;  // Defines the CO2 variable for coverting to a percent value.
int C_DHT11_Temperature = 0;  // Defines the temperature collection variable initially as 0.
int DHT11_Humidity = 0;  // Defines the humidity collection variable initially as 0.
int Light_Percent;  // Defines the variable for the light sensor in percent form.
int CO2_Analog_Value;  // Defines the CO2 variable for collecting the analog value.
int Light_Analog_Value;  // Defines the light sensor variable for the collection of the analog value.
int Light_Difference;  // Defines the light sensor variable for the difference between what was read and the max reading (allows for inverse reading of the light since the sensor actually reads how dark it is and we want to know how light it is).
char Value_Text[50];  // Defines the character variable used for the sprintf.
unsigned long Minutes_for_Sensors = 5;  // Defines the minutes taken for the CO2 sensor to heat up.
unsigned long Seconds_for_Sensors = Minutes_for_Sensors * 60;  // Convertes the minutes to seconds for the CO2 sensor.
unsigned long Previous_Time;  // Defines the previous time variable.
DHT dht(DHT11_Pin, DHT_Type);  // Initializes the DHT11 sensor with the parameters needed.

ISR(WDT_vect) {  // An ISR function for watchdogs.
  wdt_reset();  // Resets the watchdog timer when called upon.
}

void setup() {  // Setup code that only runs once.
  Serial.begin(9600);  // Starts running the serial monitor.
  pinMode(Transistor_Pin, OUTPUT);  // Sets transistor pin to be an output.
  pinMode(XBee_Pin, OUTPUT);  // Sets the XBee pin as an output for signaling the sleep mode.
  pinMode(DHT11_Power_Pin, OUTPUT);  // Sets the DHT11 power pin to output for signaling sleep.
  pinMode(Light_Power_Pin, OUTPUT);  // Sets the light sensor power pin to output for signaling to sleep.
  wdt_disable();  // Disables the watchdog timer at the beginning to ensure that it is not enabled to mess with the rest of the code.
  dht.begin();  // Starts running the DHT11 sensor to read the values.
}

void loop() {  // Loop code that runs on a loop continuously.
  int sleep_time = 60 - Minutes_for_Sensors;  // The amount of minutes you would like to be powered down is defined here.
  Transistor_Wake_Function();  // Calls the transistor wake function to let the CO2 sensor heat up.
  CO2_Analog_Value = analogRead(CO2_Pin);  // Collects the analog reading value of 0-1023 for the sensor.
  Transistor_Sleep_Function();  // Calls the transistor sleep function to turn the CO2 sensor off.
  XBee_Wake_Function();  // Calls the XBee wake function to turn the XBee on.
  Sensors_Wake_Function();  // Calls the sensor wake function below to wake the sensors up.
  Previous_Time = millis();  // Gets the current time past since the program has started running as a previous variable.
  do {  // "do" loop to have the Arduino continuosly check the DHT11 values until the "while" condidtion it met.
    C_DHT11_Temperature = dht.readTemperature();  // Reads the Celsius temperature value from the DHT11 sensor.
    DHT11_Humidity = dht.readHumidity();  // Reads the humidity value from the DHT11 sensor.
  }while ((millis() - Previous_Time < 30000) && ((C_DHT11_Temperature == 0) || (DHT11_Humidity == 0)));  // "while" condition to make sure a non-zero variable is recived from the DHT11 sensor until continuing or it times out and needs to continue to not break the program.
  Light_Analog_Value = analogRead(Light_Pin);  // Collects the analog reading value of 0-1023 for the sensor.
  Sensors_Sleep_Function();  // Calls the sensor sleep function from below to make the sensors go to sleep.
  CO2_Voltage = CO2_Analog_Value * (5.0 / 1023.0);  // Converts the analog vaue to a voltage value of 0-5V.
  CO2_Percent = MGGetPercentage(CO2_Voltage, CO2_Curve);  // Sets up MGGetPercentage as a function. Also, calls the MGGetPercentage section below to run.
  Light_Difference = Light_Analog_Value - 1023;  // Takes the difference and inverse of the analog reading.
  Light_Difference_Abs = abs(Light_Difference);  // Takes the absolute value of the number calculated above.
  Light_Decimal = Light_Difference_Abs / 1023;  // Gets the decimal for the light amount.
  Light_Percent = Light_Decimal * 100;  // Gets the percent for the light amount.
  sprintf(Value_Text, "id,%04d,t,%d,rh,%d,co,%d,li,%d", Arduino_Number, C_DHT11_Temperature, DHT11_Humidity, CO2_Percent, Light_Percent);  // Collects all the data and makes it a single string for sending.
  Serial.println(Value_Text);  // Prints the data from the sprintf data string all at once.
  XBee_Sleep_Function();  // Calls the XBee sleep function to turn the XBee off.
  Sleep_Function(sleep_time);  // Calling the sleep function to start the power down process.
}

int MGGetPercentage(float CO2_Voltage, float * pcurve) {  // Starts the MGGetPercentage section and initiates the varriables.
  return pow(10, ((CO2_Voltage / 8.5) - pcurve[1]) / pcurve[2] + pcurve[0]);  // Gives a return value for the actual value of the CO2 value based on the curve from above.
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
  digitalWrite(DHT11_Power_Pin, LOW);  // Sets the DHT11 power pin to low to go to sleep.
  digitalWrite(Light_Power_Pin, LOW);  // Sets the light sensor power pin to low to go to sleep.
  delay(50);  // Makes sure that XBee is ready.
}

void Sensors_Wake_Function() {  // Function for waking the sensors up after it has been sleeping.
  digitalWrite(DHT11_Power_Pin, HIGH);  // Sets the DHT11 power pin to low to go to sleep.
  digitalWrite(Light_Power_Pin, HIGH);  // Sets the light sensor power pin to low to go to sleep.
  C_DHT11_Temperature = 0;  // Resets the DHT11 temperature to 0.
  DHT11_Humidity = 0;  // Resets the DHT11 humidity to 0.
}

void Transistor_Sleep_Function() {  // Function for sleeping the transistor after heating up and collecting the data.
  digitalWrite(Transistor_Pin, LOW);  // Turn the transistor power to off.
}

void Transistor_Wake_Function() {  // Function for waking the transistor up to allow the CO2 sensor to heat up and collect data.
  digitalWrite(Transistor_Pin, HIGH);  // Turn the transistor power to on so that the CO2 sensor can warm up and collect data.
  delay(Seconds_for_Sensors * 1000);  // Delay specified minutes from above to allow the CO2 sensor to heat up and others to get ready.
}

void XBee_Sleep_Function() {  // Function for sleeping the XBee after the sensor data has been sent.
  delay(100);  // Delays to ensure that the data has sent via the XBee before sleeping the XBee.
  digitalWrite(XBee_Pin, HIGH);  // Sets the XBee output pin to high to make the XBee go to sleep.
}

void XBee_Wake_Function() {  // Function for waking up the XBee to send the data.
  digitalWrite(XBee_Pin, LOW);  // Sets the XBee output pin to low to wake the XBee up.
}
