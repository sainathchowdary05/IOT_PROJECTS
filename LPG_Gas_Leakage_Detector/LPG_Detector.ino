#include <Servo.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C LCD

#define MQ2_PIN A0
#define RELAY_PIN 2
#define BUZZER_PIN 7 
#define SERVO_PIN 9
#define GSM_RX 10
#define GSM_TX 11
#define STATUS_LED 12
#define GAS_THRESHOLD 200

Servo gasValve;
SoftwareSerial gsm(GSM_RX, GSM_TX);

bool alertSent = false;
bool servoActivated = false;

void setup() {
  Serial.begin(9600);
  gsm.begin(9600);
  delay(1000);

  pinMode(MQ2_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(STATUS_LED, HIGH);  // Safe

  gasValve.attach(SERVO_PIN);
  gasValve.write(0);  // Valve open

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LPG Detector Ready");

  Serial.println("LPG Gas Detector Ready");
}

void loop() {
  int gasLevel = analogRead(MQ2_PIN);
  Serial.print("Gas Level: ");
  Serial.println(gasLevel);

  // Display gas level
  lcd.setCursor(0, 1);
  lcd.print("Gas: ");
  lcd.print(gasLevel);
  lcd.print("     "); // Clear trailing digits

  if (gasLevel > GAS_THRESHOLD) {
    // Danger condition
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(STATUS_LED, LOW);

    lcd.setCursor(0, 0);
    lcd.print("!! GAS DETECTED !!");

    if (!servoActivated) {
      gasValve.write(180);   // Close valve once
      servoActivated = true;
    }

    if (!alertSent) {
      sendSMS("⚠️ LPG Gas Leak Detected! Take action immediately.");
      alertSent = true;
    }

  } else {
    // Safe condition
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(STATUS_LED, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("Status: Safe     ");

    if (servoActivated) {
      gasValve.write(0);   // Open valve again
      servoActivated = false;
    }

    alertSent = false;
  }

  delay(1000);
}

void sendSMS(String message) {
  gsm.println("AT+CMGF=1");  // Text mode
  delay(200);
  gsm.println("AT+CMGS=\"+1234567890\"");  // Replace with your number
  delay(300);
  gsm.print(message);
  delay(100);
  gsm.write(26);  // Ctrl+Z
  delay(2000);
}
