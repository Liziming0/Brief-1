// === Air Pump Control with Fabric Touch Switch ===
// Function:
// Press fabric once -> EXHALE (long) -> INHALE (short) -> STOP
// Press again to repeat one full cycle

// ---------------- Pin Definitions ----------------
#define E1 10        // Pump 1 (exhale)
#define E2 11        // Pump 2 (inhale)
#define VALVE 4      // Valve control
#define TOUCH_PIN 2  // Fabric switch: one fabric to D2, the other to GND

// ---------------- Time Settings ------------------
const unsigned long exhaleTime = 7000UL; // Exhale duration (ms) - LONGER
const unsigned long inhaleTime = 3000UL; // Inhale duration (ms) - SHORTER
const unsigned long debounceDelay = 60UL; // Touch debounce time (ms)

// ---------------- State Variables ----------------
bool running = false;            // Is a cycle currently running?
int phase = 0;                   // 0 = exhale, 1 = inhale
unsigned long phaseStartMs = 0;  // Start time of current phase

// Touch debounce variables
bool lastRawTouch = HIGH;
unsigned long lastChangeMs = 0;
bool stableTouch = HIGH;
bool lastStablePrev = HIGH;

// ---------------- Setup --------------------------
void setup() {
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);
  pinMode(VALVE, OUTPUT);

  pinMode(TOUCH_PIN, INPUT_PULLUP); // Not touched = HIGH, touched = LOW

  stopAll(); // Ensure everything is off at startup
}

// ---------------- Main Loop ----------------------
void loop() {
  // 1) Read fabric touch and debounce
  bool raw = digitalRead(TOUCH_PIN);
  if (raw != lastRawTouch) {
    lastChangeMs = millis();
    lastRawTouch = raw;
  }
  if (millis() - lastChangeMs > debounceDelay) {
    stableTouch = raw;
  }

  // 2) Detect press (HIGH -> LOW) when not running
  if (!running) {
    if (lastStablePrev == HIGH && stableTouch == LOW) {
      running = true;
      phase = 0;                 // Start with exhale
      phaseStartMs = millis();
    }
  }
  lastStablePrev = stableTouch;

  // 3) Run one full exhale + inhale cycle
  if (running) {
    unsigned long now = millis();

    if (phase == 0) {
      // ---- EXHALE ----
      digitalWrite(VALVE, HIGH);
      analogWrite(E1, 200);
      analogWrite(E2, 0);

      if (now - phaseStartMs >= exhaleTime) {
        phase = 1;
        phaseStartMs = now;
      }
    }
    else if (phase == 1) {
      // ---- INHALE ----
      digitalWrite(VALVE, LOW);
      analogWrite(E1, 0);
      analogWrite(E2, 200);

      if (now - phaseStartMs >= inhaleTime) {
        running = false;
        stopAll(); // End of one full cycle
      }
    }
  }
  else {
    stopAll();
  }

  delay(10); // Small delay to reduce CPU usage
}

// ---------------- Helper Function ----------------
void stopAll() {
  digitalWrite(VALVE, LOW);
  analogWrite(E1, 0);
  analogWrite(E2, 0);
}
