#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

#define WIFI_SSID "Dialog 4G plus"
#define WIFI_PASSWORD "danidu@1324"
#define API_KEY "AIzaSyDBndDJrH91zSPtvyntWmJTde2HNZNP4YY" // Insert Firebase project API Key
#define DATABASE_URL "https://iotultrasonicservoanddht11soil-default-rtdb.asia-southeast1.firebasedatabase.app/" // Insert RTDB URL

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

Servo myservo;

const int TRIG_PIN_SERVO = 27;
const int ECHO_PIN_SERVO = 26;
const int SERVO_PIN = 14;

const int TRIG_PIN_BUZZER = 33;
const int ECHO_PIN_BUZZER = 25;
const int BUZZER_PIN = 32;

const int DISTANCE_THRESHOLD_SERVO = 10;
const int DISTANCE_THRESHOLD_BUZZER = 10;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("Connected with IP: " + WiFi.localIP().toString());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.print("SignUp Ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(TRIG_PIN_SERVO, OUTPUT);
  pinMode(ECHO_PIN_SERVO, INPUT);

  myservo.attach(SERVO_PIN);
  myservo.write(0);

  pinMode(TRIG_PIN_BUZZER, OUTPUT);
  pinMode(ECHO_PIN_BUZZER, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  // Measure distance for controlling the servo
  float distance_servo = measureDistance(TRIG_PIN_SERVO, ECHO_PIN_SERVO);

  // Measure distance for controlling the buzzer
  float distance_buzzer = measureDistance(TRIG_PIN_BUZZER, ECHO_PIN_BUZZER);

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Control servo based on distance
    if (distance_servo > DISTANCE_THRESHOLD_SERVO) {
      myservo.write(90);  // Rotate servo motor to 90 degrees
      updateDistance("Sensor/Distance", distance_servo);
      Firebase.RTDB.setString(&fbdo, "Message/alert", "Bridgedown");
      delay(500);
    } else {
      myservo.write(0);   // Rotate servo motor to 0 degrees
      updateDistance("Sensor/Distance", distance_servo);
      Firebase.RTDB.setString(&fbdo, "Message/alert", "Bridgeup");
      delay(500);
    }

    // Control buzzer based on distance
    if (distance_buzzer < DISTANCE_THRESHOLD_BUZZER) {  //10
      digitalWrite(BUZZER_PIN, HIGH);
      updateDistance("Elephantfence/Distance", distance_buzzer);
      Firebase.RTDB.setString(&fbdo, "ElephantMessage/alert", "ELEPHANTcome");
      delay(500);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      updateDistance("Elephantfence/Distance", distance_buzzer);
      Firebase.RTDB.setString(&fbdo, "ElephantMessage/alert", "SAFETYroad");
      delay(500);
    }

    // Update distances in the database
    //updateDistance("Sensor/Distance", distance_servo);
    //updateDistance("Elephantfence/Distance", distance_buzzer);
  }
}

float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration_us = pulseIn(echoPin, HIGH);
  return 0.017 * duration_us;
}

void updateDistance(const char* path, float distance) {
  if (Firebase.RTDB.setFloat(&fbdo, path, distance)) {
    Serial.print(path);
    Serial.print(" Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println(" Failed : " + fbdo.errorReason());
  }
}
