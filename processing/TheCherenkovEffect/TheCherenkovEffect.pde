import javax.sound.midi.*;
import processing.serial.*;
import themidibus.*;

ArrayList<Note> notes;
Note note;
MidiBus bus;
Serial input;
SerialManager serialManager;
String arduinoAddress = "/dev/tty.usbmodem14301";

void setup() {
  printArray(Serial.list());
  setupMidi();
  setupSerial();
  setupShutdownHook();
}

void draw() {
  String in = serialManager.listen();
  
  if (in != null) {
    println(in);
    if (in.indexOf(":") == -1 || in.indexOf(",") == -1) {
      return;
    }
    
    String[] parts = in.split(",");
    String[] pinInfo = parts[0].split(":");
    String[] usvHr = parts[1].split(":");
    String[] bucket = parts[2].split(":");
    int bucketLevel = int(bucket[1]);
    int channel = int(pinInfo[1]);
    float msph = float(usvHr[1]);
    float scale = map(msph, 0, 1, 30, 42);
    float velocity = map(msph, 0, 1, 20, 100);
    note = new Note(bus, channel, int(velocity), int(scale), bucketLevel);
    notes.add(note);
  }
  
  playNotes();
}

void playNotes() {
  for (int i = notes.size() - 1; i >= 0; i--) {
    Note note = notes.get(i);
    if (note.hasStarted() && !note.isRunning()) {
      note.off();
      notes.remove(i);
    }
    if (!note.hasStarted()) {
      note.on();
    }
  }
}

void setupMidi() {
  notes = new ArrayList<Note>();
  bus = new MidiBus( this, -1, "CherenkovEffect" );
}

void setupSerial() {
  input = new Serial(this, arduinoAddress, 9600);
  serialManager = new SerialManager(input);
}

void setupShutdownHook() {
  Runtime.getRuntime().addShutdownHook( new Thread( new Runnable() {
    public void run() {
      bus.sendMessage( ShortMessage.CONTROL_CHANGE, 0, 0x7B, 0 );
      bus.close();
    }
  }
  ));
}
