#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi101.h>
#include <WebSocketServer.h>

Adafruit_SSD1306 display = Adafruit_SSD1306();

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Enabe debug tracing to Serial port.  TODO: Remove it
#define DEBUG

// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64

// WiFi Radio Status
int status = WL_IDLE_STATUS;

// These will be filled in by the automagic include
extern char ssid[];
extern char pass[];

// Our Server
// Create a Websocket server
WiFiServer server(80);
// We only allow one client
WiFiClient client;
WebSocketServer webSocketServer;
IPAddress ipAddr;

/**
 * Main: Setup and loop
 */

// Setup and Loop
void setup() {
  Serial.begin(9600);

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

  // Display update
  displayRefresh(ipAddr, false);
}

void loop() {

  bool clientConnected = false;
  // We can code sloppy cause we only want to handle 1 client at a time
  if (webSocketServer.clientConnected()) {
    String data = webSocketServer.getData();
    if (data.length() > 0) {
       Serial.print("Got data: ");
       Serial.println(data);
    }
    data = "hello world!";
    webSocketServer.sendData(data);
    Serial.print("Sent data: ");
    Serial.println(data);
    clientConnected = true;
  } else {
    client = server.available();
    if (client.connected() && webSocketServer.handshake(client)) {
      Serial.println("Handshake Successful");
    }
  }

  displayRefresh(ipAddr, clientConnected);
  delay(1000);
}

void displayRefresh(IPAddress ip, bool clientConnected) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("SSID: ");
  display.println(WiFi.SSID());
  display.print("IP  : ");
  display.println(ip);
  display.println(clientConnected ? "Client Connected!" : "No Client");
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
