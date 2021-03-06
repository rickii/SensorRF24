/*
 /*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/

 * *************************************************************************
 * This particular sketch does not work in it's current format.
 * The UNO and small micros are compiling to approx > 90% of capacity with the GPS modules included
 * It is posted here for later improvements 
 *
 *
 * RF24Ethernet Arduino library by TMRh20 - 2014-2015
 *
 * Automated (mesh) wireless networking and TCP/IP communication stack for RF24 radio modules
 *
 * RF24 -> RF24Network -> UIP(TCP/IP) -> RF24Ethernet
 *                     -> RF24Mesh
 *
 *      Documentation: http://tmrh20.github.io/RF24Ethernet/
 *
 * *************************************************************************
 *
 * What it does?:
 *
 * RF24Ethernet allows tiny Arduino-based sensors to automatically
 * form and maintain an interconnected, wireless mesh network capable of utilizing
 * standard (TCP/IP) protocols for communication. ( Nodes can also use
 * the underlying RF24Network/RF24Mesh layers for internal communication. )
 *
 * Any device with a browser can connect to and control various sensors, and/or the sensors
 * can communicate directly with any number of IP based systems.
 *
 * Why?
 *
 * Enabling TCP/IP directly on the sensors enables users to connect directly
 * to the sensor nodes with any standard browser, http capable tools, or with
 * virtually any related protocol. Nodes are able to handle low level communications
 * at the network layer and/or TCP/IP based connections.
 *
 * Remote networks can be easily interconnected using SSH tunnelling, VPNs etc., and
 * sensor nodes can be configured to communicate without the need for an intermediary or additional programming.
 *
 * Main Features:
 *
 * 1. Same basic feature set as any Arduino Ethernet adapter, only wireless...
 * 2. Uses RPi OR Arduino+Linux OR Arduino + any SLIP capable device as the wireless gateway/router.
 * 3. Easy Arduino configuration: Just assign a unique IP address to each node, ending in 2-255 (ie: 192.168.1.32)
 *    *Linux devices use standard TCP/IP networking (IPTABLES,NAT,etc) and tools (wget, ftp, curl, python...)
 * 4. Automated (mesh) networking creates and maintains network connectivity as nodes join the network or move around
 * 5. Automated, multi-hop routing allows users to greatly extend the range of RF24 devices
 * 6. API based on the official Arduino Ethernet library. ( https://www.arduino.cc/en/Reference/Ethernet )
 * 7. RF24Gateway (companion program for RPi) provides a user interface that automatically handles TCP/IP
 *    data, and is easily modified to handle custom RF24Network/RF24Mesh data.
 * 8. Reduce/Remove the need for custom applications. Any device with a browser can connect directly to the sensors!
 * 9. Handle (relatively) large volumes of data and file transfers automatically.
 *
 * *************************************************************************
 * Example Network:
 *
 * In the following example, 8 Arduino devices have assembled themselves into a
 * wireless mesh network, with 3 sensors attached directly to RPi/Linux. Five
 * additional sensors are too far away to connect directly to the RPi/Gateway,
 * so they attach automatically to the closest sensor, which will automatically
 * relay all communications for the distant node.
 *
 * Example network:
 *
 * Arduino 4 <-> Arduino 1 <-> Raspberry Pi    <-> Webserver
 * Arduino 5 <->              OR Arduino+Linux <-> Database
 * Arduino 6 <->                               <-> PHP
 *                                             <-> BASH (Wget, Curl, etc)
 * Arduino 7 <-> Arduino 2 <->                 <-> Web-Browser
 * Arduino 8 <->                               <-> Python
 *               Arduino 3 <->                 <-> NodeJS
 *                                             <-> SSH Tunnel <-> Remote RF24Ethernet Sensor Network
 *                                             <-> VPN        <->
 *
 * In addition to communicating with external systems, the nodes are able to
 * communicate internally using TCP/IP, and/or at the RF24Mesh/RF24Network
 * layers.
 *
 * **************************************************************************
 *
 * RF24Ethernet simple web client example
 *
 * RF24Ethernet uses the fine uIP stack by Adam Dunkels <adam@sics.se>
 *
 * In order to minimize memory use and program space:
 * 1. Open the RF24Network library folder
 * 2. Edit the RF24Networl_config.h file
 * 3. Un-comment #define DISABLE_USER_PAYLOADS
 *
 * This example connects to google and downloads the index page
 */



#include <RF24.h>
#include <SPI.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
//#include <printf.h>
#include <RF24Ethernet.h>
#if !defined __arm__ && !defined __ARDUINO_X86__
#include <EEPROM.h>
#endif

/*** Configure the radio CE & CS pins ***/
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24EthernetClass RF24Ethernet(radio, network, mesh);

//*********** GPS *********//
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);


//**************************


EthernetClient client;

void setup() {

  Serial.begin(115200);
  // printf_begin();
  Serial.println("Start");

  // Set the IP address we'll be using. The last octet mast match the nodeID (9)
  IPAddress myIP(10, 10, 2, 4);
  Ethernet.begin(myIP);
  mesh.begin();

  // If you'll be making outgoing connections from the Arduino to the rest of
  // the world, you'll need a gateway set up.
  IPAddress gwIP(10,10,2,2);
  Ethernet.set_gateway(gwIP);

  //************ GPS setup ********************
 // Serial.begin(115200);
  ss.begin(GPSBaud);

  Serial.println(F("DeviceExample.ino"));
//  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  //Serial.print(F("Testing TinyGPS++ library v. ")); 
 //// Serial.println(TinyGPSPlus::libraryVersion());
 // Serial.println(F("by Mikal Hart"));
  // Serial.println();

}

uint32_t counter = 0;
uint32_t reqTimer = 0;

uint32_t mesh_timer = 0;

// HTTP Server Settings
IPAddress serverUri(10,10,2,2);
const int updateInterval = 600 * 1000;      // Time interval in milliseconds to do a sensor read and send an update (number of seconds * 1000 = interval)

// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;

// Setup up sensor variables
int temperatureSensorPin = A0;    // select the input pin for the potentiometer
int temperatureSensorValue = 0;  // variable to store the value coming from the sensor
int tempWholePart = 0;
int tempFractPart = 0;

String nodeId = (String)4; // This will be sent with the POST data to help identify this node

String latitude =(String)-35.88;
String longitude =(String)149.60;

void loop() {
  //***********************************************

// This sketch displays information every time a new sentence is correctly encoded.
/*
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();
*/

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

if (ss.available() > 0){
  if (gps.encode(ss.read()))
 //     displayInfo();

      
      latitude = (String)gps.location.lat();
      longitude = (String)gps.location.lng();
   //  Serial.println( "Lat1   " + latitude);
   //   Serial.println("long1  " + longitude);
      
  
}

  //***********************************************

  // Optional: If the node needs to move around physically, or using failover nodes etc.,
  // enable address renewal
  if (millis() - mesh_timer > 30000) { //Every 30 seconds, test mesh connectivity
    mesh_timer = millis();
    if ( ! mesh.checkConnection() ) {
      //refresh the network address
      mesh.renewAddress();
    }
  }

  size_t size;

  // Get incoming data
  if (size = client.available() > 0) {
    char c = client.read();
    Serial.print(c);
  }
  

  // Disconnect from the server
  if (!client.connected() && lastConnected)
  {
    Serial.println("...disconnected");
    Serial.println();

    client.stop();
  }

  // See if its time to send an update
  if (!client.connected() && (millis() - lastConnectionTime > updateInterval)) {
    Serial.println("Time to send an update");
    // Get 10 readings of the temperature sensor and average them
    for (byte i = 0; i < 10; i++)
    {
      temperatureSensorValue += analogRead(temperatureSensorPin);
    }
    temperatureSensorValue = temperatureSensorValue / 10;

    // Adjusted whole value
    temperatureSensorValue = 25 *  temperatureSensorValue - 2050;
    tempWholePart = temperatureSensorValue / 100;
    tempFractPart = temperatureSensorValue % 100;

    String tempString = (String)tempWholePart + ".";
    if (tempFractPart  < 10)
    {
      tempString = tempString + "0";
    }

    tempString = tempString + (String)tempFractPart;

    
    Serial.println("Will send   " + tempString + " " + nodeId + " lat " + latitude + "  long2  " + longitude);
    // Call the send method
    sendSensorData(tempString, nodeId , latitude , longitude);

  }
  else {
   // Serial.print("Not ready for an update yet");
  }
  // Reset all our readings
temperatureSensorValue = 0; 
tempWholePart = 0;
tempFractPart = 0;


  lastConnected = client.connected();

  // We can do other things in the loop, but be aware that the loop will
  // briefly pause while IP data is being processed.
 // delay(1000);
}

void sendSensorData(String tempData, String nodeId, String lati, String longi)
{
  // concatenate all the data into a single string of key value pairs
  String formData = "&temperature=" + tempData + "&nodeid=" + nodeId  + "&lat=" + lati + "&long=" + longi ;
  Serial.println("formD   " + formData);
 // Serial.println( gps.location.lat());
  //   Serial.println(gps.location.lng());

  if (client.connect(serverUri, 3000))
  {
    client.print("POST /api/sensor HTTP/1.1\n");
    client.print("Host: 10.10.2.2\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(formData.length());
    client.print("\n\n");

    client.print("formD " + formData);
    client.print("\n\n");

    lastConnectionTime = millis();

    if (client.connected())
    {
      Serial.println("POSTing data...");
      Serial.println();

      failedCounter = 0;
    }
    else
    {
      failedCounter++;

      Serial.println("Connection to server failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }

  }
  else
  {
    failedCounter++;

    Serial.println("Connection to server Failed spciall(" + String(failedCounter, DEC) + ")");
    Serial.println();

    lastConnectionTime = millis();
  }
}
void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

