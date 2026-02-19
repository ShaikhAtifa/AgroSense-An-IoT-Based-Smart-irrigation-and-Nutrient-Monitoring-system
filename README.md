# AgroSense-An-IoT-Based-Smart-irrigation-and-Nutrient-Monitoring-system
An IoT based smart irrigation system using real-time sensor data and open weather API , Nutrient monitoring i,e NPK prediction using Machine Learning Model(XGBoost). 
AgroSense
IoT-Based Smart Irrigation & ML-Driven NPK Prediction System

AgroSense is an integrated IoT + Machine Learning precision agriculture system that carries out real-time soil monitoring, weather-controlled smart irrigation, and multi-output nutrient prediction (NPK) with XGBoost.

 Key Features
Real-time soil moisture, temperature & humidity monitoring
Rain detection + 24-hour weather forecast integration
Intelligent pump automation
Multi-output NPK prediction using XGBoost
ThingSpeak cloud integration
OLED live monitoring display
Outlier-controlled nutrient prediction

 System Architecture
Sensors → ESP8266 → ThingSpeak → ML Model (XGBoost) → NPK Prediction
        ↓
   Weather API → Smart Irrigation Logic → Relay Pump

 Machine Learning Model
Algorithm: XGBoost Regressor (Multi-Output)
Inputs:
Soil moisture
pH
Temperature
Humidity
Soil type
Crop label

Outputs:
Nitrogen (N)
Phosphorus (P)
Potassium (K)

Model Characteristics:
300 estimators
Learning rate: 0.05
Regularization applied
Feature scaling using StandardScaler
Outlier clipping for realistic nutrient ranges

 Hardware Components
Component	Purpose
ESP8266	WiFi-enabled microcontroller
Soil Moisture Sensor	Soil condition detection
DHT11	Temperature & Humidity
Rain Sensor	Rain detection
Relay Module	Pump control
OLED SSD1306	Live monitoring

🌦 Weather Integration
Uses OpenWeather API
Fetches:
Current weather
24-hour forecast
If rain is predicted, → Irrigation is disabled

 Cloud Integration
ThingSpeak:
Uploads sensor data
Stores predicted NPK values
Enables remote monitoring
