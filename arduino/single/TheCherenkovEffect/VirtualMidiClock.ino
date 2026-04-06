#include "MidiClock.h"

#ifndef USE_HARDWARE_MIDI_CLOCK

// Virtual MIDI clock — uses millis() to generate clock ticks at a given BPM
// Drop-in replacement for HardwareMidiClock when no external clock is available

#define DEFAULT_BPM 120

volatile unsigned long clockTickCount = 0;
volatile bool clockRunning = false;
int quantizeSubdivision = QUANTIZE_QUARTER;

float bpm = DEFAULT_BPM;
unsigned long lastTickMicros = 0;
unsigned long tickIntervalMicros = 0;

void computeTickInterval()
{
    // 24 PPQN: interval = 60,000,000 / (BPM * 24) microseconds
    tickIntervalMicros = (unsigned long)(60000000UL / (bpm * TICKS_PER_QUARTER_NOTE));
}

void setupClock()
{
    computeTickInterval();
    lastTickMicros = micros();
    clockRunning = true;
    clockTickCount = 0;
}

void setBpm(float newBpm)
{
    bpm = newBpm;
    computeTickInterval();
}

void updateClock()
{
    if (!clockRunning)
    {
        return;
    }

    unsigned long now = micros();
    while (now - lastTickMicros >= tickIntervalMicros)
    {
        clockTickCount++;
        lastTickMicros += tickIntervalMicros;
    }
}

bool isOnGridBoundary()
{
    return clockRunning && (clockTickCount % quantizeSubdivision == 0);
}

#endif
