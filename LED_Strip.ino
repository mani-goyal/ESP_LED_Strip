//-----------------------------------------------
//                Tech Random DIY
//               LED_WALL_Source
//                 Chris Parker
//-----------------------------------------------
// WebSocket Globals
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Preferences.h>
#include "html.h"
#include <FastLED.h>


String configJson;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
//-----------------------------------------------
// LED Gloabals
int NUM_LEDS = 193;
int start_LED = 0;
int stop_LED = NUM_LEDS-1;
int MAX_BRIGHTNESS = 100;
int brightnessFactor = 122; // 50% 
int currentBrightness = map(brightnessFactor, 0, 255, 0, MAX_BRIGHTNESS);
const int LED_PIN = 2;
int LED_r = 255;
int LED_g = 0;
int LED_b = 0;
CRGB leds[500];

StaticJsonDocument<250> jsonDocument;
Preferences preferences;

void fillLEDs()
{
  Serial.println("FILL_LED");
  Serial.println("currentBrightness");
  Serial.println(currentBrightness);
  Serial.println(NUM_LEDS);
  Serial.println(start_LED);
  Serial.println(stop_LED);
  Serial.println(LED_r);
  Serial.println(LED_g);
  Serial.println(LED_b);
  for(int i=0;i<NUM_LEDS; i++)
  {
    if((i<start_LED) || (i>stop_LED))
    {
      leds[i].r = 0;
      leds[i].g = 0;
      leds[i].b = 0;
    }
    else
    {
      leds[i].r = map(LED_r, 0, 255, 0, currentBrightness);
      leds[i].g = map(LED_g, 0, 255, 0, currentBrightness);
      leds[i].b = map(LED_b, 0, 255, 0, currentBrightness);
    }
  }
  FastLED.show();
  FastLED.show();
}

void changeBrightness()
{
  if (server.hasArg("plain") == false) 
  {
    Serial.println("body not found");
  }
  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);

  brightnessFactor = jsonDocument["brightness"];
  preferences.putInt("brt-ness", brightnessFactor);
  currentBrightness = map(brightnessFactor, 0, 255, 0, MAX_BRIGHTNESS);
  Serial.println("currentBrightness:");
  Serial.println(brightnessFactor);
  Serial.println(currentBrightness);

  fillLEDs();
  server.send(200, "application/json", "{}");
}

void changeLEDConfig()
{
  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);

  NUM_LEDS = jsonDocument["num_leds"];
  start_LED = jsonDocument["start_led"];
  stop_LED = jsonDocument["stop_led"];
  //LED_PIN = jsonDocument["led_pin"];
  preferences.putInt("num_leds", NUM_LEDS);
  preferences.putInt("start_led", start_LED);
  preferences.putInt("stop_led", stop_LED);
  preferences.putInt("LED_PIN", LED_PIN);

  fillLEDs();
  server.send(200, "application/json", "{}");
}

void changeLEDColor()
{
  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);

  LED_r = jsonDocument["LED_r"];
  LED_g = jsonDocument["LED_g"];
  LED_b = jsonDocument["LED_b"];
  preferences.putInt("LED_r", LED_r);
  preferences.putInt("LED_g", LED_g);
  preferences.putInt("LED_b", LED_b);

  fillLEDs();
  server.send(200, "application/json", "{}");
}


void setup_routing() 
{          
  server.on("/brightness", HTTP_POST, changeBrightness);     
  server.on("/led_config", HTTP_POST, changeLEDConfig);
  server.on("/led_color", HTTP_POST, changeLEDColor);   
}

//-----------------------------------------------
String wifi_ssid = "shockwave";
String wifi_password = "ig0tkilledbyh0melander";
//-----------------------------------------------
// Set a Static IP address
IPAddress local_IP(192, 168, 29, 15);
// Set a Gateway IP address
IPAddress gateway(192, 168, 29, 1);
IPAddress subnet(255, 255, 255, 0);
//-----------------------------------------------
/*****************************************************************    
 *    webSocketEvent()
 * 
 *    For now the server only sends data so this is empty.
 *****************************************************************/
void webSocketEvent(uint8_t clientNum, WStype_t type, uint8_t *payload, size_t welength) 
{
  Serial.print("WStype = ");   Serial.println(type);  
  Serial.print("WS length = ");
  Serial.println(welength);  
  Serial.print("WS payload = ");
  for(int i = 0; i < welength; i++) { Serial.print((char) payload[i]); }
  Serial.println();
  
  if (type == WStype_TEXT)
  {  
    // Loop through LED array
    for (int i = 0; i <= (stop_LED - start_LED); i++){
      // Seperate payload data into individual colors
      // Get the first 5 bits of byte 1
      int red = *(payload + (3*i));
      // Get the last 3 bits of byte 1 and the first 3 bits of byte 2
      int green = *(payload + (3*i)+1);
      // Get the last 5 bits of byte 
      int blue = *(payload + (3*i)+2);
      leds[start_LED + i].r = map(red, 0, 255, 0, currentBrightness);
      leds[start_LED + i].g = map(green, 0, 255, 0, currentBrightness);
      leds[start_LED + i].b = map(blue, 0, 255, 0, currentBrightness);
    }
    // Refresh Display
    FastLED.show();
    FastLED.show();
  }
}
/*****************************************************************    
 *    webpage()
 * 
 *    This function sends the webpage to clients
 *****************************************************************/
void webpage()
{
  server.send(200,"text/html", htmlCode);
}


void readData()
{
  wifi_ssid = preferences.getString("wifi_ssid", wifi_ssid);
  wifi_password = preferences.getString("wifi_password", wifi_password);
  brightnessFactor = preferences.getInt("brt-ness", brightnessFactor);
  currentBrightness = map(brightnessFactor, 0, 255, 0, MAX_BRIGHTNESS);
  NUM_LEDS = preferences.getInt("num_leds", NUM_LEDS);
  start_LED = preferences.getInt("start_led", start_LED);
  stop_LED = preferences.getInt("stop_led", stop_LED);
  //LED_PIN = preferences.getInt("LED_PIN", LED_PIN);
  LED_r = preferences.getInt("LED_r", LED_r);
  LED_g = preferences.getInt("LED_g", LED_g);
  LED_b = preferences.getInt("LED_b", LED_b);
}

/*****************************************************************    
 *    setup()
 * 
 *    Connect to WiFi, start the server, start the websocket 
 *    connection, then start serial. Serial connection must be as 
 *    fast as possible for the best frame rate.
 *****************************************************************/
void setup() 
{
  Serial.begin(115200);
  Serial.println("1.");
  preferences.begin("my-led-app", false);
  Serial.println("2.");
  readData();
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.println("3.");

  // Connect to Wi-Fi network
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.println("wifi_ssid:");
  Serial.println(wifi_ssid.c_str());
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println(".");
    delay(500);
  }
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("4.");
  setup_routing();
  server.on("/", webpage);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("5.");
  fillLEDs();
}
/*****************************************************************    
 *    loop()
 * 
 *    Handles server and web socket. Reads data over serial then
 *    broadcasts it to all clients.
 *****************************************************************/
void loop() 
{
  server.handleClient();
  webSocket.loop();
}
