#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// Define this if you want to scan for open WiFi networks to connect
#define FREE_WIFI

// Otherwise these settings will be used for WiFi connection
#define AP_SSID     "Your_SSID"
#define AP_PASSWORD "Your_Password"

/* Serial Baud Rate */
#define SERIAL_BAUD       115200
/* Delay paramter for connection. */
#define WIFI_DELAY        500
/* Max SSID octets. */
#define MAX_SSID_LEN      32
/* Wait this much until device gets IP. */
#define MAX_CONNECT_TIME  30000

/* SSID that to be stored to connect. */
char ssid[MAX_SSID_LEN] = "";

char host[] = "www.googleapis.com";
String google_apikey = _Your_Google_Maps_API_key_;

/* Scan available networks and request Google to determine your location. */
void scanAndLocate() {
int errorCode;
  
  memset(ssid, 0, MAX_SSID_LEN);
  int n = WiFi.scanNetworks();
  Serial.println("Scan done!");
  if (n == 0) {
    Serial.println("No networks found!");
  } else {

    DynamicJsonBuffer  jsonBuffer;
    byte mac[6];
    JsonObject& root = jsonBuffer.createObject();
    root["considerIp"] = "false";
    JsonArray& data = root.createNestedArray("wifiAccessPoints");
    for (int i = 0; i < n; i++) {
        JsonObject& wifiAP = jsonBuffer.createObject();
        wifiAP["macAddress"] = WiFi.BSSIDstr(i);
        wifiAP["signalStrength"] = WiFi.RSSI(i);
        wifiAP["signalToNoiseRatio"] = 0;
        data.add(wifiAP);
    }
    root.prettyPrintTo(Serial);
    Serial.println();
    
    String postdata;
    root.printTo(postdata);
    
    String resultdata = httpsPost("/geolocation/v1/geolocate?key="+google_apikey, "application/json", postdata, errorCode);

    Serial.println(errorCode);

    JsonObject& res_root = jsonBuffer.parseObject(resultdata);

    if (!root.success()) {
      Serial.println("JSON parsing failed!");
      return;
    }
    
    double lat = res_root["location"]["lat"];
    double lng = res_root["location"]["lng"];
    double acc = res_root["accuracy"];

    Serial.print(lat,5);
    Serial.print(",");
    Serial.println(lng,5);
  }
}

WiFiClientSecure client;

String httpsPost(String url, String contentType, String data, int &errorCode) {
  if (client.connect(host, 443)) {
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: " + (String)host);
    client.println("User-Agent: ESP8266/1.0");
    client.println("Connection: close");
    client.print("Content-Type: ");
    client.println(contentType);
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
    delay(50);
    String response = client.readString();

    errorCode = response.substring(response.indexOf(' ')+1).toInt();
    if (errorCode == 0) {
      errorCode = 444;
      return "No Response";
    }
    
    int bodypos =  response.indexOf("\r\n\r\n") + 4;
    return response.substring(bodypos);
  }
  else {
    errorCode = 600;
    return "Network Unreachable";
  }
}

/* Scan available networks and sort them in order to their signal strength. */
void scanAndSort() {
  memset(ssid, 0, MAX_SSID_LEN);
  int n = WiFi.scanNetworks();
  Serial.println("Scan done!");
  if (n == 0) {
    Serial.println("No networks found!");
  } else {
    Serial.print(n);
    Serial.println(" networks found.");
    int indices[n];
    for (int i = 0; i < n; i++) {
      indices[i] = i;
    }
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          std::swap(indices[i], indices[j]);
        }
      }
    }
    
    for (int i = 0; i < n; ++i) {
      Serial.print(WiFi.SSID(indices[i]));
      Serial.print(" ");
      Serial.print(WiFi.RSSI(indices[i]));
      Serial.print(" ");
      Serial.print(WiFi.encryptionType(indices[i]));
      Serial.println();
      if(WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE) {
        Serial.println("Found non-encrypted network. Store it and exit to connect.");
        memset(ssid, 0, MAX_SSID_LEN);
        strncpy(ssid, WiFi.SSID(indices[i]).c_str(), MAX_SSID_LEN);
        break;
      }
    }
  }
}

#ifndef FREE_WIFI 

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Started.");

  WiFi.begin(AP_SSID, AP_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  scanAndLocate();
}

void loop () {
}

#else

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Started.");
}

void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    /* Clear previous modes. */
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(WIFI_DELAY);
    /* Scan for networks to find open guy. */
    scanAndSort();
    delay(WIFI_DELAY);
    /* Global ssid param need to be filled to connect. */
    if(strlen(ssid) > 0) {
      Serial.print("Going to connect for : ");
      Serial.println(ssid);
      /* No pass for WiFi. We are looking for non-encrypteds. */
      WiFi.begin(ssid);
      unsigned short try_cnt = 0;
      /* Wait until WiFi connection but do not exceed MAX_CONNECT_TIME */
      while (WiFi.status() != WL_CONNECTED && try_cnt < MAX_CONNECT_TIME / WIFI_DELAY) {
        delay(WIFI_DELAY);
        Serial.print(".");
        try_cnt++;
      }
      if(WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        scanAndLocate();
      } else {
        Serial.println("Cannot established connection on given time.");
      }
    } else {
      Serial.println("No non-encrypted WiFi found.");  
    }
  }
}
#endif
