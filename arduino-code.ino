
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

// DHT11 Configuration
#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// WiFi Configuration
const char* ssid = "POCO F3";
const char* password = "jgnahplz";

// Google Apps Script URL
const char* scriptURL = "https://script.google.com/macros/s/AKfycbzuIyNIqWXrhZsHAHMAch2ba8xs4zc-BtduHa2cyx3pNquhX-LVn0gp3iVHrAmGpnU3/exec";

void setup() {
  Serial.begin(115200);
  delay(2000);

  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.println(WiFi.localIP());
}

void loop() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read DHT11!");
    delay(5000);
    return;
  }

  Serial.println("----------------------");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  
WiFiClientSecure client;
client.setInsecure();

HTTPClient http;

http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
http.setTimeout(20000);

const char* headerKeys[] = {"Location"};
http.collectHeaders(headerKeys, 1);

String jsonData = "{";
jsonData += "\"temperature\":" + String(temperature, 2) + ",";
jsonData += "\"humidity\":" + String(humidity, 2);
jsonData += "}";

Serial.println("Sending data to Google Sheets...");

if (http.begin(client, scriptURL)) {

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonData);

  Serial.print("POST Response: ");
  Serial.println(httpCode);

  String redirectURL = http.header("Location");

  Serial.print("Redirect URL received: ");
  Serial.println(redirectURL.length() > 0 ? "YES" : "NO");

  http.end();

  if (httpCode == 302 && redirectURL.length() > 0) {

    Serial.println("Following Google redirect...");

    HTTPClient http2;
    http2.setTimeout(20000);

    if (http2.begin(client, redirectURL)) {

      int finalCode = http2.GET();

      Serial.print("Final Response: ");
      Serial.println(finalCode);

      Serial.println("Server Response:");
      Serial.println(http2.getString());

      http2.end();

    } else {
      Serial.println("Unable to connect to redirect URL!");
    }

  } else {
    Serial.println("No valid redirect received.");
  }

} else {
  Serial.println("Unable to connect to Google Script!");
}

  delay(5000);
}
