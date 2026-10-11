#include <painlessMesh.h>
#include <ArduinoJson.h>

#define MESH_PREFIX   "rasyactk"
#define MESH_PASSWORD "12345678"
#define MESH_PORT     5544

#define ledPin D6 // Pin D6

Scheduler userScheduler;
painlessMesh mesh;

// Menyimpan data terakhir kedua sensor
float suhuTerakhir = 0;
int adcTerakhir = 0;

bool sudahAdaSuhu = false;
bool sudahAdaCahaya = false;

unsigned long previousMillis = 0;
const long interval = 5000;

// Mengevaluasi kondisi LED
void cekKondisiLED() {
  bool suhuPanas = sudahAdaSuhu && suhuTerakhir > 31.0;
  bool cahayaGelap = sudahAdaCahaya && adcTerakhir < 300;

  if (suhuPanas || cahayaGelap) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED MENYALA");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("LED MATI");
  }
}

// Menerima dan memproses JSON
void receivedCallback(uint32_t from, String &msg) {
  StaticJsonDocument<200> doc;

  DeserializationError error = deserializeJson(doc, msg);

  if (error) {
    Serial.println("Gagal membaca JSON!");
    return;
  }

  String tipe = doc["tipe"].as<String>();

  Serial.println("==========================");

  if (tipe == "suhu_node") {
    Serial.println("Data dari Node 1 (Sensor DHT)");

    suhuTerakhir = doc["suhu"];
    float kelembapan = doc["kelembapan"];
    sudahAdaSuhu = true;

    Serial.printf("Suhu       : %.2f C\n", suhuTerakhir);
    Serial.printf("Kelembapan : %.2f %%\n", kelembapan);

  } else if (tipe == "cahaya_node") {
    Serial.println("Data dari Node 2 (Sensor LDR)");

    adcTerakhir = doc["adc"];
    sudahAdaCahaya = true;

    Serial.printf("ADC Cahaya : %d\n", adcTerakhir);

  } else {
    Serial.println("Tipe data tidak dikenal");
    return;
  }

  cekKondisiLED();

  Serial.println("==========================");
}

// Mendeteksi koneksi baru
void newConnectionCallback(uint32_t nodeId) {
  Serial.println("Koneksi Node baru terdeteksi!");
}

// Mendeteksi perubahan topologi
void changedConnectionCallback() {
  Serial.println("Topologi mesh berubah");
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  mesh.setDebugMsgTypes(ERROR | STARTUP);

  mesh.init(MESH_PREFIX, MESH_PASSWORD,
            &userScheduler, MESH_PORT);

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  Serial.println("==========================");
  Serial.println("Node 3 Hub Aktuator Siap!");
  Serial.println("Menunggu koneksi Mesh...");
  Serial.println("==========================");
}

void loop() {
  mesh.update();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (!sudahAdaSuhu || !sudahAdaCahaya) {
      Serial.println("Menunggu data dari Node 1 dan Node 2...");
    } else {
      Serial.println("Node 3 aktif - Monitoring sensor...");
    }
  }
}