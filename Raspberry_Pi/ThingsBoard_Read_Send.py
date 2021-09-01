#! /usr/bin/python3
# Import time for gathering the time values needed.
import time
# Import serial for the serial port communication.
import serial
# Import threading to be able to thread and run multiple functions at once.
import threading
# Import json to call the ThingsBoard json file where to dump data.
import json
# Import os to call the ThingsBoard shell script that sends the json file data to the ThingsBoard server.
import os

# Gather the initial start time of the script.
Start_Time = time.time()
# Insert the hours between Arduino battery checking here.
Error_Check_Hours = 5
# Calculates the seconds for the hours defined above.
Error_Check_Seconds = Error_Check_Hours * 3600
# Initialize variables to initial values.
Delay_Time = 0
counter1 = 0
counter2 = 0
counter3 = 0
counter4 = 0
counter5 = 0
connection = 0
# Define the statements sent to ThingsBoard for the devices in the server.
Ras_Pi_Status_C = 'Connected'
Ras_Pi_Status_E = 'Error: Reconnect USB/Reboot Pi'
Arduino_Status_C = 'Connected'
Arduino_Status_E = 'Error: Check Arduino Battery'
Arduino_Status_NP = 'Error: Arduino Not Programmed'


# Serial function to connect to the proper serial port and call on the other functions as needed. There are two serial ports defined here because the Raspberry Pi likes to change the port if the USB becomes disconnected and reconnected.
def serial_connection():
    # Global variable that will be passed around between each function.
    global connection, Ras_Pi_Status_E
    # "while True" statement to allow the function to continue to run even if these serial ports are not available.
    while True:
        # "try" statement to allow the function to check the serial port /dev/ttyUSB0 and if available set the connection toggling variable to 0.
        try:
            serial_port = serial.Serial('/dev/ttyUSB0', 9600, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE)
            # "while True" statement to allow the program to continue to run as long as this serial port is open.
            while True:
                arduino_logic(serial_port)
        # "except" statement to catch any serial port errors.
        except:
            # "if" statement to send the error message to ThingsBoard one time and change the connection value.
            if connection == 0:
                connection = 1
                # Data sent to the ThingsBoard json file in json format.
                json_data = {
                    f'connection_status_ras_pi': Ras_Pi_Status_E,
                }
                # Dumping the json data from above into the json file and calling the shell script to run using this function.
                dump_json(json_data)
            # Using this function to reset the counter values to 0.
            reset_counters()
        # "try" statement to allow the function to check the serial port /dev/ttyUSB1 and if available set the connection toggling variable to 0.
        try:
            serial_port = serial.Serial('/dev/ttyUSB1', 9600, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE)
            # "while True" statement to allow the program to continue to run as long as this serial port is open.
            while True:
                arduino_logic(serial_port)
        # "except" statement to catch any serial port errors.
        except:
            # "if" statement to send the error message to ThingsBoard one time and change the connection value.
            if connection == 0:
                connection = 1
                # Data sent to the ThingsBoard json file in json format.
                json_data = {
                    f'connection_status_ras_pi': Ras_Pi_Status_E,
                }
                # Dumping the json data from above into the json file and calling the shell script to run using this function.
                dump_json(json_data)
            # Using this function to reset the counter values to 0.
            reset_counters()


# Battery checking function computing all the time differences and counting and checking the Arduino batteries.
def arduino_errors():
    # Global variable that will be passed around between each function.
    global counter1, counter2, counter3, counter4, counter5, connection, Start_Time, Delay_Time, Error_Check_Seconds, Arduino_Status_E
    # "while True" statement to allow the function to continue to run, but not send battery errors, if the serial port is not available.
    while True:
        # "if" statement to determine if the serial port is connected or not via the connection toggling variable and continually to recalculate the delay time until the next "if" statement is true.
        if connection == 0:
            Delay_Time = time.time() - Start_Time
            # "if" statement to determine if the delay time from above equals the declared hour interval in seconds.
            if round(Delay_Time) == Error_Check_Seconds:
                # "if" statement to determine if the Arduino has not sent data.
                if counter1 == 0:
                    # Data sent to the ThingsBoard json file in json format.
                    json_data = {
                        f'connection_status_1': Arduino_Status_E,
                    }
                    # Dumping the json data from above into the json file and calling the shell script to run using this function.
                    dump_json(json_data)
                # "if" statement to determine if the Arduino has not sent data.
                if counter2 == 0:
                    # Data sent to the ThingsBoard json file in json format.
                    json_data = {
                        f'connection_status_2': Arduino_Status_E,
                    }
                    # Dumping the json data from above into the json file and calling the shell script to run using this function.
                    dump_json(json_data)
                # "if" statement to determine if the Arduino has not sent data.
                if counter3 == 0:
                    # Data sent to the ThingsBoard json file in json format.
                    json_data = {
                        f'connection_status_3': Arduino_Status_E,
                    }
                    # Dumping the json data from above into the json file and calling the shell script to run using this function.
                    dump_json(json_data)
                # "if" statement to determine if the Arduino has not sent data.
                if counter4 == 0:
                    # Data sent to the ThingsBoard json file in json format.
                    json_data = {
                        f'connection_status_4': Arduino_Status_E,
                    }
                    # Dumping the json data from above into the json file and calling the shell script to run using this function.
                    dump_json(json_data)
                # "if" statement to determine if the Arduino has not sent data.
                if counter5 == 0:
                    # Data sent to the ThingsBoard json file in json format.
                    json_data = {
                        f'connection_status_5': Arduino_Status_E,
                    }
                    # Dumping the json data from above into the json file and calling the shell script to run using this function.
                    dump_json(json_data)
                # Using this function to reset the counter values to 0.
                reset_counters()
                # Reset the start time to the current time.
                Start_Time = time.time()
        # "else" statement for if the serial port is not available then just reset the start time and do nothing else.
        else:
            Start_Time = time.time()


# Function to communicate with the ThingsBoard server.
def dump_json(json_data):
    # "with" statement to open the json file in the Raspberry Pi directory and call it 'JSON_File'.
    with open('/home/pi/ThingsBoard/Data/telemetry-data.json', 'w') as JSON_File:
        # Dump the json data into the json file.
        json.dump(json_data, JSON_File)
    # Find the shell script in the Raspberry Pi directory and run it to send the json data to ThingsBoard.
    os.popen('sh /home/pi/ThingsBoard/Data/mqtt-js.sh').read()


# Function that resets the counter values to 0.
def reset_counters():
    global counter1, counter2, counter3, counter4, counter5
    counter1 = 0
    counter2 = 0
    counter3 = 0
    counter4 = 0
    counter5 = 0


# Arduino logic function that computes all the functions given by each Arduino.
def arduino_logic(serial_port):
    global counter1, counter2, counter3, counter4, counter5, connection, Ras_Pi_Status_C, Arduino_Status_C, Arduino_Status_NP
    # Read the line of data, decode it into normal language, split it by commas, and assign the Arduino ID to a variable.
    connection = 0
    Read = serial_port.readline()
    Line = Read.decode('utf-8')
    Data_Split = Line.split(',')
    Arduino_ID = Data_Split[1]
    # "if" statement to determine the Arduino ID and assign the data to variables.
    if Arduino_ID == '0001':
        Temperature1_Value = Data_Split[3]
        Humidity1_Value = Data_Split[5]
        CO21_Value = Data_Split[7]
        Light1_Value = Data_Split[9]
        # Data sent to the ThingsBoard json file in json format.
        json_data = {
            f'connection_status_ras_pi': Ras_Pi_Status_C,
            f'connection_status_1': Arduino_Status_C,
            f'temperature_1': Temperature1_Value,
            f'humidity_1': Humidity1_Value,
            f'co2_1': CO21_Value,
            f'light_intensity_1': Light1_Value,
        }
        # Dumping the json data from above into the json file and calling the shell script to run using this function. Increase the counter variable.
        dump_json(json_data)
        counter1 = counter1 + 1
    # "elif" statement to determine the Arduino ID and assign the data to variables.
    elif Arduino_ID == '0002':
        Soil_Moisture2_Value = Data_Split[3]
        # Data sent to the ThingsBoard json file in json format.
        json_data = {
            f'connection_status_ras_pi': Ras_Pi_Status_C,
            f'connection_status_2': Arduino_Status_C,
            f'soil_moisture_2': Soil_Moisture2_Value,
        }
        # Dumping the json data from above into the json file and calling the shell script to run using this function. Increase the counter variable.
        dump_json(json_data)
        counter2 = counter2 + 1
    # "elif" statement to determine the Arduino ID and assign the data to variables.
    elif Arduino_ID == '0003':
        Soil_Moisture3_Value = Data_Split[3]
        # Data sent to the ThingsBoard json file in json format.
        json_data = {
            f'connection_status_ras_pi': Ras_Pi_Status_C,
            f'connection_status_3': Arduino_Status_C,
            f'soil_moisture_3': Soil_Moisture3_Value,
        }
        # Dumping the json data from above into the json file and calling the shell script to run using this function. Increase the counter variable.
        dump_json(json_data)
        counter3 = counter3 + 1
    # "elif" statement to determine the Arduino ID and assign the data to variables.
    elif Arduino_ID == '0004':
        Soil_Moisture4_Value = Data_Split[3]
        # Data sent to the ThingsBoard json file in json format.
        json_data = {
            f'connection_status_ras_pi': Ras_Pi_Status_C,
            f'connection_status_4': Arduino_Status_C,
            f'soil_moisture_4': Soil_Moisture4_Value,
        }
        # Dumping the json data from above into the json file and calling the shell script to run using this function. Increase the counter variable.
        dump_json(json_data)
        counter4 = counter4 + 1
    # "elif" statement to determine the Arduino ID and assign the data to variables.
    elif Arduino_ID == '0005':
        Soil_Moisture5_Value = Data_Split[3]
        # Data sent to the ThingsBoard json file in json format.
        json_data = {
            f'connection_status_ras_pi': Ras_Pi_Status_C,
            f'connection_status_5': Arduino_Status_C,
            f'soil_moisture_5': Soil_Moisture5_Value,
        }
        # Dumping the json data from above into the json file and calling the shell script to run using this function. Increase the counter variable.
        dump_json(json_data)
        counter5 = counter5 + 1
    # "else" statement to catch all un-programmed Arduino's and write a statement in the terminal for user help.
    else:
        print(Arduino_Status_NP)


# Threading section to call the functions to be ran at the sametime continuously.
threading.Thread(target=serial_connection).start()
threading.Thread(target=arduino_errors).start()
