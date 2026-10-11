#include <painlessMesh.h>
#include <ArduinoJson.h>

#define MESH_PREFIX   "rasyactk"
#define MESH_PASSWORD "12345678"
#define MESH_PORT     5544

#define LDRPIN A0

const char* nodeName = "Node 2";

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();
Task taskSendMessage(TASK_SECOND * 3, TASK_FOREVER, &sendMessage);

// Mengirim data sensor LDR
void sendMessage() {
  int adc = analogRead(LDRPIN);

  StaticJsonDocument<200> doc;
  doc["node"] = nodeName;
  doc["tipe"] = "cahaya_node";
  doc["adc"] = adc;

  String msg;
  serializeJson(doc, msg);

  mesh.sendBroadcast(msg);

  Serial.println("==========================");
  Serial.println("Node 2 Mengirim Data LDR");
  Serial.printf("ADC Cahaya : %d\n", adc);
  Serial.println("==========================");
}

// Menerima data dari node lain
void receivedCallback(uint32_t from, String &msg) {
  StaticJsonDocument<200> doc;

  DeserializationError error = deserializeJson(doc, msg);

  if (error) {
    Serial.println("Gagal membaca JSON!");
    return;
  }

  String namaNode = doc["node"].as<String>();

  if (namaNode != "") {
    Serial.print("Data diterima dari ");
    Serial.println(namaNode);
  }
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

  mesh.setDebugMsgTypes(ERROR | STARTUP);

  mesh.init(MESH_PREFIX, MESH_PASSWORD,
            &userScheduler, MESH_PORT);

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  Serial.println("==========================");
  Serial.println("Node 2 Sensor LDR Siap!");
  Serial.println("Menunggu koneksi Mesh...");
  Serial.println("==========================");
}

void loop() {
  mesh.update();
}