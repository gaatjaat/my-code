/*
This is just a simple timer that activates a sequence of events every 15 minutes:
When it triggers the event, it will first activate an air raid siren to run for
a preset length of time, then let the air raid siren spin down, and then
release a fireball.
*/

// Global constants:
const long eventInterval = 300000;
const int outPin = 5;

const int sirenRelay = 2;
const int pyroRelay = 3;
const int strobeRelay = 4;
const int popperRelay = 5;
const int spotlightRelay = 6;

const int pyroButton = 8;
const int strobeButton = 9;
const int popperButton = 10;
const int spotlightButton = 11;

const int popperTime = 40;
const int sirenOnTime = 7500;
const int SirenWindDownTime = 4500;
const int pyroOnTime = 750;
const int on = LOW;
const int off = HIGH;

// int buttonState = 0;  // variable for reading the pushbutton status
long showTime = millis();
long nextShowTime = showTime;

bool cycle = 0;
int randQuantity = 0;
int randTime = 0;

void setup() {
    pinMode(sirenRelay, OUTPUT);
    pinMode(pyroRelay, OUTPUT);
    pinMode(strobeRelay, OUTPUT);
    pinMode(popperRelay, OUTPUT);
    pinMode(spotlightRelay, OUTPUT);
    pinMode(pyroButton, INPUT);
    pinMode(strobeButton, INPUT);
    pinMode(popperButton, INPUT);
    pinMode(spotlightButton, INPUT);
    digitalWrite(sirenRelay, off);
    digitalWrite(pyroRelay, off);
    digitalWrite(strobeRelay, off);
    digitalWrite(popperRelay, off);
    digitalWrite(spotlightRelay, off); 
}

void loop() {
    if (millis() >= nextShowTime) {
        pyroShow();
        showTime = millis();
        nextShowTime = showTime + eventInterval;
    }
    for(int i=8 ; i<=11 ; i++) {
        checkPush (i);
    }
}

void checkPush(int pinNumber){
  int pushed = digitalRead(pinNumber);  // read input value
  if (pushed == HIGH) // check if the input is HIGH (button released)
    digitalWrite((pinNumber - outPin), LOW);  // turn LED OFF
  else
    digitalWrite((pinNumber - outPin), HIGH);  // turn LED ON
}

void pyroShow() {
    randQuantity = random(5, 20);
    for(int a = 1; a <= randQuantity ; a++) {
        randTime = (random(1, 45) * 75);
        digitalWrite(popperRelay, on);
        delay(popperTime);
        digitalWrite(popperRelay, off);
        delay(popperTime);
        delay(randTime);
    }

    digitalWrite(strobeRelay, on);
    randQuantity = random(5, 20);
    for(int b = 1; b <= randQuantity ; b++) {
        randTime = (random(1, 45) * 25);
        digitalWrite(popperRelay, on);
        delay(popperTime);
        digitalWrite(popperRelay, off);
        delay(popperTime);
        delay(randTime);
    }

    randQuantity = random(15, 40);
    for(int c=1; c<=randQuantity ; c++) {
        randTime = (random(1, 45) * 10);
        digitalWrite(popperRelay, on);
        delay(popperTime);
        digitalWrite(popperRelay, off);
        delay(popperTime);
        delay(randTime);
    }

    randQuantity = random(25, 70);
    for(int d=1; d<=randQuantity ; d++) {
        if (( (d/3) % 2) == 0)
            if (cycle == 0)
                cycle = 1;
            else
                cycle = 0;
            digitalWrite(spotlightRelay, cycle);
        randTime = (random(1, 45) * 5);
        digitalWrite(popperRelay, on);
        delay(popperTime);
        digitalWrite(popperRelay, off);
        delay(popperTime);
        delay(randTime);
    }
    digitalWrite(sirenRelay, on);   // Turn on the air raid siren
    long sirenStartTime = millis();
    while (millis() <= (sirenStartTime + sirenOnTime)) {
        randTime = (random(1, 20) * 5);
        if ((randTime % 2) == 0)
            if (cycle == 0)
                cycle = 1;
            else
                cycle = 0;
            digitalWrite(spotlightRelay, cycle);
        digitalWrite(popperRelay, on);
        delay(popperTime);
        digitalWrite(popperRelay, off);
        delay(popperTime);
        delay(randTime);
    }
    digitalWrite(sirenRelay, off);    // Turn the air raid siren off
    long sirenDownTime = millis();
    while (millis() <= sirenDownTime + SirenWindDownTime) {
        randTime = (random(1, 20) * 5);
        if ((randTime % 2) == 0)
            if (cycle == 0)
                cycle = 1;
            else
                cycle = 0;
            digitalWrite(spotlightRelay, cycle);
        digitalWrite(popperRelay, on);
        delay(popperTime);
        digitalWrite(popperRelay, off);
        delay(popperTime);
        delay(randTime);
    }
    digitalWrite(spotlightRelay, on);
    delay(400);
    digitalWrite(strobeRelay, off);
    digitalWrite(pyroRelay, on);    // Open the propane solenoid
    delay(pyroOnTime);                // Leave the solenoid open for this pre-identified length of time
    digitalWrite(pyroRelay, off);     // Close the propane solenoid
    delay(5000);
    digitalWrite(spotlightRelay, off);
}