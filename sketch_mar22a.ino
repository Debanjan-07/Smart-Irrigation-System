#define BLYNK_TEMPLATE_ID "TMPL3DKx54XuH"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation System EcoIrrigate"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi & Blynk credentials
char auth[] = "pKfrnCSM4sqJZdyqoet8HECDtryEzAYD";
char ssid[] = "vivo T3 5G";
char pass[] = "00000000";

// Sensor Pins
#define DHTPIN 26       
#define DHTTYPE DHT11   
#define SOIL_MOISTURE_PIN 33   
#define RELAY_PIN 23        

// Virtual Pins
#define SOIL_VPIN V2    
#define TEMP_VPIN V1    
#define HUM_VPIN V3     
#define PUMP_VPIN V0    

DHT dht(DHTPIN, DHTTYPE);

// Global variable to track manual pump control
int manualPumpState = 0;
bool pumpStatusNotified = false;  // Track if notification is already sent

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Initialize Blynk and sensors
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  
  // Initialize relay as output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Turn off the pump by default (LOW=ON, HIGH=OFF)
}

// Function to send soil moisture warning
void sendSoilMoistureNotification() {
  Blynk.logEvent("ecoirrigate"," WARNING!☠ Soil Moisture is too Low.The pump is now ON."); // Send notification to Blynk
  
}

void loop() {
  Blynk.run();
  
  // Read data from sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  
  // Check if the sensor values are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Convert soil moisture value to percentage
  int soilMoisturePercent = map(soilMoistureValue, 0, 4095, 0, 100); // Adjust range for ESP32 ADC resolution
  soilMoisturePercent = (soilMoisturePercent - 100) * -1;

  // Display sensor data on serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");

  // Send sensor data to Blynk app
  Blynk.virtualWrite(TEMP_VPIN, temperature);  // Temperature to Blynk V0
  Blynk.virtualWrite(HUM_VPIN, humidity);      // Humidity to Blynk V1
  Blynk.virtualWrite(SOIL_VPIN, soilMoisturePercent);  // Soil Moisture to Blynk V2

  // Check if pump should be on based on soil moisture or manual switch
  if (soilMoisturePercent < 20) {
    // Automatically turn on the pump
    digitalWrite(RELAY_PIN, HIGH);  // Turn on pump
    Serial.println("Water Pump ON (Automatic due to low soil moisture)");

    // Send notification to Blynk if not already sent
    if (!pumpStatusNotified) {
      sendSoilMoistureNotification();  // Send soil moisture notification
      pumpStatusNotified = true;       // Mark that the notification has been sent
    }

  } else if (manualPumpState == 1) {
    // Manually turn on the pump via Blynk switch
    digitalWrite(RELAY_PIN, HIGH);  // Turn on pump
    Serial.println("Water Pump ON (Manual Control)");

  } else {
    // Turn off the pump if no conditions are met
    digitalWrite(RELAY_PIN, LOW); // Turn off pump
    Serial.println("Water Pump OFF");
    pumpStatusNotified = false;    // Reset notification flag when pump is off
    
  }

  delay(5000);
}

// Blynk function to control water pump manually through the app (V12)
BLYNK_WRITE(PUMP_VPIN) {
  manualPumpState = param.asInt(); // Get the state of the pump from the app (V12)
  Serial.print("Manual Pump Control: ");
  Serial.println(manualPumpState == 1 ? "ON":"OFF");

}
