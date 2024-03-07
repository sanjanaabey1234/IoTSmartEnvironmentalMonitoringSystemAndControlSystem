#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <ESP32Servo.h>

#define TRIG_PIN  23  // ESP32 pin GPIO23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN  22  // ESP32 pin GPIO22 connected to Ultrasonic Sensor's ECHO pin
#define SERVO_PIN 26  // ESP32 pin GPIO26 connected to Servo Motor's pin
const int DISTANCE_THRESHOLD = 10; // centimeters

Servo myservo;

// Insert your network credentials
#define WIFI_SSID "RedmiNote11"
#define WIFI_PASSWORD "sanjana123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDJ8bocu6ib-xlKEpft7n5ENyigvkMz7w8"

// Insert RTDB URL
#define DATABASE_URL "https://ultrasonicsensorandservomotor-default-rtdb.firebaseio.com/" 

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  myservo.attach(SERVO_PIN);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase Authentication Successful");
    signupOK = true;
  }
  else{
    Serial.printf("Firebase Authentication Failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  long duration, distance;

  // Trigger ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pulse
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in centimeters
  distance = duration * 0.034 / 2;

  // Print distance to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Move servo based on distance and update to Firebase
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Update distance in Firebase
    if (Firebase.RTDB.setInt(&fbdo, "Distance", distance)) {
      Serial.println("Distance updated to Firebase");
    } else {
      Serial.println("Failed to update distance to Firebase");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Move servo based on distance
    if (distance > DISTANCE_THRESHOLD) {
      myservo.write(90); // Adjust the angle as needed
      delay(500);
    } else {
      myservo.write(0);  // Adjust the angle as needed
      delay(500);
    }
  }

  delay(1000);  // Delay between distance measurements
}
