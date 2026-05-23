/*******************************************************************************
 * UCA21 Board - Detecteur de flamme IR - Version basique sans LoRa
 * 
 * Capteur IR sur D5, Buzzer sur D3, LEDs WS2812 sur D4
 * Bibliotheque : FastLED
 * 
 * Comportement :
 *   - Pas de flamme : LEDs vertes, silence
 *   - Flamme detectee : LEDs alternance bleu/rouge + buzzer
 * 
 * Note : le capteur IR donne LOW quand il detecte une flamme
 ******************************************************************************/

#include <FastLED.h>

// LEDs WS2812
#define DATA_PIN    4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    21
#define BRIGHTNESS  30
CRGB leds[NUM_LEDS];

// Pins
#define BUZZER_PIN  3
#define IR_PIN      5

// Variables
bool alarmActive = false;
unsigned long lastBlink = 0;
bool ledState = false;

// LED helpers
void setAllLeds(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = color;
  FastLED.show();
}

// Buzzer
void beepAlert() {
  tone(BUZZER_PIN, 900, 200);
  delay(250);
  tone(BUZZER_PIN, 1200, 200);
  delay(250);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("=== UCA21 Detecteur IR - Version basique ==="));

  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  setAllLeds(CRGB::Green);
  Serial.println(F("Pret - en attente de flamme..."));
}

void loop() {
  unsigned long now = millis();
  bool flameDetected = (digitalRead(IR_PIN) == LOW);

  if (flameDetected) {
    if (!alarmActive) {
      alarmActive = true;
      Serial.println(F(">>> FLAMME DETECTEE ! <<<"));
    }
    // Alternance bleu/rouge
    if (now - lastBlink > 300) {
      lastBlink = now;
      ledState = !ledState;
      ledState ? setAllLeds(CRGB::Red) : setAllLeds(CRGB::Blue);
    }
    beepAlert();
  } else {
    if (alarmActive) {
      alarmActive = false;
      Serial.println(F("Flamme eteinte - RAS"));
      noTone(BUZZER_PIN);
      setAllLeds(CRGB::Green);
    }
  }
}
