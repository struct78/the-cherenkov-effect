import javax.sound.midi.*;
import java.util.HashMap;
import processing.serial.*;
import themidibus.*;
import controlP5.*;

enum PlayMode {
  TOGGLE,
  CUMULATIVE,
}

PlayMode playMode = PlayMode.TOGGLE;
int scaleMin = 0;
int scaleMax = 0;
int velocityMin = 0;
int velocityMax = 0;
int controllerChangeChannel = 9;
int customMidiMapChannels = 5;

ControlP5 cp5;
ArrayList<Note> notes;
ArrayList<ControllerChange> controllerChanges;
HashMap<String, Note> noteMap = new HashMap<String, Note>();

ScrollableList usbList;
Note note;
ControllerChange controllers;
MidiBus bus;
Serial input;
SerialManager serialManager;

void settings() {
  size(1250, 180);
}

void setup() {
  setupMidi();
  setupShutdownHook();
  setupControls();
}

void draw() {
  background(245);
  
  if (serialManager == null) {
    return;
  }
  
  String in = serialManager.listen();
  
  if (in != null) {
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
    float scale = map(msph, 0, 1, scaleMin, scaleMax);
    float velocity = map(msph, 0, 1, velocityMin, velocityMax);
    
    note = new Note(bus, channel, int(velocity), int(scale), bucketLevel);
    
    if (playMode == PlayMode.CUMULATIVE) {
      notes.add(note);
    }
    
    if (playMode == PlayMode.TOGGLE) {
      String key = pinInfo[1];
      
      if (noteMap.containsKey(key)) {
        Note currentNote = noteMap.get(key);
        currentNote.off();
        noteMap.remove(key);
      } else {
        note.on();
        noteMap.put(key, note);
      }
    }
  }
  
  playNotes();
}

void setupControls() {
  cp5 = new ControlP5(this);
  cp5.addLabel("SELECT ARDUINO DEVICE", 17, 20).setColor(0x000000);
  
  usbList = cp5.addScrollableList("channels", 20, 40, 200, 200).setBarHeight(20).setType(ScrollableList.DROPDOWN).setLabel("Devices").close();
  
  String[] options = Serial.list();
  for (int x = 0 ; x < options.length ; x++) {
    usbList.addItem(options[x], options[x]);
  }
  
  usbList.onChange(new CallbackListener() {
    public void controlEvent(CallbackEvent theEvent) {
      String value = (String)usbList.getItem(int(usbList.getValue())).get("value");
      
      if (value == null) {
        return;
      }
      
      setupSerial(value);
    }
  }).onClick(new CallbackListener() {
    public void controlEvent(CallbackEvent theEvent) {
      if (usbList.isOpen()) {
        usbList.bringToFront();
      } else {
        usbList.close();
      }
    }
  });
  
  cp5.addLabel("MAP MIDI CONTROL CHANNEL EVENTS", 227, 20).setColor(0x000000);
  
  for (int x = 0 ; x < customMidiMapChannels; x++) {
    cp5.addButton("customMidiMapChannel_" + x, x, 230, 40 + (x*20+(x*4)), 200, 20).setLabel("MIDI Map " + x).onClick(new CallbackListener() {
        public void controlEvent(CallbackEvent e) {
          int number = int(e.getController().getValue());
          ControllerChange controllerChange = new ControllerChange(bus, controllerChangeChannel, number, 0);
          controllerChange.send();
        }
    });
  } //<>//
  
  cp5.addLabel("VELOCITY MIN/MAX", 496, 20).setColor(0x000000);
  cp5.addRange("velocity").setPosition(500, 40).setRange(1, 127).setRangeValues(50, 100).onChange(new CallbackListener() {
    public void controlEvent(CallbackEvent e) {
      velocityMin = int(e.getController().getArrayValue(0));
      velocityMax = int(e.getController().getArrayValue(1));
    }
  });
  
  
  cp5.addLabel("SCALE MIN/MAX", 496, 65).setColor(0x000000);
  
  cp5.addRange("scale").setLabel("Scale Min / Max").setPosition(500, 85).setRange(1, 127).setRangeValues(50, 100).onChange(new CallbackListener() {
    public void controlEvent(CallbackEvent e) {
      scaleMin = int(e.getController().getArrayValue(0));
      scaleMax = int(e.getController().getArrayValue(1));
    }
  });
}

void playNotes() {
  if (playMode != PlayMode.CUMULATIVE) {
    return;
  }
  
  for (int i = notes.size() - 1; i >= 0; i--) {
    Note note = notes.get(i);
    if (note.hasStarted() && !note.isRunning()) {
      note.off();
      notes.remove(i);
    }
    
    if (!note.hasStarted()) {
      // TODO - Fix this so it only sends it if they are set
      //for (int x = 0; x < controllerChanges.size(); x++) {
      //  ControllerChange controllerChange = controllerChanges.get(x);
      //  controllerChange.send(int(random(127)));
      //}
      note.on();
    }
  }
}

void setupMidi() {
  notes = new ArrayList<Note>();
  controllerChanges = new ArrayList<ControllerChange>();
  bus = new MidiBus( this, -1, "TheCherenkovEffect" );
  
  for (int x = 0 ; x < customMidiMapChannels ; x++) {
    ControllerChange controllerChange = new ControllerChange(bus, controllerChangeChannel, x, 0);
    controllerChanges.add(controllerChange);
  }
}

void setupSerial(String arduinoAddress) {
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
