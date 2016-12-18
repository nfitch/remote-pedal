#include <Bounce2.h>

/*****************************
      +------------------+
      | mode  down   up  |
      |  o     o      o  |
      +------------------+
TRS      T     R     TR
GPIO     11    12    11/12

******************************/

// Our Bounce Switches and state
Bounce b11;
Bounce b12;
int b11fall = 0;
int b12fall = 0;

void setup() {
  Serial.begin(9600);
  // initialize digital pin 11 and 12 as inputs with pullup resistors.
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  b11 = Bounce();
  b12 = Bounce();
  b11.attach(11);
  b12.attach(12);
}

void report(String button, int startTime, int endTime) {
  Serial.print(button);
  Serial.print(" pressed for ");
  Serial.print((endTime - startTime) / 1000);
  Serial.println(" seconds");
}

void loop() {
  bool b11updated = b11.update();
  bool b12updated = b12.update();

  // Since we use internal pullup, fall is push, rise is release. 
  if (b11updated || b12updated) {
    if (b11.fell()) {
      b11fall = millis();
    }
    if (b12.fell()) {
      b12fall = millis();
    }
    bool changed = 0;
    if (b11.rose()) {
      if (b11fall != 0) {
        if (b12fall != 0) {
          report("right", b11fall, millis());
        } else {
          report("left", b11fall, millis());
        }
      }
      changed = 1;
    } else if (b12.rose()) {
      if (b12fall != 0) {
        if (b11fall != 0) {
          report("right", b12fall, millis());
        } else {
          report("middle", b12fall, millis());
        }
      }
      changed = 1;
    }
    if (changed) {
      b11fall = 0;
      b12fall = 0;
    }
  }
}

