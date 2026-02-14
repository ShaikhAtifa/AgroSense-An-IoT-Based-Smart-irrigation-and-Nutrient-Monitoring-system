import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.multioutput import MultiOutputRegressor
from sklearn.metrics import mean_squared_error, r2_score
from xgboost import XGBRegressor
import joblib
import io
import requests
from google.colab import files, drive

uploaded = files.upload()
filename = next(iter(uploaded))
df = pd.read_csv(io.BytesIO(uploaded[filename]))

le_soil = LabelEncoder()
le_label = LabelEncoder()
df['soil'] = le_soil.fit_transform(df['soil'])
df['label'] = le_label.fit_transform(df['label'])

X = df.drop(['N', 'P', 'K'], axis=1)
y = df[['N', 'P', 'K']]

scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)

xgb = XGBRegressor(
    n_estimators=300,
    learning_rate=0.05,
    max_depth=6,
    subsample=0.8,
    colsample_bytree=0.8,
    random_state=42,
    reg_alpha=0.1, 
    reg_lambda=0.1  
)
multioutput_xgb = MultiOutputRegressor(xgb)
multioutput_xgb.fit(X_train, y_train)

y_pred = multioutput_xgb.predict(X_test)
rmse = np.sqrt(mean_squared_error(y_test, y_pred))
r2 = r2_score(y_test, y_pred)
print(f"RMSE: {rmse:.4f}")
print(f"R² Score: {r2:.4f}")

drive.mount('/content/drive')
joblib.dump(multioutput_xgb, '/content/drive/MyDrive/xgb_model.pkl')

model = joblib.load('/content/drive/MyDrive/xgb_model.pkl')

def read_sensor_data():
    channel_id = '2958025'
    read_api_key = 'I8R19U2FG4Q5O9TY'
    url = f"https://api.thingspeak.com/channels/{channel_id}/feeds.json"
    params = {'api_key': read_api_key, 'results': 5}  
    response = requests.get(url, params=params)

    if response.status_code != 200:
        print("Failed to read from ThingSpeak.")
        return None

    try:
        feeds = response.json()['feeds'][::-1]  
        for feed in feeds:
            if all(feed[f'field{i}'] is not None for i in range(1, 5)):
                sensor_data = {
                    'moisture': float(feed['field1']),
                    'pH': float(feed['field4']),
                    'temp': float(feed['field2']),
                    'humidity': float(feed['field3']),
                    'soil': 'loamy',  
                    'label': 'normal' 
                }
                return sensor_data
        print("No valid recent sensor data found.")
        return None
    except Exception as e:
        print("Error parsing ThingSpeak data:", e)
        return None

def safe_label_encode(encoder, value):
    try:
        return encoder.transform([value])[0]
    except ValueError:
        print(f"Warning: Unseen label '{value}' — using default 0.")
        return 0

def handle_outliers(data, min_val, max_val):
    return np.clip(data, min_val, max_val)

def predict_and_send():
    sensor = read_sensor_data()
    if not sensor:
        print("Sensor data not available.")
        return

    sensor['soil'] = safe_label_encode(le_soil, sensor['soil'])
    sensor['label'] = safe_label_encode(le_label, sensor['label'])

    input_data = np.array([[sensor['moisture'], sensor['pH'], sensor['temp'],
                            sensor['humidity'], sensor['soil'], sensor['label']]])

    input_scaled = scaler.transform(input_data)

    prediction = model.predict(input_scaled)

    n, p, k = handle_outliers(prediction[0][0], 0, 140), \
          handle_outliers(prediction[0][1], 0, 145), \
          handle_outliers(prediction[0][2], 0, 205)

    print(f"Predicted NPK: N={n}, P={p}, K={k}")

    url = "https://api.thingspeak.com/update"
    api_key = 'C1UC1TCPKCSQ4NTN'  # Write  Key
    payload = {
        'api_key': api_key,
        'field5': n,
        'field6': p,
        'field7': k
    }
    response = requests.post(url, params=payload)
    if response.status_code == 200:
        print("NPK data sent to ThingSpeak.")
    else:
        print(f"Error sending data: {response.status_code} - {response.text}")

predict_and_send()
