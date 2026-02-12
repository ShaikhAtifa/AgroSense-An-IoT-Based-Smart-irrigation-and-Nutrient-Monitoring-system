//  open weather api
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool willRainSoon = false; 
String reason = "Initializing...";

#define SOIL_MOISTURE_PIN A0
#define RELAY_PIN D5
#define RAIN_SENSOR_PIN D6
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float pHLevel = 7.0;
float npkLevel = 4.0;

const char* ssid = "";
const char* password = "";
const String apiKey = "82d38460bba08ea5c65466c24b1d0d54";
const String city = "Pune";
const String country = "IN";

WiFiClient client;
void fetchCurrentWeather() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + country + "&appid=" + apiKey + "&units=metric";
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"];
    int humidity = doc["main"]["humidity"];
    String condition = doc["weather"][0]["description"];
    String cityName = doc["name"];
    display.clearDisplay();
    display.setTextSize(1);  
    display.setTextColor(SSD1306_WHITE);  
    display.setCursor(0, 0);
    display.print("City: " + cityName);
    display.setCursor(0, 10);
    display.print("Temp: " + String(temp) + "C");
    display.setCursor(0, 20);
    display.print("Humidity: " + String(humidity) + "%");
    display.setCursor(0, 30);
    display.print("Condition: " + condition);
    display.display();  
  } else {
    Serial.println("Failed to fetch current weather data.");
  }

  http.end();
}
void fetch24HourForecast() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/forecast?q=" + city + "," + country + "&appid=" + apiKey + "&units=metric";
  http.begin(client, url);
  int httpCode = http.GET();

  willRainSoon = false;
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, payload);

    JsonArray list = doc["list"];
    
    for (int i = 0; i < 8; i++) {  
      String condition = list[i]["weather"][0]["main"];  
      if (condition == "Rain") {
        willRainSoon = true;
        break;
      }
    }

  } else {
    Serial.println("Failed to fetch forecast data.");
  }

  http.end();
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  fetchCurrentWeather();

  fetch24HourForecast();

  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  digitalWrite(RELAY_PIN, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  dht.begin();
  display.setTextColor(SSD1306_WHITE);
}

String getMoistureLevel(int value) {
  if (value < 300) return "Wet";
  else if (value < 700) return "Moist";
  else return "Dry";
}

void loop() {
  int moistureValue = analogRead(SOIL_MOISTURE_PIN);
  int rainState = digitalRead(RAIN_SENSOR_PIN);
  String moistureLevel = getMoistureLevel(moistureValue);
  String pumpStatus;

if (rainState == LOW) {
    digitalWrite(RELAY_PIN, HIGH);
    pumpStatus = "OFF (Rain)";
    reason = "Rain Detected";
  } else if (willRainSoon) {
    digitalWrite(RELAY_PIN, HIGH);
    pumpStatus = "OFF (Rain)";
    reason = "Forecast: Rain";
  } 
  else 
  {
    if (moistureLevel == "Dry") 
    {
      digitalWrite(RELAY_PIN, LOW);
      pumpStatus = "ON";
      reason = "Soil dry";
    } 
    else 
    {
      digitalWrite(RELAY_PIN, HIGH);
      pumpStatus = "OFF";
      reason = "Soil Mois/Wet";
    }
}

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Soil Moisture: ");
  display.print(moistureValue);
  display.print(" (");
  display.print(moistureLevel);
  display.print(")");
  display.display();
  delay(2000); yield();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.print(temperature);
  display.print(" C");
  display.display();
  delay(2000); yield();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Humidity: ");
  display.print(humidity);
  display.print(" %");
  display.display();
  delay(2000); yield();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("pH Level: ");
  display.print(pHLevel);
  display.display();
  delay(2000); yield();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("NPK Level: ");
  display.print(npkLevel);
  display.display();
  delay(2000); yield();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Pump: ");
  display.print(pumpStatus);
  display.display();
  delay(2000); yield();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Rain: ");
  display.print(rainState == LOW ? "Detected" : "No Rain");
  display.display();
  delay(2000); yield();
}
