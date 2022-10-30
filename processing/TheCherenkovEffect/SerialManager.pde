class SerialManager {
  Serial input;
  int breakCharacter;
  
  SerialManager(Serial input) {
    this.input = input;
    this.breakCharacter = 10;
  }
  
  String listen() {
    String result = null;
    while (this.input.available() > 0) {
      result = this.input.readStringUntil(this.breakCharacter);
    }
    this.input.clear();
    return trim(result);
  }
}
