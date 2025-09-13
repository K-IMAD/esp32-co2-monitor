// Full working example (Arduino-style) combining your sensor code + MQTT publish
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
#include <time.h>

// replace these
const char* ssid = "ALHN-918E";
const char* wifi_pass = "2670429197";

const char* mqtt_host = "495e05bee6cb40cd97eeb41fc597850e.s1.eu.hivemq.cloud"; // e.g. abc123.s2.eu.hivemq.cloud
const int   mqtt_port = 8883;            // TLS port for TCP
const char* mqtt_user = "esp32-device1";
const char* mqtt_pass = "Password@2025";
const char* device_id = "esp32-co2-01";

LiquidCrystal_I2C lcd(0x27, 16, 2);
HardwareSerial sensor1(1);
HardwareSerial sensor2(2);

byte cmd_get_sensor[] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};

WiFiClientSecure net;
PubSubClient mqttClient(net);

unsigned long lastPublish = 0;
const unsigned long publishIntervalMs = 10 * 1000; // publish every 10s (adjust as needed)

int readCO2(HardwareSerial &serial) {
  byte response[9];
  serial.write(cmd_get_sensor, 9);  
  delay(100);                      

  if (serial.available() >= 9) {
    serial.readBytes(response, 9);
    if (response[0] == 0xFF && response[1] == 0x86) {
      return (response[2] << 8) | response[3];
    }
  }
  return -1; 
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  // handle incoming commands if you want (not used here)
  Serial.print("Incoming ["); Serial.print(topic); Serial.print("]: ");
  for (unsigned int i=0;i<len;i++) Serial.print((char)payload[i]);
  Serial.println();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(900);
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
}

void setupTime() {
  // set timezone offset (seconds). Algeria is UTC+1 -> 3600
  const long gmtOffset_sec = 3600;
  const int daylightOffset_sec = 0;
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time");
  time_t now = time(nullptr);
  int tries = 0;
  while (now < 1609459200UL && tries < 20) { // wait until 2021-01-01 roughly
    Serial.print(".");
    delay(1000);
    now = time(nullptr);
    tries++;
  }
  Serial.println();
  if (now < 1609459200UL) Serial.println("Warning: NTP time not set, TLS may fail");
  else Serial.println("NTP OK");
}

void setupMQTT() {
  mqttClient.setServer(mqtt_host, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting MQTT...");
    // clientId should be unique
    String clientId = String(device_id) + "-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // subscribe to a command topic if you want:
      String cmdTopic = String("sensors/") + device_id + "/cmd";
      mqttClient.subscribe(cmdTopic.c_str(), 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  lcd.init();            
  lcd.backlight();        
  lcd.setCursor(0, 0);    
  lcd.print("CO2 Monitor");

  sensor1.begin(9600, SERIAL_8N1, 16, 17);
  sensor2.begin(9600, SERIAL_8N1, 4, 2);

  connectWiFi();
  setupTime();

  // TLS: production -> use setCACert(root_ca_pem)
  // Quicktest: use setInsecure() (disables certificate validation) -> NOT for production
  net.setInsecure(); // REMOVE for production and use setCACert()
  // Example: net.setCACert(your_root_ca_pem_string);

  setupMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  if (millis() - lastPublish >= publishIntervalMs) {
    lastPublish = millis();

    int co2_1 = readCO2(sensor1);
    int co2_2 = readCO2(sensor2);

    // show on LCD
    lcd.clear();
    lcd.setCursor(0,0);
    if (co2_1 > 0) {
      lcd.print("S1: "); lcd.print(co2_1); lcd.print(" ppm");
    } else lcd.print("S1: Err");
    lcd.setCursor(0,1);
    if (co2_2 > 0) {
      lcd.print("S2: "); lcd.print(co2_2); lcd.print(" ppm");
    } else lcd.print("S2: Err");

    // make JSON payload
    StaticJsonDocument<256> doc;
    doc["device"] = device_id;
    doc["co2_1"] = co2_1;
    doc["co2_2"] = co2_2;
    doc["ts"] = time(nullptr); // epoch seconds

    char payload[256];
    size_t n = serializeJson(doc, payload, sizeof(payload));

    String topic = String("sensors/") + device_id + "/co2";
    //bool ok = mqttClient.publish(topic.c_str(),const uint8_t*) payload,false);
    Serial.print("Published to "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload);
    //if (!ok) Serial.println("Publish failed");
  }
}
