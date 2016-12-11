#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306();

#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define POT_PIN 5

void setup() {  
  Serial.begin(9600);

  Serial.println("OLED FeatherWing test");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  
  Serial.println("IO test");

  // text display tests
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Pos: ");
  display.setCursor(0,0);
  display.display(); // actually display all of the above
}

int last = 0;
void loop() {
  // Divide by 10 puts number between 1-10X
  int pot = analogRead(POT_PIN) / 10;
  if (pot != last && pot < 100) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Pos: ");
    display.println(pot);
    display.display();
    last = pot;
  }
  delay(10);
  yield();
}
