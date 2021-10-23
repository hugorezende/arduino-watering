/*
  Regador 2.0
*/

#define SAMPLES_SIZE 30
#include <Adafruit_BMP085.h>
#include <string.h>
#include <SoftwareSerial.h>

// Seconds between interval
int timeoutReader = 1;

// Threasholder limit
int thresholds[] = {600, 600};
int measuresS1[SAMPLES_SIZE];
int countThresholder = 1;

// Timers
unsigned long Time1;
int Period1;

// Sensor values
int sensor0 = A0;
int sensor1 = A1;
int sensor0Val = 0;
int sensor1Val = 0;

int valve0 = 6;

// Commands
String inputString = "";
String inputString2 = "";
bool stringComplete = false;
bool activeAutoWatering = true;

String command;
String command2;
String newTimeoutReader;

SoftwareSerial portSerial(8, 9);

Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(115200);
  portSerial.begin(115200);

  //bmp.begin();
  pinMode(sensor0, INPUT);
  pinMode(sensor1, INPUT);
  pinMode(valve0, OUTPUT);
  digitalWrite(valve0, LOW);

  Time1 =  millis();
  Period1 = 5000;
}

void loop() {
  if (millis() - Time1 >= Period1) {
    sensor0Val = analogRead(sensor0);
    sensor1Val = analogRead(sensor1);
    printSensorValues();
    pushVal(measuresS1, sensor0Val);
    checkWater();
    Time1 = millis();
  }

  if (portSerial.available()  > 0) {
    mySerialFunction();
  }
}

void readSensorValues() {
  sensor0Val = analogRead(sensor0);
  sensor1Val = analogRead(sensor1);
}

void printSensorValues() {
  portSerial.print("DATA,");
  portSerial.print(sensor0Val);
  portSerial.print(",");
  portSerial.print(sensor1Val);
  portSerial.print(",");
  portSerial.println(0);

  Serial.print("DATA,");
  Serial.print(sensor0Val);
  Serial.print(",");
  Serial.print(sensor1Val);
  Serial.print(",");
  Serial.println(0);
  delay(1000);
}

void checkWater() {
  int max_try = 0;
  digitalWrite(valve0, LOW);

  // CHECK SENSOR 01
  Serial.print("Water sensor 01 - ");
  Serial.println(verifyThreshold(measuresS1, thresholds[0], countThresholder));

  if (verifyThreshold(measuresS1, thresholds[0], countThresholder)) {
    Serial.println("PUMP 01 ON");
    sensor0Val = analogRead(sensor0);
    while (sensor0Val > thresholds[0]) {

      // If sensor read does not change after some time RETURN and inform error
      if (max_try > 200) {
        thresholds[0] = thresholds[0] + 1000;
        Serial.print("ERROR - value didn't change, reuturn threshold to  ");
        Serial.println(thresholds[0]);
        digitalWrite(valve0, LOW);
        return;
      }

      Serial.print("PUMP 01 ON");
      Serial.println(sensor0Val);
      readSensorValues();
      digitalWrite(valve0, HIGH);
      max_try ++;
      delay(100);
    }

    Serial.print("PUMP 01 OFF");
  }

}

void mySerialFunction() {
  //  command = "";
  inputString = "";
  // char *commands = NULL;

  while (portSerial.available()) {
    // get the new byte:
    char inChar = (char)portSerial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar != '\n') {
      inputString += inChar;
    }
    delay(10);
  }
  Serial.println(inputString);


}

void serialEvent() {
  command2 = "";
  inputString2 = "";
  char *commands = NULL;

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar != '\n') {
      inputString2 += inChar;
    }
    delay(10);
  }

  command2 = inputString2.substring(0, 3);
//  Serial.println("----");
//  Serial.println(inputString2);
//  Serial.println(command2);

  if (command2 == "T1:"  || command2 == "T2:" ) {
    Serial.print("Changing threshold ");
    Serial.print(inputString2.substring(1, 2));
    Serial.print(" to ");
    Serial.println(inputString2.substring(3, inputString2.length()));
    if (command2 == "T1:") thresholds[0] = inputString2.substring(3, inputString2.length()).toInt();
    if (command2 == "T2:") thresholds[1] = inputString2.substring(3, inputString2.length()).toInt();
  }

   if (command2 == "A1:"  || command2 == "A2:" ) {
    
    if (command2 == "A1:") {
      Serial.print("-> Threshold A1: ");
      Serial.print(thresholds[0]);
    }
    
  }

  //  if (command2 == "TI:") {
  //    newTimeoutReader = inputString2.substring(3, inputString.length());
  //    Serial.print("Changing reading time interval to ");
  //    Serial.print(newTimeoutReader);
  //    Serial.println(" seconds");
  //    timeoutReader = newTimeoutReader.toInt();
  //  } else {
  //    if (command2 == "T1:" || command2 == "T2:" ) {
  //      String th = "";
  //      th = inputString2.substring(3, inputString2.length());
  //      Serial.print("Changing threshold ");
  //      Serial.print(inputString2.substring(1, 2));
  //      Serial.print(" to ");
  //      Serial.println(th);
  //      if (command2 == "T1:") thresholds[0] = th.toInt();
  //      if (command2 == "T2:") thresholds[1] = th.toInt();
  //    } else {
  //      Serial.print("--- ");
  //      Serial.println(inputString2);
  //    }
  //  }

}


//String getValue(String data, char separator, int index)
//{
//  int found = 0;
//  int strIndex[] = {0, -1};
//  int maxIndex = data.length() - 1;
//
//  for (int i = 0; i <= maxIndex && found <= index; i++) {
//    if (data.charAt(i) == separator || i == maxIndex) {
//      found++;
//      strIndex[0] = strIndex[1] + 1;
//      strIndex[1] = (i == maxIndex) ? i + 1 : i;
//    }
//  }
//
//  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
//}

// Operator values
void pushVal(int a[], int value) {
  for (int i = SAMPLES_SIZE; i > 0; i--) {
    a[i] = a[i - 1];
  }
  a[0] = value;
}

void printVals(int a[]) {
  for (int i = 0; i <=  SAMPLES_SIZE; i++) {
    Serial.print(a[i]);
    Serial.print(",");
  }
  Serial.println();
}

int getMean(int a[]) {
  float mean = 0;
  for (int i = 0; i <=  SAMPLES_SIZE; i++) {
    mean += a[i];
  }
  return int(mean / SAMPLES_SIZE);
}

bool verifyThreshold(int a[], int threshold, int maxTimes) {
  int countTimes = 0;

  for (int i = 0; i <=  SAMPLES_SIZE; i++) {
    if (a[i] > threshold) {
      countTimes++;
    }
  }

  if (countTimes > maxTimes) {
    return true;
  } else {
    return false;
  }

}
