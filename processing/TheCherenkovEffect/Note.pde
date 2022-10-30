public class Note {
  int channel;
  int velocity;
  int pitch;
  long duration;
  long time;
  boolean isPlaying;
  MidiBus bus;

  Note(MidiBus bus, int channel, int velocity, int pitch, int duration) {
    this.bus = bus;
    this.channel = channel;
    this.velocity = velocity;
    this.pitch = pitch;
    this.duration = duration;
    this.isPlaying = false;
    this.time = millis();
  }
  
  boolean isRunning() {
    return isPlaying && millis() - this.time < this.duration; 
  }
  
  boolean hasStarted() {
    return isPlaying;
  }
  
  void on() {
    bus.sendNoteOn( this.channel, this.pitch, this.velocity );
    this.isPlaying = true;
  }
  
  void off() {
    bus.sendNoteOff( this.channel, this.pitch, this.velocity );
  }
}
