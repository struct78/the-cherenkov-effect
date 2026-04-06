// Calculate millisieverts per hour
// e.g. 60 clicks per minute * 0.0057 = 0.342 µSv/hr
// 
// Most people are exposed to about 3000 µSv per year. 
// - Chest x-ray = 100 µSv -> 10 days of background radiation
// - Mammogram = 400 µSv -> 7 weeks of background radiation
// - CT Scan = = 10000 µSv / 10 mSv -> 3.5 years of background radiation
// - PET/CT Scan = 25000 µSv / 25 mSv -> 8 years of background radiation
// 
// - 1,000,000 µSv / 1000 mSv = Radiation sickness
// - 10,000,000 µSv / 10,000 mSv = Dead within weeks

void computeRadiation() {
  // This block is used to calculate the microSievertsPerhour
  if (millis() - lastCountTime > clickCountPeriod) {
    // Clicks per minute = clicks * 60000 millisecons, divided by the current time
    // e.g. 60000 * 10 / (10000 - 0) = 6 clicks in 10 seconds / 60 clicks per minute
    for (int x = 0 ; x < sizeof(geigerCounterInputPins)/sizeof(int); x++) {
      clicksPerMinute[x] =  (60000 * clicks[x]) / (millis() - startCountTime);
      clicks[x] = 0;
      prevClicks[x] = 0;

      microSievertsPerhour[x] = clicksPerMinute[x] * MICRO_SIEVERTS_PER_HOUR_MULTIPLIER;
      octaves[x] = constrain(mapFloat(microSievertsPerhour[x], 0, 0.2, MIN_OCTAVE, MAX_OCTAVE), MIN_OCTAVE, MAX_OCTAVE);

      if (x == 0) {
        Serial.print("µSv channel ");
        Serial.print(x);
        Serial.print(": ");
        Serial.println(microSievertsPerhour[x]);

        // octaves[x] = mapFloat(microSievertsPerhour[x], 0, 0.2, 3, 8);
        Serial.print("Octave: ");
        Serial.println(octaves[x]);
        Serial.println();
        Serial.print("Note: ");
        Serial.println(bar[beat][octaves[x]]);
        Serial.print("Beat: ");
        Serial.println(sizeof(bar));
        Serial.println(beat);
      }
    }
    
    startCountTime = millis();
    lastCountTime += clickCountPeriod;
  }
}