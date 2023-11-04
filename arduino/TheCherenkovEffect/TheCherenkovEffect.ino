// NOTE: I use an Arduino Giga, so all pins are usable for interrupts - check your board specs to find which pins can be used
// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
int inputPins[5] = {
  2, 3, 4, 5, 6
};
int clicksPerMinute[5] = {
  0, 0, 0, 0, 0
};
int clickCount[5] = {
  0, 0, 0, 0, 0
};
int prevClickCount[5] = {
  0, 0, 0, 0, 0
};
float microSievertsPerhour[5] = {
  0.0, 0.0, 0.0, 0.0, 0.0
};
int bucketCapacity[5] = {
  250, 250, 250, 250, 250
};
float microSievertsPerHourMultiplier = 0.0057;
long clickCountPeriod = 12000;
long startCountTime;
long lastCountTime;
long startBucketCountTime;
long lastBucketCountTime;
int bucketCountPeriod = 500;
int dripRate = 5;
int dropSize = 20;
int bucketLowerBound = 250;
int bucketUpperBound = 4000;

void setup() {
  setupSerial();
  setupTimers();
  setupPins();
}

void onPulse1() {
  clickCount[0]++;
}

void onPulse2() {
  clickCount[1]++;
}

void onPulse3() {
  clickCount[2]++;
}

void onPulse4() {
  clickCount[3]++;
}

void onPulse5() {
  clickCount[4]++;
}

// This function opens a serial port that communicates with Processing
void setupSerial() {
  Serial.begin(9600);
}

void setupTimers() {
  startCountTime = lastCountTime = millis();
  startBucketCountTime = startCountTime;
}

void setupPins() {
  for (int x = 0 ; x < sizeof(inputPins)/sizeof(int); x++) {
    pinMode(inputPins[x], INPUT);
  }
  
  // Attaching an interrupt lets us call a function when a pulse is sent
  // Had to do a call for each pin as lambda wouldn't work
  attachInterrupt(digitalPinToInterrupt(inputPins[0]), onPulse1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[1]), onPulse2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[2]), onPulse3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[3]), onPulse4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[4]), onPulse5, CHANGE);
}

void loop() {
  // If we've received a click (or more) since last loop
  for (int x = 0 ; x < sizeof(inputPins)/sizeof(int); x++) {
    if (prevClickCount[x] < clickCount[x]) {
      prevClickCount[x] = clickCount[x];
      bucketCapacity[x] += dropSize;
      clampBucketCapacity(x);
      
      // Send a message to processing
      Serial.print("pin:");
      Serial.print(x);
      Serial.print(",uSv/hr:");
      Serial.print(microSievertsPerhour[x]);
      Serial.print(",bucket:");
      Serial.println(bucketCapacity[x]);
    }
  }
  
  // This block is used to calculate the microSievertsPerhour
  if (millis() - lastCountTime > clickCountPeriod) {
    // Clicks per minute = clicks * 60000 millisecons, divided by the current time
    // e.g. 60000 * 10 / (10000 - 0) = 6 clicks in 10 seconds / 60 clicks per minute
    for (int x = 0 ; x < sizeof(inputPins)/sizeof(int); x++) {
      clicksPerMinute[x] =  (60000 * clickCount[x]) / (millis() - startCountTime);
      clickCount[x] = 0;
      prevClickCount[x] = 0;

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
      microSievertsPerhour[x] = clicksPerMinute[x] * microSievertsPerHourMultiplier;
    }
    
    startCountTime = millis();
    lastCountTime += clickCountPeriod;
  }

  // Bucket drip
  if (millis() - lastBucketCountTime > bucketCountPeriod) {
    startBucketCountTime = millis();
    lastBucketCountTime += bucketCountPeriod;
    for (int x = 0 ; x < sizeof(inputPins)/sizeof(int); x++) {
      bucketCapacity[x] -= dripRate;
      clampBucketCapacity(x);
    }
  }
  
  delay(10);
}

void clampBucketCapacity(int pin) {
  if (bucketCapacity[pin] < bucketLowerBound) {
    bucketCapacity[pin] = bucketLowerBound;
  }

  if (bucketCapacity[pin] > bucketUpperBound) {
    bucketCapacity[pin] = bucketUpperBound;
  }
}