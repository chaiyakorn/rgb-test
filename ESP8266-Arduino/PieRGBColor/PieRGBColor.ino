#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <debug.h>
#include <string.h>

#define APPID   "RGBtest"
#define KEY     "H7sEM7JwmIA2WrG"
#define SECRET  "aq7PbBFxYvXNvAnT47UBn00S9"
#define ALIAS   "striprgb"

#define PIN            4
#define NUMPIXELS      12
#define MAX_STRING_LEN 3

WiFiClient client;
AuthClient *authclient;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 1; // delay for half a second
char *p, *i;
int x;
char *rgb[3];
int r=0,g=0,b=0;

MicroGear microgear(client);

char* subStr (char* input_string, char *separator, int segment_number) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;

  strcpy(copy, input_string);

  for (i = 1, act = copy; i <= segment_number; i++, act = NULL) {
    sub = strtok_r(act, separator, &ptr);
    if (sub == NULL) break;
  }
  return sub;
}

void setColor(int r, int g , int b){
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(r,g,b));
    pixels.show();
    delay(delayval); // Delay for a period of time (in milliseconds).
  }  
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char *str = (char*)msg;
  str[msglen] = '\0';
    
  for (int x = 1; x <= 3; x++) {
    rgb[x-1]=(subStr(str, ",", x));
  }
  
  r = atoi((char*)rgb[0]);
  g = atoi((char*)rgb[1]);
  b = atoi((char*)rgb[2]);
  Serial.print(r);
  Serial.print(" : ");
  Serial.print(g);
  Serial.print(" : ");
  Serial.println(b);
  if((r>=0&&r<=256) && (g>=0&&g<=256) && (b>=0&&b<=256)) setColor(r,g,b);
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.subscribe("/striprgb/color");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show();
  
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);

  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect("PieStripRGB")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(CONNECTED, onConnected);
  
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);
}

void loop() {

  if (microgear.connected()) {
    microgear.loop();
  }
  else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
}
