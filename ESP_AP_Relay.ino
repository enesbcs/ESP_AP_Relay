#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>

#define APSSID "GateControl" // SSID & Title
#define APPASSWORD "12345678" // Blank password = Open AP
#define SUBTITLE "Local IoT"

#define RELAY 12
//#define LED 2

const byte HTTP_CODE = 200;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 217, 28, 1);
//IPAddress APIP(192, 168, 4, 1);

unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;"); a.replace(">", "&gt;");
  a.substring(0, 200); return a;
}

String footer() {
  return "<footer>"
         "<div class=\"footer\""
         " <p><b>Author:</b> <a href='https://bitekmindenhol.blog.hu/'>NS Tech</a> <b>2020</b></p>"
         "</div>"
         "</footer>";
}

String header() {
  String a = String(APSSID);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.35em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
               "input { border-radius: 0; border: 1px solid #555555; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }"
               ".button {"
               "background-color: #4CAF50;"
               "border: 1px solid black;"
               "border-radius: 6px;"
               "color: white;"
               "padding: 15px 32px;"
               "text-align: center;"
               "text-decoration: none;"
               "display: inline-block;"
               "font-size: 32px;width:100%;height:5em;"
               "margin: 4px 2px;"
               "cursor: pointer;"
               "}"
               ".buttonb { background-color: #555555; }"
               ".footer {"
               "position: fixed;"
               "left: 0;"
               "bottom: 0;"
               "width: 100%;"
               "background-color: #0066ff;"
               "color: white;"
               "text-align: center;"
               "font-family: \"Verdana\", Sans, serif;"
               "border-radius: 0px;"
               "height: 25px"
               "}";
  String h = "<!DOCTYPE html><html>"
             "<head><title>" + a + " :: " + SUBTITLE + "</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><br>";
  return h;
}

String index() {
  return header() +
         // 1st ONE:
         +"<center><table><tr><th><div><form action=/1ON method=post>" +
         +"<input type=submit class=\"button\" value=\"Nyit/Z&aacute;r\"></form></center></th></tr>" +
         +"</table></center><br>" + footer();
}


void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);
  #ifdef LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW); // The built-in led will notify us that the setup is finished.
  #endif
  digitalWrite(RELAY, LOW);
  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(APSSID, APPASSWORD);

  /* Setup the DNS server redirecting all the domains to the APIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", APIP);

  webServer.on("/1ON", []() {
    digitalWrite(RELAY, HIGH);
    webServer.send(HTTP_CODE, "text/html", index());
    Serial.println("Relay activated");
    delay(4000); // 4 sec
    digitalWrite(RELAY, LOW);
  });
  webServer.onNotFound([]() {
    //  webServer.on("/",[]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });

  ElegantOTA.begin(&webServer);
  webServer.begin();
  Serial.println("Webserver started");
  Serial.println(APIP);
  delay(1000);
  #ifdef LED
  digitalWrite(LED, HIGH);
  #endif
}

void loop() {
  if ((millis() - lastTick) > TICK_TIMER) {
    lastTick = millis();
  }
  dnsServer.processNextRequest();
  webServer.handleClient();
  delay(1);
}
