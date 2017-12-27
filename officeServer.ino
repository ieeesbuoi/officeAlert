/*
 * This project is develped by the Student Branch of IEEE at University of Ioannina
 * and demonestrates a simple solution for a smart office system using ESP8266 board
 * with embedded WiFi chip. The ESP8266 is communicating via Slack API with a specific
 * channel.
 *
 * For any queries regarding the project or bugs send an email at ieeesbuoi@gmail.com.
 */

// Include any library here.
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <DHT_U.h>

/*****************************************************************************
 * Pinout definition
*****************************************************************************/
#define DHT11_PIN 5      // Connected to GPIO pin 1 of ESP8266 (temp and hum)
#define trigPin 14       // -
#define echoPin 15       // -
#define motionPin 4      // Connected to GPIO pin 2 of ESP8266
#define magnetPin 2      // Connected to GPIO pin 4 of ESP8266

#define DHTTYPE DHT11    // Set the DHT type to DHT11

/*****************************************************************************
 * RULES and THRESHOLDS
*****************************************************************************/
#define TEMP_THRESHOLD 23
#define HUM_THRESHOLD 50

/*****************************************************************************
 * Credentials for the SLACK API. DO NOT CHANGE. KEEP THEM SECRET!
*****************************************************************************/

// Domainname of the slack API request
const char* host = "****";

// Second part of the slack url request
//String url = "/services/T2NUR9UG4/B81FT2HPG/4dDvXyjNA10F0PfaJcTrU4HF";
String url = "****";

// HTTPS port number for the slack API request.
const int httpsPort = "****";

// SHA1 fingerprint of the SLACK certificate
const char* fingerprint = "****";

// Credentials for the WiFi network or hotspot.
const char* ssid = "****";
const char* password = "****";

/*****************************************************************************
 * GLOBAL VARIABLES and CONSTANTS
*****************************************************************************/
DHT dht(DHT11_PIN, DHTTYPE);
int maximumRange = 13;   // Maximum range needed
int minimumRange = 0;     // Minimum range needed

float temperature = 0.0;    
float humidity = 0.0;
int MOTION_STATUS = 0;

// PIR sensor
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

int prevStatus = 0; // closed
int curStatus = 0;
int DOOR = 0;


/*****************************************************************************
 * Place your functions below. Before each function use comments to describe
 * what it does.
*****************************************************************************/

/*
 * This function updates the distance variable with the current dirstance
 * that the door sensor has.
 */
void checkDistance() {
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 long duration = pulseIn(echoPin, HIGH);
 
 // Calculate the distance (in cm) based on the speed of sound.
 long distance = duration/58.2;

  // If the distance is between the min and max range as defined
  // above then update the status of the door.
 if( (distance >= minimumRange) && (distance < maximumRange) ) {
    // close
    curStatus=0;
  }else {
    // open
    curStatus=1;
  }
}

void checkMotion() {
  val = digitalRead(motionPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      
      MOTION_STATUS = 1;
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("Motion ended!");
    
      MOTION_STATUS = 0;
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}

/*
 * It uses DHT11 sensor to gather all the information about the temperature
 * and humidity sensor. Using the dht object it returns the temp and humidty
 * in two different variables.
 */
void checkDHTInfo() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();  
}

/*
 * Read a REED sensor and return the value. Then update a global variable about the 
 * status of the DOOR.
 */
void checkMagnet() {
  int magnetValue = digitalRead(magnetPin);
  if(magnetValue == 1) {
    DOOR = 1;  
  }else if(magnetValue ==0) {
    DOOR = 0;
  }else {
    DOOR = -1;
  }
}

/*
 * Create a request using the slack API to a specific channel. The function input
 * is a simple string (the message to be send)
 */
void requestFunc(String myStr) {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("Connecting to ");
  Serial.println(host);

  // Verify that the connection was established successfully.
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  // Verify the ESP8266 using the pre-defined fingerprint.
  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  // Prepare the message as a json file.
  String msgtoSend = "{\"text\": \""+myStr+"\"}";

  // Prepare the reqeust headers.
  String sendReq = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + msgtoSend.length() + "\r\n"
               "\r\n" + msgtoSend +
               "\r\n";

  // Send the request to the slack server.
  client.print(sendReq);

  // Read the header response from the slack server (for debuggin purposes)
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r") {
      //Serial.println("headers received");
      break;
    }
  }

  // Read the data response from the slack server (for debuggin purposes)
  String line = client.readStringUntil('\n');
  //Serial.println(line);
  if (line.startsWith("{\"state\":\"success\"")) {
    //Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
}

/*
 * This function was through all of the defined rules and decides if the
 * ESP8266 will send any messages to slack.
 */
void checkRules() {
  // Update Values
  checkDHTInfo();
  checkMotion();
  checkMagnet();
  
  // Temperature is over the threshold
  if(temperature > TEMP_THRESHOLD) {
    String tempAlertMsg = "Temperature is over the threshold!\nTemperature: ";
    
    requestFunc(tempAlertMsg+temperature);
  }

  // Humidity is over the threshold
  if(humidity > HUM_THRESHOLD) {
    String humAlertMsg = "Humidity is over the threshold!\nHumidity: ";
    requestFunc(humAlertMsg+humidity);
  }

  // Humidity is over the threshold
  // (ADD code that the door was left open with millis() counter )
  if(DOOR == 1) {
    String doorAlertMsg = "Room door was opened!";
    requestFunc(doorAlertMsg);
  }
}

void printStatus() {
  // Get values from all sensors connected.
  checkDHTInfo();
  checkMotion();
  checkMagnet();

  // Prepare all the strings and variables (initial slack report stage)
  String msg1 = "Smart office system is connected to the network...\nPreparing sensor analysis...\n";  
  String temp_msg = "Temperature(celcius): ";
  String hum_msg = "\nHumidity(%): ";
  String mot_msg = "\nMotion(0 for NO): ";
  String door_msg = "\nDoor status(0 for CLOSE): ";
  String end_msg = "\nUp and ready...";
  String totalMsg = msg1+temp_msg+(int)temperature+hum_msg+(int)humidity+mot_msg+MOTION_STATUS+door_msg+DOOR+end_msg;
  requestFunc(totalMsg);
  delay(2000);
}

/*
 * Setup function will be executed everytime the ESP8266 board connects 
 * to a power or reboots it self.
 */
void setup() {
  Serial.begin(115200);
  Serial.println();
  dht.begin();

  // Specify INPUT/OUTPUT mode for each pin you use.
  pinMode(motionPin, INPUT);
  pinMode(magnetPin, INPUT);

  // Connect to the WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connection was successfull!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Wait 5 sec for any hardware changes that you need to make (usually connect jumper wires).
  // All the TX/RX pins in the ESP8266 must NOT be connected before this step.
  Serial.println("I am gonna sleep now for 5 sec...");
  delay(5000);
  
  // Get initial values from all sensors connected.
  checkDHTInfo();
  checkMotion();
  checkMagnet();

  // Prepare all the strings and variables (initial slack report stage)
  String msg1 = "Smart office system is connected to the network...\nPreparing sensor analysis...\n";  
  String temp_msg = "Temperature(celcius): ";
  String hum_msg = "\nHumidity(%): ";
  String mot_msg = "\nMotion(0 for NO): ";
  String door_msg = "\nDoor status(0 for CLOSE): ";
  String end_msg = "\nUp and ready...";
  String totalMsg = msg1+temp_msg+(int)temperature+hum_msg+(int)humidity+mot_msg+MOTION_STATUS+door_msg+DOOR+end_msg;
  requestFunc(totalMsg);
  delay(2000);
}

/*
 * Loop function is like a while loop that is always true.
 */
void loop() {
  // This function was through all of the defined rules and decides if the
  // ESP8266 will send any messages to slack.
  checkRules();

  // Debuggin Purposes
  //printStatus();
  
  // Delay 1/2 sec before the next sensors read
  delay(500);
}

