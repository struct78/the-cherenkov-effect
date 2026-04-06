#include <Wire.h>
#include <Control_Surface.h>
#include "MidiClock.h"

#define MICRO_SIEVERTS_PER_HOUR_MULTIPLIER 0.0057

// Pin definitions
#define GEIGER_COUNTER_INPUT_PIN 2
#define VOLTMETER_PIN 5 // PWM pin - meter inertia smooths the signal

// Voltmeter settings
#define BACKGROUND_RADIATION_USVH 0.2                    // Typical background radiation in μSv/h
#define MAX_DISPLAY_USVH (BACKGROUND_RADIATION_USVH * 2) // 10x background = 2.0 μSv/h
#define PWM_RESOLUTION 255                               // 8-bit PWM

// Timing constants
#define ROLLING_WINDOW_MS 60000 // 60-second rolling window for CPM
#define MAIN_LOOP_DELAY_MS 10
#define MILLIS_PER_MINUTE 60000

// Serial communication
#define SERIAL_BAUD_RATE 9600

// MIDI settings
#define MIDI_VELOCITY 0x60
#define MIDI_CHANNEL Channel_1

const uint8_t MIDI_NOTES[] = {60, 62, 61, 58, 61, 60};
const int MIDI_NOTES_COUNT = sizeof(MIDI_NOTES) / sizeof(MIDI_NOTES[0]);

int midiNoteIndex = 0;

USBMIDI_Interface midi; // TODO: USBMIDI conflicts with USB Serial on the Giga's shared USB-C port

// NOTE: I use an Arduino Giga, so all pins are usable for interrupts - check your board specs to find which pins can be used
// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

// Radiation variables (runtime values)
int clicksPerMinute = 0;
int totalClicks = 0;
float microSievertsPerhour = 0.0;

// Rolling window click buffer
#define CLICK_BUFFER_SIZE 256
volatile unsigned long clickTimestamps[CLICK_BUFFER_SIZE];
volatile int clickBufferHead = 0;
volatile int clickBufferCount = 0;

// MIDI state variables
bool isFirstLoop = true;
bool isPerformanceSwitchOn = true;

// Quantization state
volatile bool pendingNoteOn = false;
volatile bool pendingNoteOff = false;
unsigned long lastProcessedTick = 0;

// MIDI objects
Channel channel = MIDI_CHANNEL;

void setup()
{
  Wire.begin();
  setupSerial();
  setupPins();
  setupClock();
  setupMidi();
  setBpm(120);
}

void setupSerial()
{
  Serial.begin(SERIAL_BAUD_RATE);
}

void setupPins()
{
  pinMode(GEIGER_COUNTER_INPUT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GEIGER_COUNTER_INPUT_PIN), onPulse, RISING);

  // Initialize voltmeter PWM output
  pinMode(VOLTMETER_PIN, OUTPUT);
  analogWrite(VOLTMETER_PIN, 128); // Start at half-scale for wiring test
}

void setupMidi()
{
  Control_Surface.begin();
}

void handlePulse()
{
  // Record click timestamp in circular buffer
  clickTimestamps[clickBufferHead] = millis();
  clickBufferHead = (clickBufferHead + 1) % CLICK_BUFFER_SIZE;
  if (clickBufferCount < CLICK_BUFFER_SIZE)
  {
    clickBufferCount++;
  }

  totalClicks++;

  if (totalClicks % 2 == 0)
  {
    pendingNoteOn = true;
  }
  else
  {
    pendingNoteOff = true;
  }
}

void onPulse()
{
  handlePulse();
}

int mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

void computeRadiation()
{
  unsigned long now = millis();

  // Count clicks within the rolling window
  int count = 0;
  int bufCount = clickBufferCount;
  for (int i = 0; i < bufCount; i++)
  {
    int idx = (clickBufferHead - 1 - i + CLICK_BUFFER_SIZE) % CLICK_BUFFER_SIZE;
    if (now - clickTimestamps[idx] <= ROLLING_WINDOW_MS)
    {
      count++;
    }
    else
    {
      break; // Older entries are further back, no need to keep checking
    }
  }

  clicksPerMinute = count;
  microSievertsPerhour = clicksPerMinute * MICRO_SIEVERTS_PER_HOUR_MULTIPLIER;
}

void sendNoteOn()
{
  if (!isPerformanceSwitchOn || isFirstLoop)
  {
    return;
  }

  MIDIAddress address = {MIDI_NOTES[midiNoteIndex], channel};
  midi.sendNoteOn(address, MIDI_VELOCITY);
  midiNoteIndex = (midiNoteIndex + 1) % MIDI_NOTES_COUNT;
}

void sendNoteOff()
{
  if (!isPerformanceSwitchOn || isFirstLoop)
  {
    return;
  }

  MIDIAddress address = {MIDI_NOTES[(midiNoteIndex - 1 + MIDI_NOTES_COUNT) % MIDI_NOTES_COUNT], channel};
  midi.sendNoteOff(address, MIDI_VELOCITY);
}

void readControlSurface()
{
  Control_Surface.loop();
}

void updateVoltmeter()
{
  // Calculate voltage output for the meter (0-5V representing 0 to 10x background radiation)
  float voltageRatio = microSievertsPerhour / MAX_DISPLAY_USVH;

  // Clamp the ratio between 0 and 1
  if (voltageRatio < 0.0)
  {
    voltageRatio = 0.0;
  }

  if (voltageRatio > 1.0)
  {
    voltageRatio = 1.0;
  }

  // Convert to PWM value (0-255 for 8-bit PWM)
  int pwmValue = int(voltageRatio * PWM_RESOLUTION);

  // Calculate actual voltage for logging
  float actualVoltage = voltageRatio * 5.0;

  // Log voltmeter output values
  Serial.print("Voltmeter: ");
  Serial.print(microSievertsPerhour, 3);
  Serial.print(" μSv/h → ");
  Serial.print(actualVoltage, 2);
  Serial.print("V (PWM: ");
  Serial.print(pwmValue);
  Serial.println(")");

  // Output to voltmeter via PWM
  analogWrite(VOLTMETER_PIN, pwmValue);
}

void processQuantizedNotes()
{
  updateClock();

  if (!isOnGridBoundary() || clockTickCount == lastProcessedTick)
  {
    return;
  }

  lastProcessedTick = clockTickCount;

  if (pendingNoteOff)
  {
    sendNoteOff();
    pendingNoteOff = false;
  }

  if (pendingNoteOn)
  {
    sendNoteOn();
    pendingNoteOn = false;
  }
}

void loop()
{
  computeRadiation();
  processQuantizedNotes();
  updateVoltmeter(); // Update the analog voltmeter display
  readControlSurface();
  delay(MAIN_LOOP_DELAY_MS);
  isFirstLoop = false;
}