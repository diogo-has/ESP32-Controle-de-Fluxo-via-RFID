#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>

#define SS_PIN 5
#define RST_PIN 22

#define RED_PIN 27
#define GREEN_PIN 25
#define BLUE_PIN 33

MFRC522 rfid(SS_PIN, RST_PIN);

byte nuidPICC[4];

const char* ssid = "ESP32";
const char* password = "123456789";

bool start_auth = false;
bool auth = false;
bool failed_auth = false;
unsigned long auth_timeout;

AsyncWebServer server(80);

String processor(const String& var) {
  return String();
}

void setup() {
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  setColor(255, 0, 0);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){
    if (millis() - auth_timeout < 60000) {
      request->send(SPIFFS, "/dashboard.html", String(), false, processor);
    } else {
      auth = false;
      request->send(403, "text/plain", "Acesso Negado");
    }
  });

  server.on("/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/style.css", "text/css");
  });

  server.on("/img/person_icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/person_icon.png", "image/png");
  });

  server.on("/img/room_icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/room_icon.png", "image/png");
  });

  server.on("/img/connection_icon.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/connection_icon.jpg", "image/jpeg");
  });

  server.on("/js/dashboard.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/dashboard.js", "application/javascript");
  });

  server.on("/js/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/index.js", "application/javascript");
  });

  server.on("/data.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/data.json", "application/json");
  });

  server.on("/start_auth", HTTP_GET, [](AsyncWebServerRequest *request){
    start_auth = true;
    request->send(200, "text/plain", "ok");
  });

  server.on("/check_redirect", HTTP_GET, [](AsyncWebServerRequest *request){
    if (auth) {
      auth_timeout = millis();
      request->send(200, "text/plain", "redirect");
    } else if (failed_auth) {
      failed_auth = false;
      request->send(200, "text/plain", "reload");
    } else {
      request->send(200, "text/plain", "ok");
    }
  });
  

  server.begin();
}

void loop() {
  if (start_auth) {
    start_auth = false;
    checkAccess();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }
  
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if ( ! rfid.PICC_ReadCardSerial())
    return;

  File file = SPIFFS.open("/data.json", "r");
  JsonDocument doc;
  deserializeJson(doc, file);
  file.close();

  JsonDocument entrada;
  entrada["time"] = time(NULL);
  for (byte i = 0; i < 4; i++)
    entrada["tag"][i] = rfid.uid.uidByte[i];

  size_t next_entry = doc["rooms"][0]["logs"].size();
  doc["rooms"][0]["logs"][next_entry] = entrada;
  
  file = SPIFFS.open("/data.json", "w");
  serializeJson(doc, file);
  file.close();

  bool same_tag;
  for (size_t i = 0; i < doc["users"].size(); i++) {
    same_tag = true;
    for (int j = 0; j < 4; j++)
      if ((int) rfid.uid.uidByte[j] != doc["users"][i]["tag"][j]) same_tag = false;
      
    if (same_tag) {
      toggleDoor();
      break;
    }
  }

  if (!same_tag) {
    setColor(0, 0, 0);
    delay(200);
    setColor(255, 0, 0);
    delay(200);
    setColor(0, 0, 0);
    delay(200);
    setColor(255, 0, 0);
  }

    // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void checkAccess() {

  setColor(0, 0, 255);
  
  File file = SPIFFS.open("/data.json", "r");
  JsonDocument doc;
  deserializeJson(doc, file);
  file.close();
  
  unsigned long timeout_timer = millis();
  while (millis() - timeout_timer < 10000) {
    if ( ! rfid.PICC_IsNewCardPresent())
      continue;
  
    if ( ! rfid.PICC_ReadCardSerial())
      continue;

    bool same_tag;
    for (size_t i = 0; i < doc["users"].size(); i++) {
      same_tag = true;
      for (int j = 0; j < 4; j++)
        if ((int) rfid.uid.uidByte[j] != doc["users"][i]["tag"][j]) same_tag = false;
        
      if (same_tag && doc["users"][i]["permissions"] == "admin") {
        auth = true;
        setColor(0, 255, 0);
        delay(50);
        setColor(0, 0, 0);
        delay(50);
        setColor(0, 255, 0);
        delay(50);
        setColor(0, 0, 0);
        delay(50);
        setColor(255, 0, 0);
        return;
      }
    }

    failed_auth = true;
    setColor(255, 0, 0);
    delay(50);
    setColor(0, 0, 0);
    delay(50);
    setColor(255, 0, 0);
    delay(50);
    setColor(0, 0, 0);
    delay(50);
    setColor(255, 0, 0);
    return;
  }
  
  failed_auth = true;
  setColor(255, 0, 0);
}

void toggleDoor() {
  setColor(0, 255, 0);
  delay(2000);
  setColor(255, 0, 0);
}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);
}

//int compareTags(byte *tag1, byte *tag2) {
//    for (int i = 0; i < 4; i++)
//      if (tag1[i] != tag2[i]) return 0;
//    return 1;
//}
