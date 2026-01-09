// === 气泵 + 布料开关控制程序（呼气时间与抽气时间可单独设置） ===

// 引脚定义
#define E1 10
#define E2 11
#define VALVE 4
#define TOUCH_PIN 2   // 布料一端接 D2，另一端接 GND（使用内部上拉）

// === 时间控制参数（可按需调整） ===
const unsigned long exhaleTime = 7000UL; // 呼气时间，示例：7000 ms = 7 秒（更长）
const unsigned long inhaleTime = 3000UL; // 抽气时间，示例：3000 ms = 3 秒（更短）

// 触摸去抖时间
const unsigned long debounceDelay = 60UL;   // 触摸去抖（ms）

// 状态变量
bool running = false;           // 当前是否在执行那次完整循环
int phase = 0;                  // 0 = 出气相 (E1), 1 = 抽气相 (E2)
unsigned long phaseStartMs = 0; // 本相开始时间

// 触摸去抖 / 原始值追踪
bool lastRawTouch = HIGH;       // 上一次原始读取值
unsigned long lastChangeMs = 0; // 上次原始变化时间
bool stableTouch = HIGH;        // 稳定（去抖后）的触摸值
bool lastStablePrev = HIGH;     // 上一次稳定状态（用于边沿检测）

void setup() {
  pinMode(E1, OUTPUT);
  pinMode(E2, OUTPUT);
  pinMode(VALVE, OUTPUT);
  pinMode(TOUCH_PIN, INPUT_PULLUP); // 使用内部上拉：未触碰 = HIGH，触碰 = LOW

  stopAll(); // 初始全部停止
}

void loop() {
  // ---------- 1) 读取触摸原始值并做去抖 ----------
  bool raw = digitalRead(TOUCH_PIN); // LOW = 碰到, HIGH = 未碰
  if (raw != lastRawTouch) {
    lastChangeMs = millis();
    lastRawTouch = raw;
  }
  if (millis() - lastChangeMs > debounceDelay) {
    stableTouch = raw;
  }

  // ---------- 2) 若当前不在运行，检测下降沿触发一次完整循环 ----------
  if (!running) {
    if (lastStablePrev == HIGH && stableTouch == LOW) {
      // 按下触发一次完整循环
      running = true;
      phase = 0;
      phaseStartMs = millis();
    }
  }
  lastStablePrev = stableTouch;

  // ---------- 3) 在 running 时根据相位与时间执行（非阻塞） ----------
  if (running) {
    unsigned long now = millis();

    if (phase == 0) {
      // 出气相（呼气）
      digitalWrite(VALVE, HIGH);
      analogWrite(E1, 200); // E1 输出占空比（可根据需要调整 0-255）
      analogWrite(E2, 0);
      // 是否该切换到抽气相？
      if (now - phaseStartMs >= exhaleTime) {
        phase = 1;
        phaseStartMs = now;
      }
    } else if (phase == 1) {
      // 抽气相（吸气）
      digitalWrite(VALVE, LOW);
      analogWrite(E1, 0);
      analogWrite(E2, 200); // E2 输出占空比（可调整）
      // 是否该结束整个循环？
      if (now - phaseStartMs >= inhaleTime) {
        running = false;
        stopAll();
      }
    }
  } else {
    // 非运行期间确保全部停止
    stopAll();
  }

  delay(10); // 小延时减轻 CPU 占用（不影响响应）
}

void stopAll() {
  digitalWrite(VALVE, LOW);
  analogWrite(E1, 0);
  analogWrite(E2, 0);
}
