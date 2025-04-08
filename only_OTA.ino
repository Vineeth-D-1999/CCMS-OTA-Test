#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <base64.h>

// WiFi credentials
const char* ssid = "Technotouch";
const char* password = "5135Innova";

/*Version URL*/
//const char* version_url = "https://raw.githubusercontent.com/NandeepYadav/Test_OTA/main/version";
const char* version_url = "https://raw.githubusercontent.com/Vineeth-D-1999/CCMS-OTA-Test/master/version.txt";
/*Firmware URL*/
//const char* firmware_url = "https://raw.githubusercontent.com/NandeepYadav/Test_OTA/main/ccms.ino.bin";
const char* firmware_url = "https://raw.githubusercontent.com/Vineeth-D-1999/CCMS-OTA-Test/master/only_OTA.ino.bin";

// Current firmware version
const String currentVersion = "1.0";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  };
  if (checkForUpdate()) {
    performOTA();
  } else {
    Serial.println("Already running the latest firmware.");
  }
}

void loop() {
  // Put your main application code here
  Serial.println("Version " + String(currentVersion));
  delay(5000);
}

// Check for update by comparing versions
bool checkForUpdate() {
  HTTPClient http;
  http.begin(version_url);
  // http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);  // Enable following redirects
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND) {
    String newLocation = http.header("Location");
    Serial.println("Redirected to: " + newLocation);
    http.end();
    http.begin(newLocation);  // Follow the new URL
    httpCode = http.GET();
  }
  Serial.println("httpCode: " + String(httpCode));
  if (httpCode == HTTP_CODE_OK) {
    String latestVersion = http.getString();
    latestVersion.trim();  // Remove whitespace or newline characters
    Serial.println("Latest Version: " + latestVersion);
    Serial.println("Current Version: " + currentVersion);
    return (latestVersion != currentVersion);
  } else {
    Serial.println("Failed to check for updates. HTTP Code: " + String(httpCode));
    return false;
  }
}

// Perform OTA update
void performOTA() {
  HTTPClient http;
  http.begin(firmware_url);
  // http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);  // Enable following redirects
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND) {
    String newLocation = http.header("Location");
    Serial.println("Redirected to: " + newLocation);
    http.end();
    http.begin(newLocation);  // Follow the new URL
    httpCode = http.GET();
  }
  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      Serial.println("Starting OTA...");
      WiFiClient& client = http.getStream();
      size_t written = Update.writeStream(client);

      if (written == contentLength && Update.end()) {
        Serial.println("OTA Update Success. Restarting...");
        ESP.restart();
      } else {
        Serial.println("OTA Update Failed. Error #: " + String(Update.getError()));
      }
    } else {
      Serial.println("Not enough space for OTA update.");
    }
  } else {
    Serial.println("Failed to fetch firmware. HTTP Code: " + String(httpCode));
  }

  http.end();
}
