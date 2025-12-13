#include <SparkFun_Alphanumeric_Display.h>
#include <Wire.h>
#include <Control_Surface.h>

#define MICRO_SIEVERTS_PER_HOUR_MULTIPLIER 0.0057
#define MIN_OCTAVE 3
#define MAX_OCTAVE 8

HT16K33 display;
USBMIDI_Interface midi;

// NOTE: I use an Arduino Giga, so all pins are usable for interrupts - check your board specs to find which pins can be used
// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

// Radiation
int geigerCounterInputPins[5] = {
  2, 3, 4, 5, 6
};

int clicksPerMinute[5] = {
  0, 0, 0, 0, 0
};

int clicks[5] = {
  0, 0, 0, 0, 0
};

int totalClicks[5] = {
  0, 0, 0, 0, 0
};

int prevClicks[5] = {
  0, 0, 0, 0, 0
};

float microSievertsPerhour[5] = {
  0.0, 0.0, 0.0, 0.0, 0.0
};

long clickCountPeriod = 12000;
long startCountTime;
long lastCountTime;

// MIDI
bool isPerformanceSwitchOn = false;
bool isFirstLoop = true;
int performancePin = D7;
int beat = 0;
int octaves[5] = {};
int velocity = 0x60;

/// MIDI note names.
/// Uses the [Scientific Pitch Notation system](https://en.wikipedia.org/wiki/Scientific_pitch_notation),
/// where <b>A<sub>4</sub></b> is 440 Hz, and <b>C<sub>-1</sub></b> is 8.1758 Hz.
///
/// |Octave| C |C♯/D♭| D |D♯/E♭| E | F  |F♯/G♭| G |G♯/A♭| A |A♯/B♭| B  |
/// |:-----|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
/// |-1    |   0|   1|   2|   3|   4|   5|   6|   7|   8|   9|  10|  11|
/// |0     |  12|  13|  14|  15|  16|  17|  18|  19|  20|  21|  22|  23|
/// |1     |  24|  25|  26|  27|  28|  29|  30|  31|  32|  33|  34|  35|
/// |2     |  36|  37|  38|  39|  40|  41|  42|  43|  44|  45|  46|  47|
/// |3     |  48|  49|  50|  51|  52|  53|  54|  55|  56|  57|  58|  59|
/// |4     |  60|  61|  62|  63|  64|  65|  66|  67|  68|  69|  70|  71|
/// |5     |  72|  73|  74|  75|  76|  77|  78|  79|  80|  81|  82|  83|
/// |6     |  84|  85|  86|  87|  88|  89|  90|  91|  92|  93|  94|  95|
/// |7     |  96|  97|  98|  99| 100| 101| 102| 103| 104| 105| 106| 107|
/// |8     | 108| 109| 110| 111| 112| 113| 114| 115| 116| 117| 118| 119|

MIDI_Notes::Note bar[4] = {
  MIDI_Notes::Ab,
  MIDI_Notes::Eb,
  MIDI_Notes::F,
  MIDI_Notes::Db,
};

MIDIAddress previousNotes[5] = {};

Channel channels[5] = {
  { Channel_1 },
  { Channel_2 },
  { Channel_3 },
  { Channel_4 },
  { Channel_5 },
};

MIDIAddress controlChannels[2][5] = {
  { { 0xB0, Channel_1 }, { 0xB1, Channel_1 }, { 0xB2, Channel_1 }, { 0xB3, Channel_1 }, { 0xB4, Channel_1 } },
  { { 0xB0, Channel_2 }, { 0xB1, Channel_2 }, { 0xB2, Channel_2 }, { 0xB3, Channel_2 }, { 0xB4, Channel_2 } }
};

uint8_t potPins[2][5] = {
  { A0, A1, A2, A3, A4 },
  { A5, A6, A7, A12, A13 },
};

int potValues[2][5] = {
  { 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0 },
};

int trackPins[1] = {
  D8
};

int trackValues[1] = {
  0
};


void setup() {
  Wire.begin();
  // setupDisplay();
  setupSerial();
  setupPins();
  setupMidi();
  setupTimers();
}

// This function opens a serial port that communicates with Processing
void setupSerial() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
}

void setupTimers() {
  startCountTime = lastCountTime = millis();
}

void setupPins() {
  pinMode(performancePin, INPUT);

  for (int x = 0; x < sizeof(trackPins) / sizeof(int); x++) {
    pinMode(trackPins[x], INPUT);
  }

  for (int x = 0; x < sizeof(geigerCounterInputPins) / sizeof(int); x++) {
    pinMode(geigerCounterInputPins[x], INPUT);
  }

  // Attaching an interrupt lets us call a function when a pulse is sent
  attachInterrupt(digitalPinToInterrupt(geigerCounterInputPins[0]), onPulse1, RISING);
  attachInterrupt(digitalPinToInterrupt(geigerCounterInputPins[1]), onPulse2, RISING);
  attachInterrupt(digitalPinToInterrupt(geigerCounterInputPins[2]), onPulse3, RISING);
  attachInterrupt(digitalPinToInterrupt(geigerCounterInputPins[3]), onPulse4, RISING);
  attachInterrupt(digitalPinToInterrupt(geigerCounterInputPins[4]), onPulse5, RISING);
}

void setupMidi() {
  Control_Surface.begin();
}

void setupDisplay() {
  if (display.begin() == false) {
    Serial.println("Waiting for LCD displays...");
    while (1)
      ;
  }

  display.print("Testing things");
}

void handlePulse(int channel) {
  // Serial.print("Pulse on: ");
  // Serial.println(channel);
  clicks[channel]++;
  totalClicks[channel]++;
  
  beat++;

  if (beat > sizeof(bar)) {
    beat = 0;
  }

  if (totalClicks[channel] % 2 == 0) {
    sendNoteOn({ bar[beat][octaves[channel]], channels[channel] }, velocity);
    previousNotes[channel] = { bar[beat][octaves[channel]], channels[channel] };
  } else {
    sendNoteOff(previousNotes[channel], velocity);
  }
}

void onPulse1() {
  handlePulse(0);
}

void onPulse2() {
  handlePulse(1);
}

void onPulse3() {
  handlePulse(2);
}

void onPulse4() {
  handlePulse(3);
}

void onPulse5() {
  handlePulse(4);
}

void loop() {
  computeRadiation();
  readControlSurface();
  readMasterSwitch();
  readTrackSwitches();
  readPots();
  delay(50);
  isFirstLoop = false;
}

int mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
