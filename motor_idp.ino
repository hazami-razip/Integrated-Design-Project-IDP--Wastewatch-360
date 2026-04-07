// Motor control pins
const int R_EN = 7;
const int L_EN = 8;
const int RPWM = 5;  // PWM
const int LPWM = 6;  // PWM

// Ultrasonic sensor pins
const int trigPin = 9;
const int echoPin = 10;

// Manual override button and LED
const int manualButtonPin = 2;
const int manualLedPin = 3;

// Motor speed
const int motorSpeed = 200;

// Distance threshold
const int openThreshold = 15;

// Timing
const int fullOpenTime = 2500;  
const int fullCloseTime = 5000;

// Debounce
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

bool lidOpen = false;
bool manualOverride = false;
int lastButtonState = HIGH;
int currentButtonState = HIGH;

void setup() {
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(manualButtonPin, INPUT_PULLUP);
  pinMode(manualLedPin, OUTPUT);
  
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  digitalWrite(manualLedPin, LOW);

  Serial.begin(9600);
  Serial.println("System started in AUTO mode.");
}

void loop() {
  int reading = digitalRead(manualButtonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && currentButtonState == HIGH) {
      manualOverride = !manualOverride;

      if (manualOverride) {
        digitalWrite(manualLedPin, HIGH);
        analogWrite(LPWM, motorSpeed);
        analogWrite(RPWM, 0);
        delay(3500);
        stopMotor();
        lidOpen = true;
        Serial.println(">> MANUAL MODE: Lid opened");
      } else {
        digitalWrite(manualLedPin, LOW);
         analogWrite(LPWM, 0);
         analogWrite(RPWM, 100);
         delay(7000);
         stopMotor();
        lidOpen = false;
        Serial.println(">> MANUAL MODE: Lid closed");
      }
    }
    currentButtonState = reading;
  }
  lastButtonState = reading;

  // If manual override, skip ultrasonic control
  if (manualOverride) return;

  // AUTO MODE
  long distance = getDistance();
  if (distance > 0 && distance < openThreshold && !lidOpen) {
    openLid();
    lidOpen = true;
    Serial.println("AUTO: Lid opened");

    delay(7000);  // Keep open

    closeLid();
    lidOpen = false;
    Serial.println("AUTO: Lid closed");
  }

  delay(200);
}

void openLid() {
  analogWrite(LPWM, motorSpeed);
  analogWrite(RPWM, 0);
  delay(fullOpenTime);
  stopMotor();
}

void closeLid() {
  analogWrite(LPWM, 0);
  analogWrite(RPWM, 100);
  delay(fullCloseTime);
  stopMotor();
}

void stopMotor() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;

  long distance = duration * 0.034 / 2;
  return distance;
}