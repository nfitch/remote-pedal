#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306();

#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// This is where the Potentiometer pin is hooked up.
#define POT_PIN 5

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
  display.display();
  display.setTextSize(3);
  display.setTextColor(WHITE);
}

// Set to -1 so that it displays the first time
int last = -1;
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
