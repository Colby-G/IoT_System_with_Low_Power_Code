# Initial Installs, Downloads, and Setups Needed:
- Download and install "Arduino IDE" on comptuer
- Download and install "XCTU" on computer
- Download and install "VNC Viewer" on computer
- Download and install "VNC Server" on Raspberry Pi
- Setup ThingsBoard on Raspberry Pi
- Setup XBee devices for all Arduinos and the Raspberry Pi
- Setup all Arduinos with sensors and XBees
- Setup the Raspberry Pi with any sensors and your account
- Setup your VNC account and login to it on both the computer and the Raspberry Pi

# What Each File/Folder Does in the Repository:
- Arduino_Mini1_V1.ino file: Includes the code for an Arduino containing all of the general sensors for the environment (humidity, temperature, CO2, light, etc.)
- Arduino_Mini2_V1.ino file: Includes the code for an Arduino containing a specific sensor, in this case a soil moisture sensor
- Running_Time_Script.sh file: This is a shell script that will run on start up (you must configure this in the crontab file on the Raspberry Pi) and give the whole Raspberry Pi to boot up before calling on the Python script to run and collect data
- ThingsBoard_Read_Send.py file: A Python script that essentually filters the data coming in from the Arduinos, it allows ThingsBoard to know which Arduino sent what data and where to display it at

# How to Run:
- Download the code
- Put the code on the Arduinos and the Raspberry Pi as needed
- Give power to all the devices
- If everything is setup properly the data will come in every hour and display onto the ThingsBoard server for remote viewing