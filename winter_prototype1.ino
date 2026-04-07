#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

// ------------------ LCD ------------------
LiquidCrystal_I2C lcd_1(0x27, 16, 2);
bool lightMode = true;
static int lastDisplayed = -1;
unsigned long lastLcdUpdate = 0;
const unsigned long lcdInterval = 500; // ms

// ------------------ Sensors ------------------
#define photoPin A0
int photoResistor = 0;
unsigned long lastSensorMillis = 0;
const unsigned long sensorInterval = 100; // ms

// ------------------ Button ------------------
#define buttonPin 2
static unsigned long lastButtonPress = 0;

// ------------------ Timer ------------------
const int millisSeconds = 1000;
unsigned int previousMillis = 0;
unsigned int seconds = 0;

// ------------------ SD ------------------
const char* dataFileName = "data.txt";
const char* metaFileName = "meta.txt";
bool sdAvailable = false;

// ------------------ Logging ------------------
int lastSavedTime = 0;
const int n = 5;
const int totalIterations = 24 * n;
unsigned int lastLogMillis = 0;
const unsigned int logInterval = 10000; // ms

// ------------------ Utility Functions ------------------

// Read metadata safely from JSON file
int readMeta() {
  if (!SD.exists(metaFileName)) return 0;

  File metaFile = SD.open(metaFileName, FILE_READ);
  if (!metaFile) return 0;

  StaticJsonDocument<64> doc;
  DeserializationError err = deserializeJson(doc, metaFile);
  metaFile.close();

  if (err) {
    Serial.println("Meta file corrupted, resetting...");
    return 0;
  }

  int value = doc["lastSavedTime"] | 0;
  if (value < 0 || value > totalIterations) {
    Serial.println("Meta value invalid, resetting...");
    return 0;
  }

  return value;
}

// Write metadata safely to JSON file
void writeMeta(int value) {
  // Remove the old meta file first
  if (SD.exists(metaFileName)) SD.remove(metaFileName);

  // Open new meta file
  File f = SD.open(metaFileName, FILE_WRITE);
  if (!f) {
    Serial.println("Failed to write meta file");
    return;
  }

  StaticJsonDocument<64> doc;
  doc["lastSavedTime"] = value;
  serializeJson(doc, f);
  f.println();
  f.close();
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);

  // LCD
  lcd_1.init();
  lcd_1.setBacklight(1);
  lcd_1.setCursor(0,0);
  lcd_1.print("BlueLight (LUX):");

  pinMode(photoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // SD init with retries
  Serial.println("Initializing SD card...");
  for (int i = 0; i < 5; i++) {
    if (SD.begin(10)) {
      sdAvailable = true;
      break;
    }
    Serial.println("SD init failed, retrying...");
    delay(500);
  }
  if (!sdAvailable) Serial.println("SD NOT available, running without logging");
  else Serial.println("SD card initialized.");

  // Read metadata
  if (sdAvailable) lastSavedTime = readMeta();
  Serial.print("Recovered lastSavedTime: ");
  Serial.println(lastSavedTime);

  // Ensure data file exists
  if (sdAvailable && !SD.exists(dataFileName)) {
    File f = SD.open(dataFileName, FILE_WRITE);
    if (f) f.close();
  }

  // Stop if finished
  if (lastSavedTime >= totalIterations) {
    Serial.println("All readings already completed.");
  }
}

// ------------------ Main Loop ------------------
void loop() {
  unsigned long currentMillis = millis();

  // ---------- Read sensor ----------
  if (currentMillis - lastSensorMillis >= sensorInterval) {
    lastSensorMillis = currentMillis;

    static int smoothed = 0;
    smoothed = (smoothed * 3 + analogRead(photoPin)) / 4;
    photoResistor = smoothed;
  }

  int luxValue = map(photoResistor, 0, 170, 0, 20000);

  // ---------- Update LCD ----------
  if (currentMillis - lastLcdUpdate >= lcdInterval) {
    lastLcdUpdate = currentMillis;
    if (abs(luxValue - lastDisplayed) > 50) {
      lcd_1.setCursor(0,1);
      lcd_1.print("      "); // clear
      lcd_1.setCursor(0,1);
      lcd_1.print(luxValue);
      lastDisplayed = luxValue;
    }
  }

  // ---------- Button ----------
  if (digitalRead(buttonPin) == LOW && currentMillis - lastButtonPress > 300) {
    lastButtonPress = currentMillis;
    lightMode = !lightMode;
    if (lightMode) {
      lcd_1.display();
      lcd_1.setBacklight(1);
    } else {
      lcd_1.noDisplay();
      lcd_1.setBacklight(0);
    }
  }

  // ---------- Seconds timer ----------
  if (currentMillis - previousMillis >= millisSeconds) {
    previousMillis = currentMillis;
    seconds++;
  }

  // ---------- Logging ----------
  if (sdAvailable && currentMillis - lastLogMillis >= logInterval) {
    lastLogMillis = currentMillis;

    if (lastSavedTime >= totalIterations) return;

    lastSavedTime++;

    File myFile = SD.open(dataFileName, FILE_WRITE);
    if (myFile) {
      StaticJsonDocument<128> doc;
      doc["Time"] = lastSavedTime;
      doc["raw"] = photoResistor;
      doc["lux"] = luxValue;
      serializeJson(doc, myFile);
      myFile.println();
      myFile.close();

      // Save meta every 5 entries to reduce SD wear
      if (lastSavedTime % 5 == 0) writeMeta(lastSavedTime);

      Serial.print("Logged: raw=");
      Serial.print(photoResistor);
      Serial.print(", lux=");
      Serial.println(luxValue);
    }
  }
}