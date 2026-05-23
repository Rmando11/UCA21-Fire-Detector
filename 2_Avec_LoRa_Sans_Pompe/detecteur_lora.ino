/*******************************************************************************
 * UCA21 Board - Detecteur de flamme IR + Alerte LoRaWAN TTN - Sans pompe
 * 
 * Capteur IR sur D5, Buzzer sur D3, LEDs WS2812 sur D4
 * Bibliotheque : IBM LMIC framework v1.5.1 + FastLED
 * 
 * Comportement :
 *   - Pas de flamme : LEDs vertes, silence
 *   - Flamme detectee : LEDs alternance bleu/rouge + buzzer + alerte TTN
 * 
 * Note : le capteur IR donne LOW quand il detecte une flamme
 * 
 * config.h doit contenir :
 *   #define CFG_eu868 1
 *   #define CFG_sx1276_radio 1
 ******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
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

// LoRaWAN keys ABP
static const PROGMEM u1_t NWKSKEY[16] = { 0x27, 0x16, 0xF0, 0x82, 0xEC, 0x49, 0xCC, 0x8A, 0x85, 0x5C, 0x34, 0x98, 0x99, 0x08, 0x3B, 0xE0 };
static const u1_t PROGMEM APPSKEY[16] = { 0x34, 0x50, 0xB5, 0xDC, 0xD9, 0x7B, 0xA4, 0x4C, 0x7E, 0x08, 0x12, 0x46, 0x5F, 0x44, 0xF5, 0x74 };
static const u4_t DEVADDR = 0x260B6574;

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// Pin mapping UCA21
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 8,
  .dio = {6, 6, 6},
};

// Variables
static osjob_t sendjob;
bool alarmActive = false;
bool alertSent = false;
unsigned long lastBlink = 0;
bool ledState = false;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN = 60000;

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

// LoRaWAN send alerte
void sendAlert(osjob_t* j) {
  if (LMIC.opmode & OP_TXRXPEND) return;
  static uint8_t mydata[3];
  mydata[0] = 0x01; mydata[1] = 0x00; mydata[2] = 0x01;
  LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
  Serial.println(F("Alerte TTN envoyee !"));
  lastAlertTime = millis();
  alertSent = true;
}

// LoRaWAN send RAS
void sendClear(osjob_t* j) {
  if (LMIC.opmode & OP_TXRXPEND) return;
  static uint8_t mydata[3];
  mydata[0] = 0x01; mydata[1] = 0x00; mydata[2] = 0x00;
  LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
  Serial.println(F("RAS envoye sur TTN"));
}

void onEvent (ev_t ev) {
  if (ev == EV_TXCOMPLETE) Serial.println(F("EV_TXCOMPLETE"));
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("=== UCA21 Detecteur IR + LoRa - Sans pompe ==="));

  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  setAllLeds(CRGB::Green);

  os_init();
  LMIC_reset();

  #ifdef PROGMEM
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);
  #else
  LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
  #endif

  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);

  LMIC_setLinkCheckMode(0);
  LMIC.dn2Dr = DR_SF9;
  LMIC_setDrTxpow(DR_SF7, 14);

  Serial.println(F("Pret - en attente de flamme..."));
}

void loop() {
  os_runloop_once();
  unsigned long now = millis();
  bool flameDetected = (digitalRead(IR_PIN) == LOW);

  if (flameDetected) {
    if (!alarmActive) {
      alarmActive = true;
      alertSent = false;
      Serial.println(F(">>> FLAMME DETECTEE ! <<<"));
    }
    if (now - lastBlink > 300) {
      lastBlink = now;
      ledState = !ledState;
      ledState ? setAllLeds(CRGB::Red) : setAllLeds(CRGB::Blue);
    }
    beepAlert();
    if (!alertSent && (now - lastAlertTime > ALERT_COOLDOWN)) {
      os_setCallback(&sendjob, sendAlert);
    }
  } else {
    if (alarmActive) {
      alarmActive = false;
      Serial.println(F("Flamme eteinte - RAS"));
      noTone(BUZZER_PIN);
      setAllLeds(CRGB::Green);
      os_setCallback(&sendjob, sendClear);
    }
  }
}
