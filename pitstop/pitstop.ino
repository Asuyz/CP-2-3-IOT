#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ====================== CONFIG ======================
#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define API_BASE_URL  "http://10.233.139.162:8000"
#define PAGER_ID      "PAGER-001"

// Polling a cada 1 minuto
#define POLL_INTERVAL 60000

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
void postLocation();
void updateDisplay();
void setStatusVisual(String status);
void startBeep(int times);
void buzzerLoop();
void buttonLoop();
void gpsLoop();

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);

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

  // refresh leve do display (1s)
  if (now - lastDisplayRefresh >= 1000) {
    lastDisplayRefresh = now;
    updateDisplay();
  }
}

// ====================== WIFI ======================
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    // sem delay (espera por polling)
  }

  wifiStatus = (WiFi.status() == WL_CONNECTED) ? "OK" : "OFF";
}

// ====================== API ======================
void pollStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiStatus = "OFF";
    connectWiFi();
    return;
  }

  String url = String(API_BASE_URL) + "/pager/" + PAGER_ID;

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  Serial.print("GET: "); Serial.println(url);
  Serial.print("HTTP: "); Serial.println(httpCode);
  if (httpCode > 0) {
    Serial.println(http.getString());
  }

  if (httpCode == 200) {
    apiStatus = "OK";
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    String novoStatus = doc["status"] | statusAtual;

    carPlate   = doc["car_plate"] | "---";
    carModel   = doc["car_model"] | "---";
    ownerName  = doc["owner_name"] | "---";
    serviceDesc= doc["service_desc"] | "---";

    if (novoStatus != statusAtual) {
      statusAtual = novoStatus;
      setStatusVisual(statusAtual);

      // buzzer
      if (statusAtual == "aguardando") startBeep(1);
      else if (statusAtual == "diagnostico") startBeep(2);
      else if (statusAtual == "manutencao") startBeep(2);
      else if (statusAtual == "aprovacao") startBeep(4);
      else if (statusAtual == "pronto") startBeep(3);

      postLocation();
    }

  } else {
    apiStatus = "OFF";
  }

  http.end();
}

void postLocation() {
  if (!gps.location.isValid()) return;

  String url = String(API_BASE_URL) + "/pager/" + PAGER_ID + "/location";

  DynamicJsonDocument doc(128);
  doc["lat"] = gps.location.lat();
  doc["lng"] = gps.location.lng();

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.POST(body);
  http.end();
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

    if (buzzer.toneOn) {
      tone(BUZZER_PIN, 1000);
    } else {
      noTone(BUZZER_PIN);
    }

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