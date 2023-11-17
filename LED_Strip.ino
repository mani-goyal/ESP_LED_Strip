#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Preferences.h>
#include "html.h"
#include <FastLED.h>
#include "constants.h"
#include <string.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
//-----------------------------------------------
// LED Gloabals
int NUM_LEDS;// = 193;
int start_LED;// = 0;
int stop_LED;// = NUM_LEDS-1;
int MAX_BRIGHTNESS = 100;
int brightnessFactor;// = 122; // 50% 
int currentBrightness = map(brightnessFactor, 0, 255, 0, MAX_BRIGHTNESS);
const int LED_PIN = 2;
int LED_r = 255;
int LED_g = 0;
int LED_b = 0;
CRGB leds[500];

String wifi_ssid; // = "shockwave";
String wifi_password; // = "ig0tkilledbyh0melander";


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

void changeBrightness(StaticJsonDocument<250> a_jsonDocument)
{
    if(!IS_KEY_PRESENT(a_jsonDocument, "brightness"))
    {
        return;
    }
    brightnessFactor = a_jsonDocument["brightness"];
    preferences.putInt("brt-ness", brightnessFactor);
    currentBrightness = map(brightnessFactor, 0, 255, 0, MAX_BRIGHTNESS);
    fillLEDs();
}

void changeLEDConfig(StaticJsonDocument<250> a_jsonDocument)
{
    if(IS_KEY_PRESENT(a_jsonDocument, "num_leds"))
        NUM_LEDS = a_jsonDocument["num_leds"];

    if(IS_KEY_PRESENT(a_jsonDocument, "start_led"))
        start_LED = a_jsonDocument["start_led"];

    if(IS_KEY_PRESENT(a_jsonDocument, "stop_led"))
        stop_LED = a_jsonDocument["stop_led"];

    preferences.putInt("num_leds", NUM_LEDS);
    preferences.putInt("start_led", start_LED);
    preferences.putInt("stop_led", stop_LED);

    fillLEDs();
}

int StrToHex(std::string strHex)
{
    return (int) strtol(strHex.c_str(), 0, 16);
}

void changeLEDColor(StaticJsonDocument<250> a_jsonDocument)
{
    String body = server.arg("plain");
    Serial.println(body);
    deserializeJson(jsonDocument, body);

    if(!IS_KEY_PRESENT(a_jsonDocument, "led_color"))
        return;

    std::string hexCode = jsonDocument["led_color"];
    if(hexCode.at(0) == '#') 
    {
        hexCode.erase(0, 1);
    }
    if(hexCode.length() != 6)
        return;

    LED_r = StrToHex(hexCode.substr(0,2));
    LED_g = StrToHex(hexCode.substr(2,2));
    LED_b = StrToHex(hexCode.substr(4,2));

    Serial.println(hexCode.c_str());
    Serial.println(hexCode.substr(0,2).c_str());
    Serial.println(StrToHex(hexCode.substr(0,2)));
    Serial.println(hexCode.substr(2,2).c_str());
    Serial.println(StrToHex(hexCode.substr(2,2)));
    Serial.println(hexCode.substr(4,2).c_str());
    Serial.println(StrToHex(hexCode.substr(4,2)));

    preferences.putString("led_color", hexCode.c_str());
    fillLEDs();
}

void changeWifiSettings(StaticJsonDocument<250> a_jsonDocument)
{
    if(!IS_KEY_PRESENT(a_jsonDocument, "wifi_ssid") && !IS_KEY_PRESENT(a_jsonDocument, "wifi_password"))
        return; 

    String new_wifi_ssid = jsonDocument["wifi_ssid"];
    String new_wifi_passwd = jsonDocument["wifi_password"];
    preferences.putString("wifi_ssid", new_wifi_ssid);
    preferences.putString("wifi_password", new_wifi_passwd);

    WiFi.disconnect();
    WiFi.begin(new_wifi_ssid, new_wifi_passwd);
}

void PerformOperation()
{
    if (server.hasArg("plain") == false) 
    {
        server.send(400, "application/json", "{\"error\": \"Body not passed\"}");
    }
    String strJsonBody = server.arg("plain");
    
    deserializeJson(jsonDocument, strJsonBody);

    if(!IS_KEY_PRESENT(jsonDocument, "op_type"))
    {
        server.send(400, "application/json", "{\"error\": \"Operation type not passed\"}");
    }

    int iOpType = jsonDocument["op_type"];
    switch (iOpType)
    {
        case CHANGE_BRIGHTNESS:
            changeBrightness(jsonDocument);
            break;
        case CHANGE_LED_CONFIG:
            changeLEDConfig(jsonDocument);
            break;
        case CHANGE_LED_COLOR:
            changeLEDColor(jsonDocument);
            break;
        case CHANGE_WIFI_SETTINGS:
            changeWifiSettings(jsonDocument);
        default:
            break;
    }
    server.send(200, "application/json", "{}");

}

void GetConfig()
{
    DynamicJsonDocument jsonResponse(2048);
    jsonResponse["wifi_ssid"] = wifi_ssid;
    jsonResponse["wifi_password"] = wifi_password;
    jsonResponse["brightnessFactor"] = brightnessFactor;
    jsonResponse["num_leds"] = NUM_LEDS;
    jsonResponse["start_led"] = start_LED;
    jsonResponse["stop_led"] = stop_LED;
    jsonResponse["led_color"] = preferences.getString("led_color");

    String strResponse;
    serializeJsonPretty(jsonResponse, strResponse);

    server.send(200, "application/json", strResponse.c_str());
    
}

void setup_routing() 
{          
    server.on("/perform_operation", HTTP_POST, PerformOperation);
    server.on("/get_config", GetConfig);
}

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
        for (int i = 0; i <= (stop_LED - start_LED); i++)
        {
            if(!(payload + (3*i)) ||
                !(payload + (3*i) +1 ) ||
                !(payload + (3*i) +2 ) )
                {
                    break;
                }
            int red = *(payload + (3*i));
            int green = *(payload + (3*i)+1);
            int blue = *(payload + (3*i)+2);
            leds[start_LED + i].r = map(red, 0, 255, 0, currentBrightness);
            leds[start_LED + i].g = map(green, 0, 255, 0, currentBrightness);
            leds[start_LED + i].b = map(blue, 0, 255, 0, currentBrightness);
        }
        FastLED.show();
        FastLED.show();

        webSocket.sendTXT(clientNum, "0");
    }
}

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

    std::string hexCode = preferences.getString("led_color").c_str();
    LED_r = StrToHex(hexCode.substr(0,2));
    LED_g = StrToHex(hexCode.substr(2,2));
    LED_b = StrToHex(hexCode.substr(4,2));
}


void setup() 
{
    Serial.begin(115200);
    preferences.begin("my-led-app", false);
    readData();
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

    // Connect to Wi-Fi network
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    Serial.print("\nwifi_ssid:");
    Serial.println(wifi_ssid.c_str());
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print(".");
        delay(500);
    }
    Serial.print("\nIP address: ");
    Serial.println(WiFi.localIP());

    setup_routing();
    server.on("/", webpage);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    fillLEDs();
}

void loop() 
{
  server.handleClient();
  webSocket.loop();
}
