#ifndef MIDI_CLOCK_H
#define MIDI_CLOCK_H

// Uncomment to use hardware MIDI clock via SparkFun MIDI Shield on Serial1
// Leave commented to use the virtual (internal timer) clock
// #define USE_HARDWARE_MIDI_CLOCK

// MIDI clock constants
#define TICKS_PER_QUARTER_NOTE 24
#define QUANTIZE_16TH 6     // ticks per 16th note
#define QUANTIZE_8TH 12     // ticks per 8th note
#define QUANTIZE_QUARTER 24 // ticks per quarter note

// Shared clock state
extern volatile unsigned long clockTickCount;
extern volatile bool clockRunning;
extern int quantizeSubdivision;

// Interface — implemented by either HardwareMidiClock or VirtualMidiClock
void setupClock();
void updateClock();
bool isOnGridBoundary();

#endif
