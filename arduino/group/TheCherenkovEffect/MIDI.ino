void sendNoteOn(MIDIAddress channel, int velocity) {
  if (!isPerformanceSwitchOn || isFirstLoop) {
    return;
  }

  midi.sendNoteOn(channel, velocity);
}

void sendNoteOff(MIDIAddress channel, int velocity) {
  if (!isPerformanceSwitchOn || isFirstLoop) {
    return;
  }

  midi.sendNoteOff(channel, velocity);
}

void sendControlChange(MIDIAddress channel, int value) {
  if (isFirstLoop) {
    return;
  }

  midi.sendControlChange(channel, value);
}

void readControlSurface() {
  Control_Surface.loop();
}