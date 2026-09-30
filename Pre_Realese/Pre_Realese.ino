#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 5
#define PIN_DATA 3

// PINNER
const int sensorPin = 2;
const int knappAvpaaPin = 4;
const int fKnappPin = 5;

// PIR
bool pirAktiv = true;

// Modus
// 0 = statisk
// 1 = blink
// 2 = regnbue
int modus = 0;

// PIR AV/PÅ
bool forrigeKnappState = LOW;

// MODUSER
int fKnappState = HIGH;
int forrigeFKnappState = HIGH;

unsigned long pressStartTime = 0;

bool modusByttet = false;

const unsigned long holdetid = 2000;

// BLINK
unsigned long forrigeBlink = 0;

bool lysPaa = false;

// For regnbue
uint8_t hue = 0;

CRGB leds[NUM_LEDS];


void setup()
{
  // PIR
  pinMode(sensorPin, INPUT);

  // Knappene kobles mellom pin og GND
  pinMode(knappAvpaaPin, INPUT_PULLUP);
  pinMode(fKnappPin, INPUT_PULLUP);

  Serial.begin(9600);

  FastLED.addLeds<NEOPIXEL, PIN_DATA>(leds, NUM_LEDS);

  Serial.println("System startet");
  Serial.println("Modus 0: Statisk");
}


void loop()
{
  // KNAPP 1 - PIR AV/PÅ

  int knappState = digitalRead(knappAvpaaPin);

  // Oppdager et nytt trykk
  if (knappState == LOW && forrigeKnappState == HIGH)
  {
    pirAktiv = !pirAktiv;

    if (pirAktiv)
    {
      Serial.println("PIR AKTIVERT");
    }
    else
    {
      Serial.println("PIR DEAKTIVERT");

      // Skru av LED-ene
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }

      FastLED.show();
    }

    delay(50);
  }

  forrigeKnappState = knappState;


  // KNAPP 2 - BYTTE MODUS

  fKnappState = digitalRead(fKnappPin);

  // Knappen blir trykket ned
  if (fKnappState == LOW && forrigeFKnappState == HIGH)
  {
    // Start å telle hvor lenge knappen holdes
    pressStartTime = millis();

    // Vi har ikke byttet modus enda
    modusByttet = false;

    Serial.println("fKnapp trykket");
  }

  // Knappen holdes inne
  if (fKnappState == LOW && modusByttet == false)
  {
    unsigned long pressDuration = millis() - pressStartTime;

    // Hvis knappen har vært holdt inne i 2 sekunder
    if (pressDuration >= holdetid)
    {
      // Bytt til neste modus
      modus++;

      // Hvis vi kommer over modus 2,
      // går vi tilbake til modus 0
      if (modus > 2)
      {
        modus = 0;
      }

      // Vis hvilken modus vi er i
      Serial.print("Ny modus: ");
      Serial.println(modus);

      if (modus == 0)
      {
        Serial.println("STATISK");
      }
      else if (modus == 1)
      {
        Serial.println("BLINK");
      }
      else if (modus == 2)
      {
        Serial.println("REGNBUE");
      }

      // Hindrer at samme hold registrerer flere ganger
      modusByttet = true;
    }
  }

  // Når knappen slippes
  if (fKnappState == HIGH)
  {
    modusByttet = false;
  }

  forrigeFKnappState = fKnappState;


  // PIR + LYS

  if (pirAktiv)
  {
    int sensorState = digitalRead(sensorPin);

    // PIR registrerer bevegelse
    if (sensorState == HIGH)
    {

      // MODUS 0 - STATISK

      if (modus == 0)
      {
        for (int i = 0; i < NUM_LEDS; i++)
        {
          leds[i] = CRGB::Red;
        }

        FastLED.show();
      }


      // MODUS 1 - BLINK

      else if (modus == 1)
      {
        // Blink hvert 500 ms
        if (millis() - forrigeBlink >= 500)
        {
          forrigeBlink = millis();

          lysPaa = !lysPaa;

          if (lysPaa)
          {
            for (int i = 0; i < NUM_LEDS; i++)
            {
              leds[i] = CRGB::Red;
            }
          }
          else
          {
            for (int i = 0; i < NUM_LEDS; i++)
            {
              leds[i] = CRGB::Black;
            }
          }

          FastLED.show();
        }
      }


      // MODUS 2 - REGNBUE

      else if (modus == 2)
      {
        for (int i = 0; i < NUM_LEDS; i++)
        {
          leds[i] = CHSV(hue + (i * 30), 255, 255);
        }

        FastLED.show();

        hue++;

        delay(20);
      }
    }


    // INGEN BEVEGELSE

    else
    {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Black;
      }

      FastLED.show();
    }
  }
}