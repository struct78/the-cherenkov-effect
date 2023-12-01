public class ControllerChange {
  int channel;
  int number;
  int value;
  MidiBus bus;

  ControllerChange(MidiBus bus, int channel, int number, int value) {
    this.bus = bus;
    this.channel = channel;
    this.number = number;
    this.value = value;
  }
  
  void send() {
    bus.sendControllerChange(this.channel, this.number, this.value);
  }
  
  void send(int value) {
    bus.sendControllerChange(this.channel, this.number, value);
  }
}
