#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ====================== CONFIG ======================
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define API_BASE_URL  "http://host.wokwi.internal:8000"  // Wokwi -> host; no ESP32 real use o IP da máquina
#define PAGER_ID      "PAGER-001"

// Polling a cada 1 minuto
#define POLL_INTERVAL 60000

// Postar localização periodicamente (mesmo sem mudar status)
#define LOCATION_INTERVAL 120000  // 2 minutos

// ====================== DISPLAY ======================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====================== PINOS ======================
#define LED_R 25
#define LED_G 26
#define LED_B 27

#define BUZZER_PIN 19
#define BUTTON_PIN 18

#define GPS_RX 16
#define GPS_TX 17
#define GPS_BAUD 9600

// ====================== OBJETOS ======================
TinyGPSPlus gps;

// ====================== ESTADOS ======================
String statusAtual = "aguardando";
String carPlate = "---";
String carModel = "---";
String ownerName = "---";
String serviceDesc = "---";
String wifiStatus = "OFF";
String apiStatus = "OFF";

int telaAtual = 0;
unsigned long lastPoll = 0;
unsigned long lastDisplayRefresh = 0;
unsigned long lastLocationPost = 0;

// Botão
bool lastButtonState = HIGH;

// Buzzer não-bloqueante
struct BuzzerState {
  bool active = false;
  int remaining = 0;
  bool toneOn = false;
  unsigned long lastToggle = 0;
} buzzer;

// ====================== FUNÇÕES ======================
void connectWiFi();
void pollStatus();
bool postLocation();
void updateDisplay();
void setStatusVisual(String status);
void startBeep(int times);
void buzzerLoop();
void buttonLoop();
void gpsLoop();

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("BOOT ESP32 OK");
  Serial.print("Sketch: ");
  Serial.println(__FILE__);
  
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  connectWiFi();
  updateDisplay();

  // Faz um GET logo no começo (pra preencher a tela)
  pollStatus();
}

// ====================== LOOP ======================
void loop() {
  gpsLoop();
  buzzerLoop();
  buttonLoop();

  unsigned long now = millis();

  // polling a cada 1 minuto
  if (now - lastPoll >= POLL_INTERVAL) {
    lastPoll = now;
    pollStatus();
  }

  // POST de localização periódico (independente de mudar status)
  if (now - lastLocationPost >= LOCATION_INTERVAL) {
    lastLocationPost = now;
    postLocation();
  }

  // refresh leve do display (1s)
  if (now - lastDisplayRefresh >= 1000) {
    lastDisplayRefresh = now;
    updateDisplay();
  }
}

// ====================== WIFI ======================
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    // sem delay (espera por polling)
  }

  wifiStatus = (WiFi.status() == WL_CONNECTED) ? "OK" : "OFF";
  Serial.print("WiFi: ");
  Serial.println(wifiStatus);
}

// ====================== API ======================
void pollStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiStatus = "OFF";
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) return;
  }

  String url = String(API_BASE_URL) + "/pager/" + PAGER_ID;

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  Serial.print("GET: ");
  Serial.println(url);
  Serial.print("HTTP(GET): ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    apiStatus = "OK";
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
      Serial.print("JSON err: ");
      Serial.println(err.c_str());
      http.end();
      return;
    }

    String novoStatus = doc["status"] | statusAtual;

    // Atualiza SEM depender de mudança de status (assim a tela sempre preenche)
    statusAtual  = novoStatus;
    carPlate     = doc["car_plate"] | "---";
    carModel     = doc["car_model"] | "---";
    ownerName    = doc["owner_name"] | "---";
    serviceDesc  = doc["service_desc"] | "---";

  } else {
    apiStatus = "OFF";
  }

  http.end();
}

bool postLocation() {
  Serial.println("postLocation() chamado");

  if (WiFi.status() != WL_CONNECTED) {
    wifiStatus = "OFF";
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Sem WiFi, nao vai postar");
      return false;
    }
  }

  // Sem GPS válido = não posta
  if (!gps.location.isValid()) {
    Serial.println("GPS INVALIDO - nao vai postar");
    return false;
  }

  double lat = gps.location.lat();
  double lng = gps.location.lng();

  Serial.print("GPS OK lat=");
  Serial.print(lat, 6);
  Serial.print(" lng=");
  Serial.println(lng, 6);

  String url = String(API_BASE_URL) + "/pager/" + PAGER_ID + "/location";

  DynamicJsonDocument doc(128);
  doc["lat"] = lat;
  doc["lng"] = lng;

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(body);

  Serial.print("POST: ");
  Serial.println(url);
  Serial.print("Body: ");
  Serial.println(body);
  Serial.print("HTTP(POST): ");
  Serial.println(code);

  if (code > 0) {
    Serial.println(http.getString());
  }

  http.end();
  return (code == 200 || code == 201);
}

// ====================== DISPLAY ======================
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (telaAtual == 0) {
    display.setCursor(0,0);
    display.println("PITSTOP PAGER");
    display.println("----------------");
    display.println("STATUS:");
    display.println(statusAtual);
  }
  else if (telaAtual == 1) {
    display.setCursor(0,0);
    display.println("VEICULO");
    display.println("----------------");
    display.println(carPlate);
    display.println(carModel);
    display.println(ownerName);
  }
  else if (telaAtual == 2) {
    display.setCursor(0,0);
    display.println("SERVICO");
    display.println("----------------");
    display.println(serviceDesc);
  }
  else if (telaAtual == 3) {
    display.setCursor(0,0);
    display.println("PAGER ID");
    display.println("----------------");
    display.println(PAGER_ID);
    display.print("WiFi: ");
    display.println(wifiStatus);
    display.print("API:  ");
    display.println(apiStatus);
  }

  display.display();
}

// ====================== LED ======================
void setStatusVisual(String status) {
  int r=0, g=0, b=0;

  if (status == "aguardando") { r=40; g=40; b=40; }
  else if (status == "diagnostico") { r=255; g=200; b=0; }
  else if (status == "manutencao") { r=0; g=0; b=255; }
  else if (status == "aprovacao") { r=255; g=0; b=0; }
  else if (status == "pronto") { r=0; g=255; b=0; }

  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

// ====================== BUZZER (sem delay) ======================
void startBeep(int times) {
  buzzer.active = true;
  buzzer.remaining = times * 2; // on/off ciclos
  buzzer.toneOn = false;
  buzzer.lastToggle = millis();
}

void buzzerLoop() {
  if (!buzzer.active) return;

  unsigned long now = millis();
  if (now - buzzer.lastToggle >= 200) {
    buzzer.lastToggle = now;
    buzzer.toneOn = !buzzer.toneOn;

    if (buzzer.toneOn) tone(BUZZER_PIN, 1000);
    else noTone(BUZZER_PIN);

    buzzer.remaining--;
    if (buzzer.remaining <= 0) {
      buzzer.active = false;
      noTone(BUZZER_PIN);
    }
  }
}

// ====================== BOTÃO ======================
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 300; // ajuste aqui (ms)

void buttonLoop() {
  bool state = digitalRead(BUTTON_PIN);

  if (state == LOW && lastButtonState == HIGH) {
    unsigned long now = millis();
    if (now - lastButtonTime > debounceDelay) {
      telaAtual = (telaAtual + 1) % 4;
      updateDisplay();
      lastButtonTime = now;
    }
  }
  lastButtonState = state;
}

// ====================== GPS ======================
void gpsLoop() {
  while (Serial2.available()) {
    gps.encode(Serial2.read());
  }
}