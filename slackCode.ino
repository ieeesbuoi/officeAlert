/*
 * This project is develped by the Student Branch of IEEE in University of Ioannina
 * and demonestrates a simple solution ofr a smart office system using ESP8266
 * arduino board with embedded WiFi chip. The ESP8266 is communicating via Slack API
 * with a selected channel.
 *
 * For any queries regarding the project or bugs send an email at ieeesbuoi@gmail.com.
 */

// Include all the necessary libraries here
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <dht.h>

// Place all the variable definitions here.
#define DHT11_PIN 7		// D5 pin of ESP8266
#define trigPin 4; 		// D2 pin of ESP8266
#define echoPin 2; 		// D4 pin of ESP8266
#define motionPin 5; 		// D1 of ESP8266

// Place all the global variables here.
dht DHT;			// For the HDT11 sensor
int maximumRange = 200; 	// Maximum range needed
int minimumRange = 0; 		// Minimum range needed
long duration, distance; 	// Duration used to calculate distance
int temperature = 0;		
int humidity = 0;

// This url is used after slack domain name.
String url = "/services/T2NUR9UG4/B81FT2HPG/j12D!d122f392fnn2f31f0i190ASD";

// Wifi credentials settings.
const char* ssid = "place_wifi_ssid_here";
const char* password = "place_password_here";

// Do not change those parameters
const char* host = "hooks.slack.com";
const int httpsPort = 443;

// SHA1 fingerprint of the SLACK certificate
const char* fingerprint = "AC 95 5A 58 B8 4E 0B CD B3 97 D2 88 68 F5 CA C1 0A 81 E3 6E";

// PIR sensor
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

int prevStatus = 0; // closed
int curStatus = 0;

/*****************************************************************************
 * Place your functions below. Before each function use comments to describe
 * what it does.
*****************************************************************************/

/*
 *
 */
void checkDistance() {
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 distance = duration/58.2;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(motionPin, INPUT);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void requestFunc(String myStr) {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  Serial.print("requesting URL: ");
  Serial.println(url);
  String msgtoSend = "{\"text\": \""+myStr+"\"}";
  
  String sendReq = String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + msgtoSend.length() + "\r\n"
               "\r\n" + msgtoSend +
               "\r\n";
               ;
  client.print(sendReq);
  Serial.println(sendReq);
  Serial.println("request sent");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println(line);
  if (line.startsWith("{\"state\":\"success\"")) {
    //Serial.println("esp8266/Arduino CI successfull!");
  } else {
    //Serial.println("esp8266/Arduino CI has failed");
  }
  //Serial.println("reply was:");
  //Serial.println("==========");
  //Serial.println(line);
  //Serial.println("==========");
  //Serial.println("closing connection");
}

void getMotionStatus() {
  val = digitalRead(motionPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}

/*
 * comment your function here
 */
void updateTempHum() {

}

// Loop forever and ever
void loop() {
  checkDistance();
  Serial.print("The distance is:");
  Serial.println(distance);
  if( (distance>=0) && (distance < 13) ) {
    // close
    curStatus=0;
  }else {
    // open
    curStatus=1;
  }
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
  delay(3000);
  getMotionStatus();

  if( (curStatus==0) && (prevStatus==1) && (motionPin == 1) ) {
    delay(3000);
    requestFunc("The door is closed.");

    prevStatus=curStatus;
  }else if( (curStatus==1) && (prevStatus==0) ) {
    delay(3000);
    getMotionStatus();
    requestFunc("The door is open.");
    
    prevStatus=curStatus;
  }
  delay(1000);
}
