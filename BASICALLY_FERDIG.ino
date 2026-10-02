#include <Arduino.h>
#include <FastLED.h>

const uint8_t PIR_PIN = 2;
const uint8_t LED_DATA_PIN = 3;
const uint8_t BUTTON1_PIN = 5; // System on/off
const uint8_t BUTTON2_PIN = 4; // kort trykk = farge, hold = modus
const uint8_t NUM_LEDS = 5;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long LONG_PRESS_MS = 2000;

CRGB leds[NUM_LEDS];

struct DebouncedButton {
  uint8_t pin;
  int stableState;
  int lastRawState;
  unsigned long lastChangeTime;
};

DebouncedButton button1{BUTTON1_PIN, LOW, LOW, 0};
DebouncedButton button2{BUTTON2_PIN, LOW, LOW, 0};

bool systemOn = false;

// MODUS
// 0 = statisk
// 1 = blink
// 2 = regnbue
int modus = 0;

//FARGE 
// 0 = grønn
// 1 = rød
// 2 = blå
// 3 = gul 
// 4 = lilla 

int farge = 0;

bool button2PressActive = false;
bool button2LongPressHandled = false;
unsigned long button2PressStartTime = 0;

// PIR
int lastPirState = -1;

// BLINK
unsigned long forrigeBlink = 0;
bool lysPaa = false;

// REGNBUE
uint8_t hue = 0;


bool updateButton(DebouncedButton &button) {
  int rawState = digitalRead(button.pin);

  if (rawState != button.lastRawState) {
    button.lastRawState = rawState;
    button.lastChangeTime = millis();
  }

  if (millis() - button.lastChangeTime >= DEBOUNCE_MS &&
      rawState != button.stableState) {
    button.stableState = rawState;

    // true når knappen blir trykket ned
    if (button.stableState == HIGH){
      return true;
    }
  }

  return false;
}

// FINN VALGT FARGE
CRGB hentFarge() {

  if (farge == 0) {
    return CRGB::Green;
  }

  else if (farge == 1) {
    return CRGB::Red;
  }

  else if (farge == 2) {
    return CRGB::Blue;
  }

  else if (farge == 3) {
    return CRGB::Yellow;
  }

  else if (farge == 4) {
    return CRGB::Purple;
  }

  return CRGB::Green;
}

// SKRIVE UT HVILKEN FARGE
void skrivFarge() {

  if (farge == 0) {
    Serial.println("Farge: GRONN");
  }

  else if (farge == 1) {
    Serial.println("Farge: ROD");
  }

  else if (farge == 2) {
    Serial.println("Farge: BLA");
  }

  else if (farge == 3) {
    Serial.println("Farge: GUL");
  }

  else if (farge == 4) {
    Serial.println("Farge: LILLA");
  }
}

void setup() {
  pinMode(PIR_PIN, INPUT);

  // Knappene er koblet til 5V
  // med eksterne 10 kOhm pull-down motstander
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  button1.stableState = button1.lastRawState = digitalRead(BUTTON1_PIN);
  button2.stableState = button2.lastRawState = digitalRead(BUTTON2_PIN);

  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(160);
  FastLED.clear(true);

  Serial.println("System startet");
  Serial.println("Modus 0: Statisk");
  Serial.println("Farge: grønn");
}


void loop() {

  // KNAPP 1 - SYSTEM AV/PÅ

  if (updateButton(button1)) {

    systemOn = !systemOn;

    lastPirState = -1;

    Serial.println(systemOn ? "System ON" : "System OFF");
  }

  // KNAPP 2 - BYTTE MODUS

  // Knappen blir trykket ned
  if (updateButton(button2)) {

    button2PressActive = true;
    button2LongPressHandled = false;
    button2PressStartTime = millis();

    Serial.println("Knapp 2 trykket");
  }

  // Knappen holdes inne
  if (button2PressActive &&
      button2.stableState == HIGH) {

    unsigned long pressDuration =
      millis() - button2PressStartTime;

    // Hvis knappen har vært holdt i 2 sekunder
    if (pressDuration >= LONG_PRESS_MS) {

      // Gå til neste modus
      modus++;

      // Etter modus 2 går vi tilbake til modus 0
      if (modus > 2) {
        modus = 0;
      }


      // Skriv ut hvilken modus vi har
      if (modus == 0) {
        Serial.println("Ny modus: STATISK");
        lysPaa = false;
      }

      else if (modus == 1) {
        Serial.println("Ny modus: BLINK");

        lysPaa = false;
        forrigeBlink = millis();
      }

      else if (modus == 2) {
        Serial.println("Ny modus: REGNBUE");

        // Start regnbuen på nytt
        hue = 0;
      }

      // Start en ny 2-sekunders periode
      // uten at knappen må slippes
      button2PressStartTime = millis();
      button2LongPressHandled = true;
    }
  }


  // Knappen slippes
  if (button2PressActive &&
      button2.stableState == LOW) {

      if(!button2LongPressHandled){
  
farge++;
      // Etter lilla går vi tilbake til grønn
      if (farge > 4) {
        farge = 0;
      }

      skrivFarge();
      }


       // Ferdig med dette trykket
    button2PressActive = false;

    button2LongPressHandled = false;
  }


  // PIR + LYS
  CRGB outputColor = CRGB::Black;


  if (systemOn) {

    int pirState = digitalRead(PIR_PIN);


    // Rapporter bare når PIR-status endres
    if (pirState != lastPirState) {

      Serial.println(
        pirState == HIGH
        ? "PIR motion detected"
        : "No PIR motion"
      );

      lastPirState = pirState;
    }

    // BEVEGELSE
    if (pirState == HIGH) {

      // MODUS 0 - STATISK
      if (modus == 0) {

        outputColor = hentFarge();
      }

      // MODUS 1 - BLINK
      else if (modus == 1) {

        if (millis() - forrigeBlink >= 500) {

          forrigeBlink = millis();

          lysPaa = !lysPaa;
        }

        if (lysPaa) {
          outputColor = hentFarge();
        }
          else{
            outputColor = CRGB::Black;
          }
      }

       

      // MODUS 2 - REGNBUE
      else if (modus == 2) {

        for (uint8_t i = 0; i < NUM_LEDS; ++i) {

          leds[i] =
            CHSV(hue + (i * 30), 255, 255);
        }

        FastLED.show();

        hue++;

        delay(20);

        // Vi har allerede vist regnbuen
        // så vi hopper over vanlig outputColor
        return;
      }
    }
  }

  // VIS LED-ENE
  for (uint8_t i = 0; i < NUM_LEDS; ++i) {

    leds[i] = outputColor;
  }

  FastLED.show();
}