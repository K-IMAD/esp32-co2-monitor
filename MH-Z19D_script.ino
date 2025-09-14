// ESP32 CO2 Monitor with MQTT
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
 #include <ArduinoJson.h> // Commented out - we'll create JSON manually
 #include <LiquidCrystal_I2C.h> // Commented out - LCD functionality disabled
#include <HardwareSerial.h>

// WiFi credentials
const char* ssid = "ALHN-6208";
const char* wifi_pass = "juventus+1897";

// MQTT settings
const char* mqtt_host = "495e05bee6cb40cd97eeb41fc597850e.s1.eu.hivemq.cloud";
const int   mqtt_port = 8883;            // TLS port
const char* mqtt_user = "esp32-device1";
const char* mqtt_pass = "Password@2025";
const char* device_id = "esp32-co2-01";

// Hardware setup
 LiquidCrystal_I2C lcd(0x27, 16, 2); // Commented out - LCD disabled
HardwareSerial sensor1(1); // Use UART1
HardwareSerial sensor2(2); // Use UART2

byte cmd_get_sensor[] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};

// LED pins for ESP32
int greenLED = 25;
int yellowLED = 26;
int redLED = 27;

// MQTT and WiFi clients
WiFiClientSecure net;
PubSubClient mqttClient(net);

unsigned long lastPublish = 0;
const unsigned long publishIntervalMs = 10 * 1000; // publish every 10s

int readCO2(HardwareSerial &serial) {
  byte response[9];
  
  // Clear any existing data
  while(serial.available()) {
    serial.read();
  }
  
  serial.write(cmd_get_sensor, 9);  
  delay(200); // Give time for response
  
  int timeout = 0;
  while(serial.available() < 9 && timeout < 50) {
    delay(10);
    timeout++;
  }
  
  if (serial.available() >= 9) {
    serial.readBytes(response, 9);
    if (response[0] == 0xFF && response[1] == 0x86) {
      return (response[2] << 8) | response[3];
    }
  }
  return -1; 
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Incoming ["); 
  Serial.print(topic); 
  Serial.print("]: ");
  for (unsigned int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_pass);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    Serial.print(".");
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void setupTime() {
  // Set timezone for Algeria (UTC+1 = 3600 seconds)
  const long gmtOffset_sec = 3600;
  const int daylightOffset_sec = 0;
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
  
  Serial.print("Waiting for NTP time");
  time_t now = time(nullptr);
  int tries = 0;
  while (now < 1609459200UL && tries < 30) { // wait until 2021-01-01 roughly
    Serial.print(".");
    delay(1000);
    now = time(nullptr);
    tries++;
  }
  Serial.println();
  
  if (now < 1609459200UL) {
    Serial.println("Warning: NTP time not set, TLS may fail");
  } else {
    Serial.println("NTP time synchronized");
    Serial.print("Current time: ");
    Serial.println(ctime(&now));
  }
}

void setupMQTT() {
  mqttClient.setServer(mqtt_host, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
  // For testing: disable certificate verification (NOT for production!)
  net.setInsecure(); 
  // For production: use proper certificate validation
  // net.setCACert(root_ca_pem);
}

void reconnectMQTT() {
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 3) {
    Serial.print("Connecting MQTT... ");
    
    // Generate unique client ID
    String clientId = String(device_id) + "-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      
      // Subscribe to command topic
      String cmdTopic = String("sensors/") + device_id + "/cmd";
      mqttClient.subscribe(cmdTopic.c_str(), 0);
      Serial.println("Subscribed to: " + cmdTopic);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
    attempts++;
  }
}

void updateLEDs(int co2_1, int co2_2) {
  // Get the highest CO2 reading for LED status
  int maxCO2 = max(co2_1, co2_2);
  
  // Turn off all LEDs first
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  
  if (maxCO2 < 0) {
    // Error state - blink red
    digitalWrite(redLED, millis() % 1000 < 500);
  } else if (maxCO2 < 800) {
    digitalWrite(greenLED, HIGH);  // Good air quality
  } else if (maxCO2 < 1200) {
    digitalWrite(yellowLED, HIGH); // Moderate air quality
  } else {
    digitalWrite(redLED, HIGH);    // Poor air quality
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("\nESP32 CO2 Monitor Starting...");

  // Initialize LCD (commented out - no LCD library)
   Wire.begin(21, 22); // SDA, SCL for ESP32
   lcd.init();            
   lcd.backlight();        
   lcd.setCursor(0, 0);    
   lcd.print("CO2 Monitor");

  Serial.println("CO2 Monitor Initialized");

  // Initialize sensors with specific pins for ESP32
  sensor1.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  sensor2.begin(9600, SERIAL_8N1, 4, 2);   // RX=4, TX=2

  // Initialize LEDs
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  
  // Test LEDs
  digitalWrite(greenLED, HIGH);
  delay(200);
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, HIGH);
  delay(200);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, HIGH);
  delay(200);
  digitalWrite(redLED, LOW);

  // WAIT for sensors to warm up
  Serial.println("Warming up sensors...");
   lcd.setCursor(0, 1);
   lcd.print("Warming up...");
  delay(10000); // wait 10s (you can extend to 20-30s)
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Ready!");
  Serial.println("Sensors ready!");
  delay(1000);

  // Connect to WiFi
  connectWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    // Setup time
    setupTime();
    
    // Setup MQTT
    setupMQTT();
    
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("WiFi Connected");
     lcd.setCursor(0, 1);
     lcd.print("MQTT Ready");
    Serial.println("WiFi Connected - MQTT Ready");
    delay(2000);
  } else {
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("WiFi Failed");
     lcd.setCursor(0, 1);
     lcd.print("Check Settings");
    Serial.println("WiFi Failed - Check Settings");
  }
}

void loop() {
  // Maintain connections
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  
  if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
    reconnectMQTT();
  }
  
  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  // Read sensors and publish data
  if (millis() - lastPublish >= publishIntervalMs) {
    lastPublish = millis();

    Serial.println("Reading CO2 sensors...");
    int co2_1 = readCO2(sensor1);
    int co2_2 = readCO2(sensor2);

    Serial.printf("Sensor 1: %d ppm, Sensor 2: %d ppm\n", co2_1, co2_2);

    // Update LCD display (commented out - no LCD library)
     lcd.clear();
     lcd.setCursor(0, 0);
     if (co2_1 > 0) {
       lcd.print("S1:");
       lcd.print(co2_1);
       lcd.print("ppm");
     } else {
       lcd.print("S1: Error");
     }
    
     lcd.setCursor(0, 1);
     if (co2_2 > 0) {
       lcd.print("S2:");
       lcd.print(co2_2);
       lcd.print("ppm");
     } else {
       lcd.print("S2: Error");
     }

    // Update LED indicators
    updateLEDs(co2_1, co2_2);

    // Publish to MQTT if connected
    if (mqttClient.connected()) {
      // Create JSON payload manually (no ArduinoJson library needed)
      String payload = "{";
      payload += "\"device\":\"" + String(device_id) + "\",";
      payload += "\"co2_1\":" + String(co2_1) + ",";
      payload += "\"co2_2\":" + String(co2_2) + ",";
      payload += "\"timestamp\":" + String(time(nullptr)) + ",";
      payload += "\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
      payload += "\"heap_free\":" + String(ESP.getFreeHeap());
      payload += "}";

      String topic = String("sensors/") + device_id + "/data";
      bool published = mqttClient.publish(topic.c_str(), payload.c_str(), false);
      
      Serial.print("Published to ");
      Serial.print(topic);
      Serial.print(" -> ");
      Serial.println(payload);
      
      if (!published) {
        Serial.println("MQTT publish failed!");
      }
    } else {
      Serial.println("MQTT not connected, skipping publish");
    }
  }
  
  // Small delay to prevent watchdog issues
  delay(100);
}
