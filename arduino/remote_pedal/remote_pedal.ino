#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include "func_proto.h"
#include <SPI.h>
#include <WebSocketServer.h>
#include <WiFi101.h>
#include <Wire.h>

Adafruit_SSD1306 display = Adafruit_SSD1306();

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Enabe debug tracing to Serial port.  TODO: Remove it
#define DEBUG

// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64

// This is where the Potentiometer pin is hooked up.
#define POT_PIN 5

// Display Modes
#define DISP_IP_ADDR_MODE 0
#define DISP_POT_MODE 1

// Our Bounce Buttons and state
Bounce b11;
Bounce b12;
int b11fall = 0;
int b12fall = 0;

// Display Variables
int disp_mode = DISP_IP_ADDR_MODE;
int disp_timeout = 0;
int DISP_TIMEOUT_MILLIS = 5000;

// Last Potentiometer Reading
int last_pot = -1;

// WiFi Radio Status
int status = WL_IDLE_STATUS;

// These will be filled in by the automagic include
extern char ssid[];
extern char pass[];

// Create a Websocket server
WiFiServer server(80);
WebSocketServer webSocketServer;
// We only allow one client and that's OK
WiFiClient client;
IPAddress ipAddr;

/**
 * Process the pot
 */
int readPot() {
  return min(analogRead(POT_PIN) / 10, 99);
}

/**
 * Process the button presses.
 */
int durationSeconds(int finishMillis, int startMillis) {
  return (finishMillis - startMillis) / 1000;
}

Press processButtons() {
  bool b11updated = b11.update();
  bool b12updated = b12.update();
  Press p = { false, "", 0 };

  // Since we use internal pullup, fall is push, rise is release.
  if (b11updated || b12updated) {
    if (b11.fell()) {
      b11fall = millis();
    }
    if (b12.fell()) {
      b12fall = millis();
    }
    if (b11.rose()) {
      if (b11fall != 0) {
        if (b12fall != 0) {
          p = { true, "right", durationSeconds(millis(), b11fall) };
        } else {
          p = { true, "left", durationSeconds(millis(), b11fall) };
        }
      }
    } else if (b12.rose()) {
      if (b12fall != 0) {
        if (b11fall != 0) {
          p = { true, "right", durationSeconds(millis(), b12fall) };
        } else {
          p = { true, "middle", durationSeconds(millis(), b12fall) };
        }
      }
    }
    if (p.pressed) {
      b11fall = 0;
      b12fall = 0;
    }
  }
  return p;
}

/**
 * Main: Setup and loop
 */

// Setup and Loop
void setup() {
  Serial.begin(9600);

  // Set up the buttons
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  b11 = Bounce();
  b12 = Bounce();
  b11.attach(11);
  b12.attach(12);

  // Set up the OLED
  Serial.println("OLED FeatherWing init");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  Serial.println("Clearing OLED");

  // Clear the OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Connecting to network");
  display.print("SSID: ");
  display.println(ssid);
  display.display();

  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);

  Serial.println("Initializing WiFi");

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    // TODO: Poll the conenction rather than just wait?
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  // Start up the server
  server.begin();

  printCurrentNet();
  printWifiData();

  // Store off ip address... we don't need to read it each time.
  ipAddr = WiFi.localIP();

  // Display the potentiometer.  It will timeout automatically.
  displayRefresh(DISP_POT_MODE, readPot(), ipAddr, false);
}

void loop() {
  // Always process the button presses since it requires the loop to detect presses
  Press p = processButtons();
  if (p.pressed) {
    Serial.print("Button pressed: ");
    Serial.println(p.button);
  }

  // Always read the potentiometer
  int pot = readPot();

  bool clientConnected = false;
  // We can code sloppy cause we only want to handle 1 client at a time
  if (webSocketServer.clientConnected()) {
    clientConnected = true;

    // We don't do anything with the data at the moment.
    String data = webSocketServer.getData();
    if (data.length() > 0) {
       Serial.print("Got data: ");
       Serial.println(data);
    }

    // Send an event if buttons were pressed
    if (p.pressed) {
      // Build some JSON manually
      // Start (1) + Version (6) + button (<=13) + duration (9) + pot (8) + end (1) + NULL (1) => ~40
      char data[40] = "";
      char num[4];
      strcat(data, "{\"v\":0,\"b\":\"");
      strcat(data, p.button.c_str());
      strcat(data, "\",\"d\":");
      itoa(p.duration, num, 10);
      strcat(data, num);
      strcat(data, ",\"p\":");
      itoa(pot, num, 10);
      strcat(data, num);
      strcat(data, "}");

      // Send it over
      webSocketServer.sendData(data);
      Serial.print("Sent data: ");
      Serial.println(data);
    }
  } else {
    // Check for a new websocket client
    client = server.available();
    if (client.connected() && webSocketServer.handshake(client)) {
      Serial.println("Handshake Successful");
    }
  }

  // Display either IP addr or potentiometer
  // The "4" is here because reading the pot jitters.
  if (abs(last_pot - pot) > 4) { // Flip to display pot mode
    disp_timeout = millis() + DISP_TIMEOUT_MILLIS;
    disp_mode = DISP_POT_MODE;
    last_pot = pot;
  }
  // Flip back to disp ip addr mode
  if (disp_timeout != 0 &&
      (millis() > disp_timeout ||
       millis() < DISP_TIMEOUT_MILLIS) // Handle millis() overflow hack
     ) {
    disp_timeout = 0;
    disp_mode = DISP_IP_ADDR_MODE;
  }
  displayRefresh(disp_mode, pot, ipAddr, clientConnected);
}

void displayRefresh(int mode, int pot, IPAddress ip, bool clientConnected) {
  display.clearDisplay();
  display.setCursor(0,0);
  if (mode == DISP_POT_MODE) {
    display.setTextSize(2);
    display.print("Pos: ");
    display.println(pot);
  } else {
    display.setTextSize(1);
    display.print("SSID: ");
    display.println(WiFi.SSID());
    display.print("IP  : ");
    display.println(ip);
    display.println(clientConnected ? "Client Connected" : "No Client");
  }
  display.display();
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}
