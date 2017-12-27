# officeAlert 

# Summary
OfficeAlert project is a combination of several lowcost arduino compatible sensors, that are used to gather information from an office and transmit all the data via slack API to a specific channel. The idea behind this project is the alert of a specific change regarding all our sensors. For example if a door opens, the temperature rises over some threshold and the window is also open then the system sends a notification to the specific slack API and alert everyone who has joined that channel about the change in the office.

# How to start
First of all, you need arduino IDE. Arduino IDE can be downloaded from https://www.arduino.cc/en/Main/Software. Arduino IDE has two basic parameters, port and arduino version. You can specify those parameters from the main menu. To choose the port you have to physically connect the arduino using a usb cable.

Secondly, you need to install all the necessary libraries. You can download them from our Libraries folder. You have to install both folders by simple copy and paste those folders to your Arduino/libraries local folder.

#System Αrchitecture
The system architecture that was installed at our office is the following.
![Alt Text](https://github.com/ieeesbuoi/officeAlert/systemarchitecture.png)
We have used UTP cable to extend any cables and connect the necessary power to the ESP8266.

#Contact
Feel free to contact anyone in our team who has contributed or send an email at ieeesbuoi@gmail.com for more information or bugs found.

Website: ieeesb.uoi.gr
Email: ieeesbuoi@gmail.com
