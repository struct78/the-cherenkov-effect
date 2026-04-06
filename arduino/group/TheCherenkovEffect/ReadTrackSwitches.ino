void readTrackSwitches() {
  for (int x = 0 ; x < sizeof(trackPins) / sizeof(int); x++) {
    int reading = digitalRead(trackPins[x]);
    int currentReading = trackValues[x];

    if (reading == HIGH) {
      trackValues[x]++;

      if (trackValues[x] >= 5) {
        trackValues[x] = 0;
      }

      Serial.println(trackValues[x]);
    }
  }
}