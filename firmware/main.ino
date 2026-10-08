const uint8_t btnPins[4] = {4, 5, 6, 7};
const uint8_t ledPins[4] = {15, 16, 17, 18};

int score, lives, mole = -1;
uint32_t moleStart, nextSpawn, moleTime;
bool playing = false;

int readButtons() {
  static bool last[4];
  static uint32_t t[4];
  int r = -1;
  for (int i = 0; i < 4; i++) {
    bool p = !digitalRead(btnPins[i]);
    if (p && !last[i] && millis() - t[i] > 40) { r = i; t[i] = millis(); }
    last[i] = p;
  }
  return r;
}

void setAll(bool on) { for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], on); }

void blinkAll(int times, int ms) {
  for (int i = 0; i < times; i++) {
    setAll(true);  delay(ms);
    setAll(false); delay(ms);
  }
}

void startGame() {
  score = 0; lives = 3; mole = -1;
  nextSpawn = millis() + 800;
  playing = true;
  Serial.println("Game start! Lives: 3");
}

void loseLife() {
  lives--;
  mole = -1;
  setAll(false);
  blinkAll(2, 100);                      // miss signal
  Serial.printf("Missed! Lives left: %d\n", lives);
  nextSpawn = millis() + random(400, 900);
  if (lives <= 0) gameOver();
}

void gameOver() {
  playing = false;
  Serial.printf("GAME OVER. Score: %d\n", score);
  blinkAll(3, 400);                      // long blinks = game over
  delay(800);
  blinkAll(score, 150);                  // short blinks = your score
  delay(500);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }
  randomSeed(esp_random());
  // Startup test: each LED lights in turn
  for (int i = 0; i < 4; i++) { digitalWrite(ledPins[i], HIGH); delay(200); digitalWrite(ledPins[i], LOW); }
}

void loop() {
  int b = readButtons();

  if (!playing) {
    // Idle: chase animation, press any button to start
    int step = (millis() / 200) % 4;
    for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], i == step);
    if (b >= 0) { setAll(false); delay(300); startGame(); }
    return;
  }

  // Spawn a mole
  if (mole < 0 && millis() >= nextSpawn) {
    mole = random(4);
    moleStart = millis();
    moleTime = max(350, 1200 - score * 35);   // gets faster
    digitalWrite(ledPins[mole], HIGH);
  }

  if (b >= 0) {
    if (mole >= 0 && b == mole) {
      score++;
      digitalWrite(ledPins[mole], LOW);
      mole = -1;
      nextSpawn = millis() + random(250, 700);
      Serial.printf("Hit! Score: %d\n", score);
    } else {
      loseLife();
    }
  } else if (mole >= 0 && millis() - moleStart > moleTime) {
    loseLife();                               // too slow
  }
}