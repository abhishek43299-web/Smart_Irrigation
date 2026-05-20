//in the first 3 lines place your blynk template ID, template name and auth token 
#define BLYNK_TEMPLATE_ID "TMPL3RvgsZeG7"
#define BLYNK_TEMPLATE_NAME "smart irrigation demo"
#define BLYNK_AUTH_TOKEN "tl3EJ9dHY1BF9shuC44V3i-_ZQ3HZegB"

#include <DHTesp.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>
#include <BlynkSimpleEsp32.h>

// ---------------- WIFI ----------------
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ---------------- BLYNK ----------------
BlynkTimer timer;

// ---------------- DHT SENSOR ----------------
const int DHT_PIN = 12;
DHTesp dhtSensor;

// ---------------- SERVO ----------------
const int servoPin = 2;
Servo servo;

// ---------------- THINGSPEAK ----------------
unsigned long channelID = 3311271;
const char* writeAPIKey = "K1W8G83XQ7HUCC1Q";

WiFiClient client;

// ---------------- PIR SENSOR ----------------
const int pirPin = 27;
const int ledPin = 14;

// ---------------- LCD ----------------
const int rs = 5;
const int en = 17;
const int d4 = 16;
const int d5 = 4;
const int d6 = 0;
const int d7 = 15;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// ---------------- SOIL SENSOR ----------------
const int sensorPin = 34;

// ---------------- VARIABLES ----------------
int pirState = LOW;

void sendSensorData() {

  // -------- DHT SENSOR --------
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  float temperature = data.temperature;
  float humidity = data.humidity;

  // -------- SOIL MOISTURE --------
  int moistureValue = analogRead(sensorPin);

  int moisturePercent = map(moistureValue, 0, 4095, 40, 100);

  // -------- PIR SENSOR --------
  int val = digitalRead(pirPin);

  if (val == HIGH) {

    digitalWrite(ledPin, HIGH);

    if (pirState == LOW) {

      Serial.println("Motion Detected!");
      pirState = HIGH;
    }

  } else {

    digitalWrite(ledPin, LOW);

    if (pirState == HIGH) {

      Serial.println("Motion Ended!");
      pirState = LOW;
    }
  }

  // -------- PUMP CONTROL --------
  String pumpState;

  if (moisturePercent < 30) {

    servo.write(180);
    digitalWrite(ledPin, HIGH);
    pumpState = "ON";

  } else {

    servo.write(0);
    digitalWrite(ledPin, LOW);
    pumpState = "OFF";
  }
  if (temperature > 35) {
  Blynk.logEvent("temp_alert", "Temperature crossed 35C");
}
if (moisturePercent < 30) {

  Serial.println("Low Soil Moisture!");

  Blynk.logEvent("moisture_alert", "Soil moisture below 30%");
}
Serial.println("Moisture: " + String(moisturePercent) + " %");
if (humidity > 80) {

  Serial.println("High Humidity Alert!");

  Blynk.logEvent("high_humidity","Humidity crossed 80%");
}
if (humidity < 30) {

  Serial.println("Low Humidity Alert!");

  Blynk.logEvent("low_humidity","Humidity below 30%");
}
  // -------- SERIAL MONITOR --------
  Serial.println("========== SENSOR DATA ==========");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println(" %");

  Serial.print("Pump Status: ");
  Serial.println(pumpState);

  Serial.println("=================================");
  Serial.println();

  // -------- LCD DISPLAY --------
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C ");

  lcd.print("M:");
  lcd.print(moisturePercent);
  lcd.print("%");

  lcd.setCursor(0, 1);

  lcd.print("H:");
  lcd.print(humidity, 0);
  lcd.print("% ");

  lcd.print("P:");
  lcd.print(pumpState);

  // -------- BLYNK --------
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, moisturePercent);

  if (pumpState == "ON") {
    Blynk.virtualWrite(V3, 1);
  } else {
    Blynk.virtualWrite(V3, 0);
  }

  // -------- THINGSPEAK --------
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, moisturePercent);

  int response = ThingSpeak.writeFields(3311271, "K1W8G83XQ7HUCC1Q");

  if (response == 200) {

    Serial.println("ThingSpeak Updated Successfully");

  } else {

    Serial.print("ThingSpeak Error: ");
    Serial.println(response);
  }

  Serial.println("---------------------------------");
}

void setup() {

  Serial.begin(115200);

  // DHT
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Servo
  servo.attach(servoPin);

  // LCD
  lcd.begin(16, 2);

  // PIR
  pinMode(pirPin, INPUT);

  // LED
  pinMode(ledPin, OUTPUT);

  // WiFi
  WiFi.begin(ssid, password);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi Connected!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  // Blynk
  Blynk.begin("tl3EJ9dHY1BF9shuC44V3i-_ZQ3HZegB", ssid, password);
  if (Blynk.connected()) {

  Serial.println("Blynk Connected!");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Blynk Connected");

} else {

  Serial.println("Blynk NOT Connected!");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Blynk Failed");
}


  // ThingSpeak
  ThingSpeak.begin(client);

  // Timer
  timer.setInterval(20000L, sendSensorData);
}
void loop() {

  Blynk.run();

  timer.run();
}