#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define INFLUXDB_URL ""
#define INFLUXDB_TOKEN ""
#define INFLUXDB_ORG ""
#define INFLUXDB_BUCKET ""

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("Plantas");

String inputString = "";      // a String to hold incoming data
String command = "";
String c1, c2, c3;
String valueRead = "";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  client.setWriteOptions(WriteOptions().batchSize(2));
  
  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  delay(500);
}

void serialEvent() {
  command, c1, c2, c3 = "";
  inputString = "";
  
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar != '\n') {
      inputString += inChar;
    }
    delay(10);
  }

  sensor.clearFields();
  command = getValue(inputString, ',', 0);
  c1 = getValue(inputString, ',', 1);
  c2 = getValue(inputString, ',', 2);
  c3 = getValue(inputString, ',', 3);

  if(command == "DATA"){
    
    long value = long(c1.toInt());
    sensor.addField("Sensor1", value);

    value = long(c2.toInt());
    sensor.addField("Sensor2", value);
    
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());
    
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Wifi connection lost");
    }
    
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
  
  delay(500);
}

String getValue(String data, char separator, int index) {

  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
