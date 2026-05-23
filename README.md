# UCA21 Fire Detector - Detecteur de flamme LoRaWAN

Projet de detection de flamme avec alerte LoRaWAN sur The Things Network (TTN), developpe sur la carte **UCA Education Board 2021** (UCA21) de l'Universite Cote d'Azur.

## Materiel utilise

| Composant | Description |
|---|---|
| UCA21 Board | ATMega328PB + RFM95W (LoRa 868MHz) |
| Capteur IR flamme | Sortie digitale sur D5 |
| Buzzer | Sur D3 |
| LEDs WS2812 | 21 LEDs integrees sur D4 |
| Module relais 5V | Sur D2 (pour la pompe) |
| Mini pompe eau 5V | Controlee via relais |

## Branchement

```
Capteur IR  : VCC -> 5V | GND -> GND | OUT -> D5
Buzzer      : (+) -> D3 | (-) -> GND
Relais      : VCC -> 5V | GND -> GND | IN  -> D2
Pompe       : (+) -> NO relais | (-) -> GND | Batterie (+) -> COM relais
```

## Structure du projet

```
UCA21-Fire-Detector/
│
├── 1_Sans_LoRa/
│   └── detecteur_basic.ino        -> IR + Buzzer + LEDs seulement
│
├── 2_Avec_LoRa_Sans_Pompe/
│   └── detecteur_lora.ino         -> IR + Buzzer + LEDs + Alerte TTN
│
├── 3_Avec_LoRa_Et_Pompe/
│   └── detecteur_complet.ino      -> IR + Buzzer + LEDs + TTN + Pompe
│
└── README.md
```

## Comportement

- **Pas de flamme** : LEDs vertes, silence, pompe OFF
- **Flamme detectee** : LEDs alternance rouge/bleu + buzzer + pompe ON + alerte TTN

## Configuration TTN (ABP)

- **LoRaWAN version** : MAC V1.0.3
- **Regional Parameters** : RP001 Rev A
- **Frequency plan** : Europe 863-870 MHz
- **Activation** : ABP

## Payload Cayenne LPP

| Octet | Valeur | Description |
|---|---|---|
| 0x01 | Channel 1 | - |
| 0x00 | Digital Input | Type |
| 0x01 | 1 | Flamme detectee |
| 0x00 | 0 | RAS |

## Bibliotheques requises

- **IBM LMIC framework** v1.5.1
- **FastLED**

## Auteurs

Projet realise dans le cadre du cours communication sans fil - Universite Cote d'Azur

