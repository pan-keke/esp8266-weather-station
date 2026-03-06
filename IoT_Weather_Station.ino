#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

#define DHTPIN 4        // D2 на NodeMCU
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

// 1. Функция, которая отдает ТОЛЬКО данные (без оформления)
void handleSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Создаем строку в формате JSON
  String json = "{";
  json += "\"temp\":\"" + String(t, 1) + "\",";
  json += "\"hum\":\"" + String(h, 0) + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// 2. Функция, которая отдает основную страницу
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html><html lang='ru'><head><meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<title>Live Climate</title>
<style>
  body { font-family: -apple-system, sans-serif; background-color: #f5f5f7; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
  .card { background: white; border-radius: 20px; padding: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.05); margin: 15px; width: 200px; text-align: center; }
  .label { font-size: 14px; color: #86868b; text-transform: uppercase; margin-bottom: 10px; letter-spacing: 1px; }
  .value { font-size: 48px; font-weight: 700; }
  .unit { font-size: 20px; color: #86868b; }
  .temp-color { color: #ff3b30; }
  .hum-color { color: #007aff; }
  .status { position: fixed; bottom: 20px; font-size: 12px; color: #ccc; }
</style>
</head>
<body>
  <div class='card'>
    <div class='label'>Температура</div>
    <div class='value temp-color'><span id='temp'>--</span><span class='unit'>°C</span></div>
  </div>
  <div class='card'>
    <div class='label'>Влажность</div>
    <div class='value hum-color'><span id='hum'>--</span><span class='unit'>%</span></div>
  </div>
  <div class='status'>Обновлено: <span id='time'>--</span></div>

<script>
  // Функция для запроса данных с сервера
  function updateData() {
    fetch('/readSensor')
      .then(response => response.json())
      .then(data => {
        document.getElementById('temp').innerText = data.temp;
        document.getElementById('hum').innerText = data.hum;
        document.getElementById('time').innerText = new Date().toLocaleTimeString();
      })
      .catch(err => console.error('Ошибка связи:', err));
  }

  // Запускать обновление каждые 2 секунды
  setInterval(updateData, 2000);
  // И один раз при загрузке
  updateData();
</script>
</body></html>
)=====";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);            // Главная страница
  server.on("/readSensor", handleSensorData); // Только данные
  server.begin();
}

void loop() {
  server.handleClient();
}