/*
  Smart Accident Detection & Alert System
  ------------------------------------------------
  Reference/reconstructed implementation for the
  project portfolio. Pin assignments were recreated
  because the original sketch was not available.

  Behavior:
  Accident detected -> Buzzer ON
                   -> LCD: "ACCIDENT DETECTED"
                   -> SIM900L sends an SMS

  Hardware:
  Arduino Uno, MPU6050, 16x2 I2C LCD, SIM900L,
  MQ135, Buzzer

  IMPORTANT:
  Verify wiring, GSM power supply, and sensor thresholds
  before uploading to physical hardware.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <MPU6050.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
MPU6050 mpu;

// Arduino RX, TX for SoftwareSerial.
// Arduino D7 <- SIM900L TX
// Arduino D8 -> SIM900L RX (use appropriate level shifting)
SoftwareSerial sim900(7, 8);

const int BUZZER_PIN = 12;
const int MQ135_PIN  = A0;

const int GAS_THRESHOLD = 400;
const float TILT_THRESHOLD = 45.0;

// Change this to the emergency phone number before real use.
const char EMERGENCY_NUMBER[] = "+91XXXXXXXXXX";

bool accidentAlertSent = false;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(9600);
  sim900.begin(9600);

  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Accident");
  lcd.setCursor(0, 1);
  lcd.print("Detection System");

  mpu.initialize();

  delay(2000);
  lcd.clear();
  lcd.print("System Ready");
  delay(1000);
}

void sendSMS() {
  sim900.println("AT");
  delay(1000);

  sim900.println("AT+CMGF=1");   // SMS text mode
  delay(1000);

  sim900.print("AT+CMGS=\"");
  sim900.print(EMERGENCY_NUMBER);
  sim900.println("\"");
  delay(1000);

  sim900.print("Emergency Alert: Accident detected.");
  sim900.write(26);              // CTRL+Z
  delay(5000);
}

void showAccidentAlert() {
  digitalWrite(BUZZER_PIN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCIDENT");
  lcd.setCursor(0, 1);
  lcd.print("DETECTED");

  if (!accidentAlertSent) {
    sendSMS();
    accidentAlertSent = true;
  }
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Calculate tilt angle from accelerometer readings.
  float angleX = atan2((float)ay, (float)az) * 180.0 / PI;
  float angleY = atan2((float)ax, (float)az) * 180.0 / PI;

  int gasValue = analogRead(MQ135_PIN);

  bool accidentDetected =
      (abs(angleX) >= TILT_THRESHOLD) ||
      (abs(angleY) >= TILT_THRESHOLD);

  if (accidentDetected) {
    showAccidentAlert();
  } else {
    digitalWrite(BUZZER_PIN, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Normal");
    lcd.setCursor(0, 1);
    lcd.print("Gas:");
    lcd.print(gasValue);

    // Allows a new alert after the vehicle returns to normal.
    accidentAlertSent = false;
  }

  delay(500);
}
