
#include <WiFi.h>
#include <WiFiUdp.h>

#define MAXUDPSIZE 1500
#define LED_PIN 25
#define MAXSERIALSIZE 10
// WiFi network name and password:
const char *networkName = "lenovo-wifi";
const char *networkPswd = "asdfghjkl";
const int ledPin = 25;

char inChar;
// Local static IP
IPAddress local_IP(192, 168, 137, 3);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char *udpAddress = "192.168.137.2"; 
const int udpPort = 3333;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;
uint8_t udpRX[MAXUDPSIZE];
// data received from serial
char serialRX[MAXSERIALSIZE];   // a String to hold incoming data
boolean stringComplete = false; // whether the string is complete
int serialRX_len = 0;

void toggleLed()
{
  digitalWrite(ledPin, !digitalRead(ledPin));
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  // Initilize hardware serial:
  Serial.setTimeout(100);
  Serial.begin(115200);

}

void loop()
{
  if (!connected)
  {
    
    connectToWiFi(networkName, networkPswd);
    while (!connected)
    {
      delay(1000);
    }
  }

  String message;

  if ((Serial.available()))
  {
    digitalWrite(ledPin, 1);

    message = Serial.readString();

    stringComplete = true;
  }
    digitalWrite(ledPin, 0);

    if ((stringComplete) && (connected) && (message.length() > 0))
  {
    toggleLed();

    message.toCharArray(serialRX, MAXSERIALSIZE);
    udp.beginPacket(udpAddress, udpPort);
    udp.write((uint8_t *)serialRX, message.length());
    udp.endPacket();
    delay(100);

    stringComplete = false;
  }


  memset(udpRX, 0, MAXUDPSIZE);
  udp.parsePacket();
  if (udp.read(udpRX, MAXUDPSIZE) > 0)
  {
    toggleLed();
    Serial.write(udpRX, MAXUDPSIZE);
    delay(50);
  }
}

void connectToWiFi(const char *ssid, const char *pwd)
{
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  // set static IP
  WiFi.config(local_IP, gateway, subnet);

  // set no power save mode to low latency
  WiFi.setSleep(false);

  //Initiate connection
  WiFi.begin(ssid, pwd);
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_CONNECTED:
    connected = true;
    break;

  case SYSTEM_EVENT_STA_GOT_IP:
    //initializes the UDP state
    //This initializes the transfer buffer
    udp.begin(WiFi.localIP(), udpPort);
    connected = true;
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    connected = false;
    break;
  }
}
