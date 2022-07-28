#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>
#include "NTPClient.h"
#include "WiFiUdp.h"

#define LAND_TIME 9500 // The timing duration for the procedure of landing;
#define LIFT_TIME 10000 // The timing duration for the procedure of lifting;

#define MOTOR_ON_PIN 0 //The Pin for powering on the motor;
#define INVERT_ROTATION_PIN 2 //The Pin for Inverting Rotation of the motor;

int timeclock = 0, seconds = 0, minutes = 0, hours = 0;
const long OffsetSeconds = -10800; // Need to change per geographic region;
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.ntp.br", OffsetSeconds); // Change NTP Server as needed;
WiFiClient client;
bool inoper = false;


/// Model V1.0, IGNUM01 protocol will be implemented in future versions, still need a lot of work in automation parts;

/// Main Code for the SPARTAN_SMART_CURTAIN Project, a simplified version without real command security,
/// designed for Home/Learning uses, not designed for industrial applications, free code to modify as 
/// needed, developed by Doctor Machine (also known as Felipe);  

void setup(void) {
  
  pinMode(MOTOR_ON_PIN, OUTPUT);
  pinMode(INVERT_ROTATION_PIN, OUTPUT);

 
  Serial.begin(115200);
  
  Serial.println("\nStarting System...");
  WiFi.mode(WIFI_STA);
  WiFi.hostname("SPARTAN_CURTAIN");
  ESP8266WiFiMulti wifiMultiSSID;

  wifiMultiSSID.addAP("1st_Wifi_Network", "wifi_password");
  wifiMultiSSID.addAP("2nd_Wifi_Network", "wifi_password"); 
  
  if (wifiMultiSSID.run() == WL_CONNECTED)
      {
      timeClient.begin();

      Serial.print("Connected to ");
      Serial.println(WiFi.SSID());
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      
      if (MDNS.begin("esp8266")) {
        Serial.println("MDNS started");
      }
    }

  server.on("/", []{
      
    String report = "Spartan Smart Curtain\nSlave_Client_1.0\n";
    report += "\n" + String (timeClient.getHours()) + ":" + String(timeClient.getMinutes());
    server.send(200, "text/plain", report);
  });

  server.on("/caselev", [] {
  Lift_Mode();
  server.send(200, "text/plain", "lifting_Curtains\n");
  });
      
  server.on("/casedes", [] { 
  Land_Mode();
  server.send(200, "text/plain", "landing_Curtains\n");
  });
   
  server.begin();
}

void loop(void) {
  
  server.handleClient();
  MDNS.update();

  //UDP Clock Check;
      
      if (timeclock == 200){
      timeClient.update();

      seconds = timeClient.getSeconds();
      minutes = timeClient.getMinutes();
      hours = timeClient.getHours();
      timeclock = 0;
         
        //scheduled tasks example;
        if ((hours == 6) && (minutes == 0) && (seconds <= 8)){
            Lift_Mode();  
        }
   
      }
 
  timeclock++;
  delay(100);    

}

void Lift_Mode(){
  
  if (!inoper){
    digitalWrite(MOTOR_ON_PIN, HIGH);
    inoper = true;
    delay(LIFT_TIME); // the timing for operation of the motor
    inoper = false;
    digitalWrite(MOTOR_ON_PIN, LOW);
  }
}

void Land_Mode(){

  if (!inoper){
    inoper = true;
    digitalWrite(MOTOR_ON_PIN, HIGH);
    digitalWrite(INVERT_ROTATION_PIN, HIGH);
    delay(LAND_TIME);
    inoper = false;
    digitalWrite(MOTOR_ON_PIN, LOW);
    digitalWrite(INVERT_ROTATION_PIN, LOW);
  }
}
