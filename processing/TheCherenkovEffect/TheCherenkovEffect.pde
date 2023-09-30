import javax.sound.midi.*;
import processing.serial.*;
import themidibus.*;

ArrayList<Note> notes;
Note note;
MidiBus bus;
Serial input;
SerialManager serialManager;
Sphere sphere;
String arduinoAddress = "/dev/tty.usbmodem144301";

void settings() {
  size(1024, 768, P3D);
}

void setup() {
  printArray(Serial.list());
  setupMidi();
  setupSerial();
  setupShutdownHook();
  setupSphere();
}

void draw() {
  String in = serialManager.listen();
  
  if (in != null) {
    println(in);
    if (in.indexOf(":") == -1 || in.indexOf(",") == -1) {
      return;
    }
    
    String[] parts = in.split(",");
    String[] pin = parts[0].split(":");
    String[] usvHr = parts[1].split(":");
    String[] bucket = parts[2].split(":");
    int bucketLevel = int(bucket[1]);
    float msph = float(usvHr[1]);
    float scale = map(msph, 0, 1, 10, 40);
    float velocity = map(msph, 0, 1, 20, 100);
    note = new Note(bus, int(pin[1]), int(velocity), int(scale), bucketLevel);
    notes.add(note);
  }
  
  sphere.draw();
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

void setupSphere() {
  sphere = new Sphere(500, 500);
  sphere.seed();
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
