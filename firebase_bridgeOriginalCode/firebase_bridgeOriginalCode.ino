#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h> // Library for Servo Motor
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Dialog 4G plus"
#define WIFI_PASSWORD "danidu@1324"
#define API_KEY "AIzaSyDBndDJrH91zSPtvyntWmJTde2HNZNP4YY" // Insert Firebase project API Key
#define DATABASE_URL "https://iotultrasonicservoanddht11soil-default-rtdb.asia-southeast1.firebasedatabase.app/" // Insert RTDB URL

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Servo object
Servo myservo;


// Constants
const int TRIG_PIN = 27;         // GPIO pin connected to Ultrasonic Sensor's TRIG pin on ESP32
const int ECHO_PIN = 26;         // GPIO pin connected to Ultrasonic Sensor's ECHO pin on ESP32
const int SERVO_PIN = 14;        // GPIO pin connected to Servo Motor's pin on ESP32
const int DISTANCE_THRESHOLD = 10; // centimeters

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);           // Initialize serial port
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

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  
  pinMode(TRIG_PIN, OUTPUT);    // Set ESP32 pin to output mode for TRIG
  pinMode(ECHO_PIN, INPUT);     // Set ESP32 pin to input mode for ECHO

  myservo.attach(SERVO_PIN);    // Attach the servo to the servo object
  myservo.write(0);             // Set initial servo position to 0 degrees
}

void loop() {
  // Generate a 10-microsecond pulse to the TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the duration of the pulse from the ECHO pin
  long duration_us = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in centimeters (Speed of sound: 343 m/s or 0.0343 cm/us)
  float distance_cm = duration_us * 0.01715;  // 1/2 * 0.0343

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) { //
    sendDataPrevMillis = millis();
    
   // Control servo based on distance
  if (distance_cm > DISTANCE_THRESHOLD) {
    myservo.write(90);  // Rotate servo motor to 90 degrees
    String  alert="BridgeOpen"; 
    Firebase.RTDB.setString(&fbdo, "Message/alert", alert);
    delay(1000);
     
  } else {
    myservo.write(0);   // Rotate servo motor to 0 degrees
     String  alert="Safety"; 
    Firebase.RTDB.setString(&fbdo, "Message/alert", alert);
     delay(1000);
  }

if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Distance", distance_cm)) //sent to datanase distance
{
  // Print the distance to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");
 

  
}
else
{
  Serial.println(" Failed : " + fbdo.errorReason());
}
   
  }
}
