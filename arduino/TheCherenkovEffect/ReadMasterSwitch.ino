void readMasterSwitch() {
  int reading = digitalRead(performancePin);
  isPerformanceSwitchOn = (reading == HIGH);
}