#include "MidiClock.h"

#ifdef USE_HARDWARE_MIDI_CLOCK

// Reads MIDI clock (0xF8) from Serial1 via the SparkFun MIDI Shield
// MIDI IN on the shield is routed to the Arduino's hardware RX pin

#define MIDI_CLOCK_TICK_MSG 0xF8
#define MIDI_START_MSG 0xFA
#define MIDI_CONTINUE_MSG 0xFB
#define MIDI_STOP_MSG 0xFC
#define MIDI_BAUD_RATE 31250

volatile unsigned long clockTickCount = 0;
volatile bool clockRunning = false;
int quantizeSubdivision = QUANTIZE_16TH;

// BPM detection
unsigned long lastTickMicros = 0;
float bpm = 0.0;

void setupClock()
{
    Serial1.begin(MIDI_BAUD_RATE);
}

void updateClock()
{
    while (Serial1.available())
    {
        uint8_t msg = Serial1.read();

        switch (msg)
        {
        case MIDI_CLOCK_TICK_MSG:
            if (clockRunning)
            {
                unsigned long now = micros();
                if (lastTickMicros > 0)
                {
                    unsigned long interval = now - lastTickMicros;
                    bpm = 60000000.0 / (interval * TICKS_PER_QUARTER_NOTE);
                }
                lastTickMicros = now;
                clockTickCount++;
            }
            break;
        case MIDI_START_MSG:
            clockTickCount = 0;
            clockRunning = true;
            break;
        case MIDI_CONTINUE_MSG:
            clockRunning = true;
            break;
        case MIDI_STOP_MSG:
            clockRunning = false;
            break;
        }
    }
}

bool isOnGridBoundary()
{
    return clockRunning && (clockTickCount % quantizeSubdivision == 0);
}

#endif
