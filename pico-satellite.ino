#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Pico_Unicorn_GFX.h>

char ssid[] = "YOUR_NETWORK_SSID_HERE";       // your network SSID (name)
char password[] = "YOUR_NETWORK_KEY_HERE";  // your network key
#define apiKey "YOUR_API_KEY_HERE" // your api key
WiFiClientSecure client;
#define TEST_HOST "api.n2yo.com"
#define TEST_HOST_FINGERPRINT "42 7e 52 21 b0 ef 4e 3b c8 d0 59 2e c6 ba d6 63 7a 87 6c 46" // Go to: Chrome > Lock on search bar > "Connection is secure" > "Certificate is valid" > "Details" > "Show:" "Properties Only"
char satID0[] = "25544"; // SPACE STATION
char satID1[] = "33591"; // NOAA 19
char satID2[] = "38771"; // METOP-B
char satID3[] = "40069"; // METEOR M2
char *satIDArray[4] = {satID0, satID1, satID2, satID3}; // Array for easily iterating through satIDs when passing to HTTP get request
double satLatitude; // Global latitude variable used for returning HTTP get request data
double satLongitude; // Global longitude variable used for returning HTTP get request data
int satCoord[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}; // Array for X&Y coordinates
int satIDState; // Store state of tracked satellite ID to auto-update positions

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
int satBrightness = 164; // set brightness for satellite pixel

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

  updateSatData(); // HTTP request to initially set satellite data
  unicorn.set_pixel(satCoord[0][0], satCoord[0][1], satBrightness, 0, 0); // Set red pixel at location of satellite 0
}


void loop() {

  // Hold for 2 minutes to slow api request rate
  int startTime = millis();
  while (millis() - startTime <= 120000) {

    if (unicorn.is_pressed(unicorn.A)) {
      displayEarth(); // Display image of the earth
      unicorn.set_pixel(satCoord[0][0], satCoord[0][1], satBrightness, 0, 0); // Set red pixel at location of satellite 0
      satIDState = 0;

      // While button held, hold to stop display flashing
      while (unicorn.is_pressed(unicorn.A)) {
        delay(10);
      }

      return; // Deal with multiple button presses in order of precedence A > B > X > Y
    }

    if (unicorn.is_pressed(unicorn.B)) {
      displayEarth(); // Display image of the earth
      unicorn.set_pixel(satCoord[1][0], satCoord[1][1], satBrightness, 0, 0); // Set red pixel at location of satellite 1
      satIDState = 1;

      // While button held, hold to stop display flashing
      while (unicorn.is_pressed(unicorn.B)) {
        delay(10);
      }

      return; // Deal with multiple button presses in order of precedence A > B > X > Y
    }

    if (unicorn.is_pressed(unicorn.X)) {
      displayEarth(); // Display image of the earth
      unicorn.set_pixel(satCoord[2][0], satCoord[2][1], satBrightness, 0, 0); // Set red pixel at location of satellite 2
      satIDState = 2;

      // While button held, hold to stop display flashing
      while (unicorn.is_pressed(unicorn.X)) {
        delay(10);
      }

      return; // Deal with multiple button presses in order of precedence A > B > X > Y
    }

    if (unicorn.is_pressed(unicorn.Y)) {
      displayEarth(); // Display image of the earth
      unicorn.set_pixel(satCoord[3][0], satCoord[3][1], satBrightness, 0, 0); // Set red pixel at location of satellite 3
      satIDState = 3;

      // While button held, hold to stop display flashing
      while (unicorn.is_pressed(unicorn.Y)) {
        delay(10);
      }

      return; // Deal with multiple button presses in order of precedence A > B > X > Y
    }

  }

  updateSatData(); // After 2 min hold, update satellite location data

  displayEarth(); // Display image of the earth

  switch (satIDState) { // Check last known satellite tracking selection
    case 0: {
        unicorn.set_pixel(satCoord[0][0], satCoord[0][1], satBrightness, 0, 0); // Set red pixel at location of satellite 0
        break;
      }
    case 1: {
        unicorn.set_pixel(satCoord[1][0], satCoord[1][1], satBrightness, 0, 0); // Set red pixel at location of satellite 1
        break;
      }
    case 2: {
        unicorn.set_pixel(satCoord[2][0], satCoord[2][1], satBrightness, 0, 0); // Set red pixel at location of satellite 2
        break;
      }
    case 3: {
        unicorn.set_pixel(satCoord[3][0], satCoord[3][1], satBrightness, 0, 0); // Set red pixel at location of satellite 3
        break;
      }

  }

}


bool makeHTTPRequest(char *satID) {

  bool fail_flag = 0; // Flag for returning when there has been a network connection error of any type

  client.setTimeout(10000); // Set a long timeout in case the server is slow to respond or whatever

  // Opening connection to server
  if (!client.connect(TEST_HOST, 443))
  {
    Serial.println(F("Connection failed"));
    fail_flag = 1;
    return fail_flag;
  }

  yield();

  // Send HTTP request
  client.print(F("GET "));
  // This is the second half of a request (everything that comes after the base URL)
  client.print("/rest/v1/satellite/positions/");
  client.print(satID); // %2C == ,
  client.print("/50/-1/0/1/&apiKey=");
  client.print(apiKey);
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(TEST_HOST);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    fail_flag = 1;
    return fail_flag;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    fail_flag = 1;
    return fail_flag;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    fail_flag = 1;
    return fail_flag;
  }

  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, client);

  if (!error) {
    //    JsonObject info = doc["info"];
    //    const char* satname = info["satname"];
    //    int satid = info["satid"];
    //    int transactionscount = info["transactionscount"];

    JsonObject positions = doc["positions"][0];
    satLatitude = positions["satlatitude"];
    satLongitude = positions["satlongitude"];
    //    float sataltitude = positions["sataltitude"];
    //    float azimuth = positions["azimuth"];
    //    float elevation = positions["elevation"];
    //    double ra = positions["ra"];
    //    double dec = positions["dec"];
    //    long api_timestamp = positions["timestamp"];
    //    bool eclipsed = positions["eclipsed"];
    return fail_flag;

  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    fail_flag = 1;
    return fail_flag;
  }

}


void updateSatData() {
  // loop for all 4 satellite ID's set
  for (int i = 0; i <= 3; i++) {
    Serial.print("HTTP request ");
    Serial.print(i);
    Serial.print(" satID: ");
    Serial.print(satIDArray[i]);

    // Retrieve data from n2yo api
    bool flag = makeHTTPRequest(satIDArray[i]);

    // Convert latitude & longitude into useful Unicorn display X & Y coordinates
    if (flag == 0) { // If there hasn't been an connection error. Else: satCoords stay the same
      satCoord[i][0] = convertLongitude();
      satCoord[i][1] = convertLatitude();

      Serial.print(" DONE LAT:");
      Serial.print(satLatitude);
      Serial.print(" LONG:");
      Serial.print(satLongitude);
      Serial.print(" X: ");
      Serial.print(satCoord[i][1]);
      Serial.print(" Y: ");
      Serial.println(satCoord[i][0]);
    } else{
      Serial.println(" HTTP get request error");
    }
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
  int convertLongitude = round(15 * ((satLongitude + 180) / 360)); // Map longitude to X pixels 0 to 15
  convertLongitude = max(convertLongitude, 0); // Handle cases where X would provisionally be to the left of the screen
  convertLongitude = min(convertLongitude, 15); // Handle cases where X would provisionally be to the right of the screen
  return convertLongitude;
}


int convertLatitude() {
  int convertLatitude = 6 - round(6 * ((satLatitude + 90) / 180)); // Map latitude to Y pixels 0 to 6
  convertLatitude = max(convertLatitude, 0); // Handle cases where Y would provisionally be above the screen
  convertLatitude = min(convertLatitude, 6); // Handle cases where Y would provisionally be below the screen
  return convertLatitude;
}
