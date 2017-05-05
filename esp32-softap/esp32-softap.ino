/*

        ESP32 - Soft Access Point (Wi Fi Hot Spot)

        V 1.0        Brian Sutherland - University of Toronto Semaphore Labs

        Manifests a local access point and simple webserver, with better performance than ESP8266
        Based on ESP32 > WiFi > WiFiIPv6 Arduino code from core libraries

        Tested on:

        Sparkfun ESP32 Thing           [https://www.sparkfun.com/products/13907]
        Adafruit ESP32 Developer Board [https://www.adafruit.com/product/3269]


*/  

#include "WiFi.h"

#define AP_SSID "Semaphore_IoT"  // Name of Access Point
#define AP_PASS ""               // Note Wi Fi passwords are minimum 8 characters, or defaults to null

static volatile bool wifi_connected = false;

// defaults to 4 wifi clients (stations) only - go for 10
WiFiServer server(80, 10);

void WiFiEvent(WiFiEvent_t event){

    // event is an integer corresponding to a list in esp_event.h
    Serial.println(event);

    // translate into intelligible serial feedback
    switch(event) {
        case SYSTEM_EVENT_WIFI_READY:
            Serial.println("WIFI_READY");
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            Serial.println("WIFI_SCAN DONE");
            break;            
        case SYSTEM_EVENT_AP_STACONNECTED:
            Serial.println("Somebody connected to AP");
            break;
         case SYSTEM_EVENT_AP_STADISCONNECTED:
            Serial.println("Somebody disconnected from AP");
            break;
        case SYSTEM_EVENT_AP_START:
            Serial.println("ESP-32 Soft AP start");
            //can set ap hostname here
            WiFi.softAPsetHostname(AP_SSID);
            //enable ap ipv6 here
            WiFi.softAPenableIpV6();
            //
            Serial.println(WiFi.softAPIP());
            break;
        case SYSTEM_EVENT_AP_STOP:
            Serial.println("ESP-32 Soft AP stop");
            break;
        case SYSTEM_EVENT_STA_START:
            Serial.println("ESP-32 Station Start");
            //set sta hostname here
            WiFi.setHostname(AP_SSID);
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("ESP-32 Station Connected");
            //enable sta ipv6 here
            WiFi.enableIpV6();
            break;
        case SYSTEM_EVENT_AP_STA_GOT_IP6:
            Serial.println("ESP-32 Got IP6");

            // both interfaces get the same event
            Serial.print("STA IPv6: ");
            Serial.println(WiFi.localIPv6());
            Serial.print("AP IPv6: ");
            Serial.println(WiFi.softAPIPv6());

            // Start TCP (HTTP) server
            server.begin();
            Serial.println("HTTP server started");
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.println("ESP-32 Connected as WiFi Client");
            // wifiOnConnect();
            // wifi_connected = true;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("ESP-32 Disconnected from WiFi");
            // wifi_connected = false;
            // wifiOnDisconnect();
            break;
        default:
            break;
    }
}

void setup(){
  
    Serial.begin(115200);
    
    WiFi.disconnect(true);
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_AP);       // list of modes appears in WiFiType.h
    WiFi.softAP(AP_SSID, AP_PASS); // syntax appears in WiFiAP.h

    // onEvent starts the webserver, and only when the Soft-AP is ready

}

void loop(void)
{
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
        return;
    }


    // Wait for data from client to become available
    while(client.connected() && !client.available()){
        delay(1);
    }

    // Read the first line of HTTP request
    String req = client.readStringUntil('\r');

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
        Serial.print("Invalid request: ");
        Serial.println(req);
        return;
    }
    req = req.substring(addr_start + 1, addr_end);
    Serial.print("Request: ");
    Serial.println(req);
    client.flush();

    String s;
    if (req == "/")
    {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        s = "HTTP/1.1 200 OK\r\nContent-Type: text/html";
        s += "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1>Hello from Semaphore IoT</h1>";
        s += ipStr;
        s += "</html>\r\n\r\n";
        Serial.println("Sending 200");
    }
    else
    {
        s = "HTTP/1.1 404 Not Found\r\n\r\n";
        Serial.println("Sending 404");
    }
    client.print(s);

    Serial.println("Done with client");

}