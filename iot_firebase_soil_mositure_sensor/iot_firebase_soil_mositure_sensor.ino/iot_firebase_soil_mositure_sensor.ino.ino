#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
 
#define WIFI_SSID "RedmiNote11"
#define WIFI_PASSWORD "sanjana123"
#define API_KEY "AIzaSyAKKIjhA0vmQcp7n0b-PkCvoPyF5aDztMw"
#define DATABASE_URL "https://smartsoilmositure-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define SENSOR_PIN A0

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int _moisture, sensor_analog;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;                     //since we are doing an anonymous sign in 
 
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
}
 
void loop() {
  sensor_analog = analogRead(SENSOR_PIN);
  _moisture = (100 - ((sensor_analog / 4095.00) * 100));

  Serial.print("Moisture = ");
  Serial.print(_moisture);
  Serial.println("%");
 
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) { //
    sendDataPrevMillis = millis();
 
if(Firebase.RTDB.setFloat(&fbdo, "Sensor/Mositure", _moisture)) //sent to  Mositure data in firebase
{
  // Print the distance to Serial Monitor
  Serial.print("Mositure: ");
  Serial.print(_moisture);
  Serial.println(" Sucessfully saved to : " + fbdo.dataPath());
  Serial.println("("+ fbdo.dataType() +")");
  delay(10000);
 
}
else
{
  Serial.println(" Failed : " + fbdo.errorReason());
}
   
  }

}
 