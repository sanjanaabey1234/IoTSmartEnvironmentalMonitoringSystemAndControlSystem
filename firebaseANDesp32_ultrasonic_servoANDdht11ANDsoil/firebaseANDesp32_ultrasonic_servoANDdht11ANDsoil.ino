#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <ESP32Servo.h>
#include <DHT.h>                // Install DHT library by adafruit 1.3.8



#define TRIG_PIN  23  // ESP32 pin GPIO23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN  22  // ESP32 pin GPIO22 connected to Ultrasonic Sensor's ECHO pin
#define SERVO_PIN 26  // ESP32 pin GPIO26 connected to Servo Motor's pin
const int DISTANCE_THRESHOLD = 6; // centimeters

Servo myservo;

#define SENSOR_PIN A0
int _moisture, sensor_analog;



#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11
//To provide the ESP32 / ESP8266 with the connection and the sensor type
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);




// Insert your network credentials
#define WIFI_SSID "RedmiNote11"
#define WIFI_PASSWORD "sanjana123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDBndDJrH91zSPtvyntWmJTde2HNZNP4YY"

// Insert RTDB URL
#define DATABASE_URL "https://iotultrasonicservoanddht11soil-default-rtdb.asia-southeast1.firebasedatabase.app/" 

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {

  dht_sensor.begin();
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


  
  sensor_analog = analogRead(SENSOR_PIN);
  _moisture = (100 - ((sensor_analog / 4095.00) * 100));

  Serial.print("Moisture = ");
  Serial.print(_moisture);
  Serial.println("%");



   
   //temperature and humidity measured should be stored in variables so the user
  //can use it later in the database

  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();

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




    
if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Mositure", _moisture)) //sent to  Mositure data in firebase
{
  // Print the distance to Serial Monitor
  Serial.print("Mositure: ");
  Serial.print(_moisture);
  Serial.println(" Sucessfully saved to : " + fbdo.dataPath());
  Serial.println("("+ fbdo.dataType() +")");
  delay(2500);
 
}

else
{
  Serial.println(" Failed : " + fbdo.errorReason());
}

   if (Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature", temperature)){
      // This command will be executed even if you dont serial print but we will make sure its working
      Serial.print("Temperature : ");
      Serial.println(temperature);
    }
    else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    
    // Enter Humidity in to the DHT_11 Table
    if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity", humidity)){
      Serial.print("Humidity : ");
      Serial.print(humidity);
    }
    else {
      Serial.println("Failed to Read from the Sensor");
      Serial.println("REASON: " + fbdo.errorReason());
    }



  }

  delay(1000);  // Delay between distance measurements
}
