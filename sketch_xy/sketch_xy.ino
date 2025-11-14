// ===== CNC Shield v3 pins =====
#define X_STEP 2
#define X_DIR  5
#define Y_STEP 3
#define Y_DIR  6
#define EN_PIN 8   // LOW = enable (shared)

const int STEP_US_X = 50;   // X speed: smaller = faster (you set 2× faster)
const int STEP_US_Y = 200;   // Y speed: tune if needed

// Modes: 'R','L','S' for X  | 'U','D','S' for Y
char modeX = 'S';
char modeY = 'S';

unsigned long tX = 0, tY = 0;
bool lvlX = false, lvlY = false;

void setup() {
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR,  OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR,  OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  digitalWrite(EN_PIN, LOW);    // enable drivers
  digitalWrite(X_STEP, LOW);
  digitalWrite(Y_STEP, LOW);

  // Default directions (don’t matter until a command arrives)
  digitalWrite(X_DIR, HIGH);
  digitalWrite(Y_DIR, HIGH);

  Serial.begin(115200);
}

void handleCommand(char axis, char cmd) {
  if (axis == 'X') {
    modeX = cmd;
    // >>> X axis mapping (FLIPPED per your request) <<<
    if (cmd == 'R')      digitalWrite(X_DIR, LOW);   // flipped: XR -> LOW
    else if (cmd == 'L') digitalWrite(X_DIR, HIGH);  // flipped: XL -> HIGH
    // 'S' stops pulses only
  } else if (axis == 'Y') {
    modeY = cmd;
    // Y axis mapping (normal: Up=HIGH, Down=LOW). Tell me if you want it flipped.
    if (cmd == 'U')      digitalWrite(Y_DIR, HIGH);
    else if (cmd == 'D') digitalWrite(Y_DIR, LOW);
    // 'S' stops pulses only
  }
}

void loop() {
  // ---- read 2-byte commands: XR, XL, XS, YU, YD, YS ----
  while (Serial.available() >= 2) {
    char a = Serial.read();      // axis 'X' or 'Y'
    char c = Serial.read();      // command letter
    if ((a=='X' && (c=='R'||c=='L'||c=='S')) ||
        (a=='Y' && (c=='U'||c=='D'||c=='S'))) {
      handleCommand(a, c);
    }
    // optional: consume \n or \r if the sender appends them
    if (Serial.peek() == '\n' || Serial.peek() == '\r') Serial.read();
  }

  // ---- X pulse generator ----
  if (modeX != 'S') {
    unsigned long now = micros();
    if (now - tX >= STEP_US_X) {
      tX = now;
      lvlX = !lvlX;
      digitalWrite(X_STEP, lvlX ? HIGH : LOW);
    }
  } else if (lvlX) {  // ensure low when stopped
    lvlX = false;
    digitalWrite(X_STEP, LOW);
  }

  // ---- Y pulse generator ----
  if (modeY != 'S') {
    unsigned long now = micros();
    if (now - tY >= STEP_US_Y) {
      tY = now;
      lvlY = !lvlY;
      digitalWrite(Y_STEP, lvlY ? HIGH : LOW);
    }
  } else if (lvlY) {  // ensure low when stopped
    lvlY = false;
    digitalWrite(Y_STEP, LOW);
  }
}
