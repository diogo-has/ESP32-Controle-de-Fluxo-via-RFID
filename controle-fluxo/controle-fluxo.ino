#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>
#include <ESP32Servo.h>

#define SS_PIN 5
#define RST_PIN 22

#define RED_PIN 27
#define GREEN_PIN 25
#define BLUE_PIN 33

#define REED_PIN 13
#define BUZZER_PIN 32
#define SERVO_PIN 21

#define LOCKED_DEG 0
#define UNLOCKED_DEG 90

#define AUTH_TIME 150000

Servo lockServo;
MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "ESP32";
const char* password = "123456789";

bool start_auth = false;
bool auth = false;
bool failed_auth = false;
unsigned long auth_timeout = -AUTH_TIME;
bool start_newtag = false;
bool newtag = false;
bool failed_newtag = false;

AsyncWebServer server(80);


void setup() {
  Serial.begin(115200);

  struct timeval tv;
  tv.tv_sec = 1750629547;
  tv.tv_usec = 0;

  settimeofday(&tv, NULL);

  SPI.begin();
  rfid.PCD_Init();

  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCKED_DEG);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(REED_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setColor(255, 0, 0);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });

  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){
    if (millis() - auth_timeout < AUTH_TIME) {
      request->send(SPIFFS, "/dashboard-logs.html", String(), false);
    } else {
      auth = false;
      request->send(403, "text/plain", "Acesso Negado");
    }
  });

  server.on("/dashboard-logs", HTTP_GET, [](AsyncWebServerRequest *request){
    if (millis() - auth_timeout < AUTH_TIME) {
      request->send(SPIFFS, "/dashboard-logs.html", String(), false);
    } else {
      auth = false;
      request->send(403, "text/plain", "Acesso Negado");
    }
  });

  server.on("/dashboard-rooms", HTTP_GET, [](AsyncWebServerRequest *request){
    if (millis() - auth_timeout < AUTH_TIME) {
      request->send(SPIFFS, "/dashboard-rooms.html", String(), false);
    } else {
      auth = false;
      request->send(403, "text/plain", "Acesso Negado");
    }
  });

  server.on("/dashboard-users", HTTP_GET, [](AsyncWebServerRequest *request){
    if (millis() - auth_timeout < AUTH_TIME) {
      request->send(SPIFFS, "/dashboard-users.html", String(), false);
    } else {
      auth = false;
      request->send(403, "text/plain", "Acesso Negado");
    }
  });

  server.on("/new-user", HTTP_GET, [](AsyncWebServerRequest *request){
    if (millis() - auth_timeout < AUTH_TIME) {
      request->send(SPIFFS, "/new-user.html", String(), false);
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

  server.on("/img/door_icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/door_icon.png", "image/png");
  });

  server.on("/img/connection_icon.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/connection_icon.jpg", "image/jpeg");
  });

  server.on("/js/logs.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/logs.js", "application/javascript");
  });

  server.on("/js/users.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/users.js", "application/javascript");
  });

  server.on("/js/rooms.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/rooms.js", "application/javascript");
  });

  server.on("/js/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/index.js", "application/javascript");
  });

  server.on("/js/new-user.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/new-user.js", "application/javascript");
  });

  server.on("/data.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/data.json", "application/json");
  });

  server.on("/start_auth", HTTP_GET, [](AsyncWebServerRequest *request){
    start_auth = true;
    request->send(200, "text/plain", "ok");
  });

  server.on("/start_newtag", HTTP_GET, [](AsyncWebServerRequest *request){
    start_newtag = true;
    request->send(200, "text/plain", "ok");
  });

  server.on("/check_newtag", HTTP_GET, [](AsyncWebServerRequest *request){
    if (newtag) {
      newtag = false;
      request->send(200, "text/plain", "success");
    } else if (failed_newtag) {
      failed_auth = false; 
      request->send(200, "text/plain", "return");
    } else {
      request->send(200, "text/plain", "ok");
    }
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

  server.on("/add_user", HTTP_GET, [](AsyncWebServerRequest *request){
    String username, perm;
    if (request->hasParam("name")) {
      username = request->getParam("name")->value();
      perm = request->getParam("perm")->value();

      File file = SPIFFS.open("/data.json", "r");
      JsonDocument doc;
      deserializeJson(doc, file);
      file.close();

      size_t new_user = doc["users"].size() - 1;
      doc["users"][new_user]["id"] = new_user + 1;
      doc["users"][new_user]["name"] = username;
      doc["users"][new_user]["permissions"] = perm;

      file = SPIFFS.open("/data.json", "w");
      serializeJson(doc, file);
      file.close();
    }
    request->send(200, "text/plain", "ok");
  });

  server.on("/delete_user", HTTP_GET, [](AsyncWebServerRequest *request){
    int index = request->getParam("id")->value().toInt() - 1;
  
    File file = SPIFFS.open("/data.json", "r");
    JsonDocument doc;
    deserializeJson(doc, file);
    file.close();
  
    JsonArray users = doc["users"].as<JsonArray>();
    if (index < 0 || index >= users.size()) {
      request->send(400, "text/plain", "invalid");
      return;
    }
    users.remove(index);
    
    file = SPIFFS.open("/data.json", "w");
    serializeJson(doc, file);
    file.close();
  
    request->send(200, "text/plain", "ok");
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

  if (start_newtag) {
    start_newtag = false;
    checkNewTag();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  if (!digitalRead(REED_PIN)) {
    setColor(0, 255, 0);
    return;
  } else setColor(255, 0, 0);
  
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
  
  String username = "Acesso Negado";
  for (size_t i = 0; i < doc["users"].size(); i++) {
    bool same_tag = true;
    for (int j = 0; j < 4; j++)
      if ((int) rfid.uid.uidByte[j] != doc["users"][i]["tag"][j])
        same_tag = false;
  
    if (same_tag) {
      username = doc["users"][i]["name"].as<String>();
      break;
    }
  }
  entrada["name"] = username;

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
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}


void checkNewTag() {
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

    size_t new_user = doc["users"].size();
    for (int i = 0; i < 4; i++)
      doc["users"][new_user]["tag"][i] = rfid.uid.uidByte[i];

    doc["users"][new_user]["clearance"][0] = 1;

    file = SPIFFS.open("/data.json", "w");
    serializeJson(doc, file);
    file.close();
    
    newtag = true;
    setColor(255, 0, 0);
    return;
  }
  
  failed_newtag = true;
  setColor(255, 0, 0);
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
  bool door_closed = digitalRead(REED_PIN);
  unsigned long timeout = millis();
  setColor(0, 255, 0);
  digitalWrite(BUZZER_PIN, 1);
  lockServo.write(UNLOCKED_DEG);
  while (door_closed && millis() - timeout < 5000) {
    door_closed = digitalRead(REED_PIN);
  }
  while (!door_closed) {
    door_closed = digitalRead(REED_PIN);
  }
  delay(2000);
  digitalWrite(BUZZER_PIN, 0);
  lockServo.write(LOCKED_DEG);
  setColor(255, 0, 0);
}


void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);
}
