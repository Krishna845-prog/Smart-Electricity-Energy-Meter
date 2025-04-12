#include <LiquidCrystal.h>
#include "EmonLib.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// LCD pin mapping: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);

// EmonLib instance
EnergyMonitor emon;

// Blynk Auth Token
char auth[] = "hsYG_5da4gdP9jZkL18O5RNcJSrBT-Ou";

// WiFi credentials
char ssid[] = "Alexahome";
char pass[] = "loranthus";

// Calibration values (adjust these based on your sensor)
#define vCalibration 83.3
#define currCalibration 0.50

// Energy tracking
float kWh = 0;
unsigned long lastmillis = millis();

// Blynk timer
BlynkTimer timer;

void myTimerEvent() {
  // Read voltage and current
  emon.calcVI(20, 2000);

  // Calculate energy in kWh
  unsigned long currentMillis = millis();
  float timeElapsed = (currentMillis - lastmillis) / 3600000.0; // Hours
  kWh += (emon.apparentPower * timeElapsed) / 1000.0; // Wh to kWh
  lastmillis = currentMillis;

  // Serial Monitor Output
  Serial.print("Vrms: "); Serial.print(emon.Vrms, 2); Serial.print(" V\t");
  Serial.print("Irms: "); Serial.print(emon.Irms, 4); Serial.print(" A\t");
  Serial.print("Power: "); Serial.print(emon.apparentPower, 4); Serial.print(" W\t");
  Serial.print("kWh: "); Serial.print(kWh, 5); Serial.println(" kWh");

  // LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vrms:"); lcd.print(emon.Vrms, 2); lcd.print("V");
  lcd.setCursor(0, 1);
  lcd.print("Irms:"); lcd.print(emon.Irms, 4); lcd.print("A");
  delay(2500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power:"); lcd.print(emon.apparentPower, 2); lcd.print("W");
  lcd.setCursor(0, 1);
  lcd.print("Energy:"); lcd.print(kWh, 3); lcd.print("kWh");
  delay(2500);

  // Send data to Blynk
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.apparentPower);
  Blynk.virtualWrite(V3, kWh);
}

void setup() {
  Serial.begin(9600);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setCursor(3, 0);
  lcd.print("IoT Energy");
  lcd.setCursor(5, 1);
  lcd.print("Meter");
  delay(3000);
  lcd.clear();

  // Start Blynk and WiFi
  Blynk.begin(auth, ssid, pass);

  // Initialize EmonLib with sensor pins and calibration
  emon.voltage(35, vCalibration, 1.7); // Voltage: pin, calibration, phase shift
  emon.current(34, currCalibration);  // Current: pin, calibration

  // Call the measurement function every 5 seconds
  timer.setInterval(5000L, myTimerEvent);
}

void loop() {
  Blynk.run();
  timer.run();
}