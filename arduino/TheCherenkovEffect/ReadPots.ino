void readPots() {
  int jitter = 5;

  for (int x = 0; x < sizeof(potPins) / sizeof(potPins[0]); x++) {
    for (int y = 0; y < sizeof(potPins[x]) / sizeof(potPins[x][0]); y++) {
      int nextValue = map(analogRead(potPins[x][y]), 0, 4095, 0, 127);
      int currentValue = potValues[x][y];

      if (abs(nextValue - currentValue) >= jitter) {
        potValues[x][y] = nextValue;
        sendControlChange(controlChannels[x][y], nextValue);

        Serial.print("Column[");
        Serial.print(x);
        Serial.print("] Row[");
        Serial.print(y);
        Serial.print("]): ");
        Serial.print(currentValue);
        Serial.print("/");
        Serial.println(nextValue);
      }
    }
  }
}