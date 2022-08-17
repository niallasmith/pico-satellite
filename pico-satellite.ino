#include <WiFi.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>
#include <Pico_Unicorn_GFX.h>

char ssid[] = "YOUR_NETWORK_SSID_HERE";       // your network SSID (name)
char password[] = "YOUR_NETWORK_KEY_HERE";  // your network key
WiFiClientSecure client;
#define TEST_HOST "api.n2yo.com"
#define TEST_HOST_FINGERPRINT "42 7e 52 21 b0 ef 4e 3b c8 d0 59 2e c6 ba d6 63 7a 87 6c 46"
double satlatitude;
double satlongitude;

PicoUnicorn unicorn = PicoUnicorn();
// Pixel coordinates for each blue and green pixel, creating the image of the (mercator projected) earth
int earthBlue[64][2] = {
  {0, 0}, {1, 0}, {5, 0}, {7, 0}, {8, 0}, {10, 0}, {15, 0},
  {0, 1}, {6, 1}, {14, 1}, {15, 1},
  {0, 2}, {1, 2}, {4, 2}, {5, 2}, {6, 2}, {7, 2}, {14, 2}, {15, 2},
  {0, 3}, {1, 3}, {2, 3}, {3, 3}, {6, 3}, {10, 3}, {13, 3}, {14, 3}, {15, 3},
  {0, 4}, {1, 4}, {2, 4}, {3, 4}, {7, 4}, {10, 4}, {11, 4}, {12, 4}, {13, 4}, {15, 4},
  {0, 5}, {1, 5}, {2, 5}, {3, 5}, {6, 5}, {7, 5}, {8, 5}, {10, 5}, {11, 5}, {12, 5}, {15, 5},
  {0, 6}, {1, 6}, {2, 6}, {3, 6}, {5, 6}, {6, 6}, {7, 6}, {8, 6}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {15, 6}
};
int earthGreen[48][2] = {
  {2, 0}, {3, 0}, {4, 0}, {6, 0}, {9, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0},
  {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}, {12, 1}, {13, 1},
  {2, 2}, {3, 2},  {8, 2}, {9, 2}, {10, 2}, {11, 2}, {12, 2}, {13, 2},
  {4, 3}, {5, 3}, {7, 3}, {8, 3}, {9, 3}, {11, 3}, {12, 3},
  {4, 4}, {5, 4}, {6, 4}, {8, 4}, {9, 4}, {14, 4},
  {4, 5}, {5, 5}, {9, 5}, {13, 5}, {14, 5},
  {4, 6},
};
int earthBrightness = 64; // set brightness for earth image
int satBrightness = 96;

void setup() {
  // Initialise Pimoroni Unicorn and clear the display
  unicorn.init();
  unicorn.clear();

  displayEarth(); // Display image of the earth

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  client.setFingerprint(TEST_HOST_FINGERPRINT);

  // Retrieve data from n2yo api
  makeHTTPRequest();

  // Convert latitude & longitude into useful Unicorn display X & Y coordinates
  int displayLongitude = convertLongitude();
  int displayLatitude = convertLatitude();

  // Set red pixel at location of satellite
  unicorn.set_pixel(displayLongitude, displayLatitude, satBrightness, 0, 0);

  // Print display latitude & longitude XY coordinates
  Serial.print("X:");
  Serial.print(displayLongitude);
  Serial.print(" , Y: ");
  Serial.println(displayLatitude);
}


void loop() {
  // Hold for 60 seconds to slow api request rate
  int startTime = millis();
  while (millis() - startTime <= 60000) {
    delay(250);
  }

  // Make a HTTP request to retrieve data from n2yo api
  Serial.println("HTTP request");
  makeHTTPRequest();

  displayEarth(); // Display image of the earth

  // Convert latitude & longitude into useful Unicorn display X & Y coordinates
  int displayLongitude = convertLongitude();
  int displayLatitude = convertLatitude();

  // Set red pixel at location of satellite
  unicorn.set_pixel(displayLongitude, displayLatitude, satBrightness, 0, 0);

  // Print display latitude & longitude XY coordinates
  Serial.print("X:");
  Serial.print(displayLongitude);
  Serial.print(" , Y: ");
  Serial.println(displayLatitude);
}


void makeHTTPRequest() {

  client.setTimeout(10000); // set a long timeout in case the server is slow to respond or whatever

  // Opening connection to server
  if (!client.connect(TEST_HOST, 443))
  {
    Serial.println(F("Connection failed"));
    return;
  }

  yield();

  // Send HTTP request
  client.print(F("GET "));
  // This is the second half of a request (everything that comes after the base URL)
  client.print("/rest/v1/satellite/positions/NORAD_ID/50/-1/0/1/&apiKey=YOUR_API_KEY_HERE"); 
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(TEST_HOST);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, client);

  if (!error) {
    JsonObject info = doc["info"];
    const char* satname = info["satname"]; // "SPACE STATION"
    int satid = info["satid"]; // 25544
    int transactionscount = info["transactionscount"]; // 0

    JsonObject positions = doc["positions"][0];
    satlatitude = positions["satlatitude"]; // 4.34709191
    satlongitude = positions["satlongitude"]; // 46.86749667
    float sataltitude = positions["sataltitude"]; // 413.82
    float azimuth = positions["azimuth"]; // 121.74
    float elevation = positions["elevation"]; // -27.71
    double ra = positions["ra"]; // 250.07401841
    double dec = positions["dec"]; // -40.83069627
    long api_timestamp = positions["timestamp"]; // 1660656544
    bool eclipsed = positions["eclipsed"]; // false

    Serial.print("satlatitude:");
    Serial.println(satlatitude);
    Serial.print("satlongitude:");
    Serial.println(satlongitude);
  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
}


void displayEarth() {
  // Display image of the earth
  for (int i = 0; i <= 63; i++) {
    unicorn.set_pixel(earthBlue[i][0], earthBlue[i][1], 0, 0, earthBrightness);
  }
  for (int i = 0; i <= 47; i++) {
    unicorn.set_pixel(earthGreen[i][0], earthGreen[i][1], 0, earthBrightness, 0);
  }
}


int convertLongitude() {
  int convertLongitude = round(15 * ((satlongitude + 180) / 360));
  return convertLongitude;
}


int convertLatitude() {
  int convertLatitude = 6 - round(6 * ((satlatitude + 90) / 180));
  return convertLatitude;
}
