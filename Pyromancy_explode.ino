/*
The popper is an air cylinder that bangs against the bottom of a 55 gallon drum.
The strobe is an emergency strobe light, like what you would see atop a forklift.
The spotlight is a shop light that illuminates the full display. This relay is a NC relay,
so the controls for it are opposite of all other relays - when the command is to turn it on,
it actually turns the spotlight off.
The siren is an air raid siren that is 3-D printed then mounted to a bench grinder.
The pyro is a flame thrower that shoots a ball of flame upward into the air above 
viewer's heads.

A 5 minute timer triggers the event, and the popper randomly activates at infrequent intervals.
Then the strobe turns on and the popper randomly activates progressively more frequently.
The spotlight begins to flicker along with the more frantic popping.
After so many rounds of random popping, the air raid siren turns on for a specific period of time, 
during which the popper continues to frantically operate and the spotlight continues to flicker.
This other activity also continues for the preset length of time the air raid siren spins down too.
Finally, all of the popping stops, the strobe and spotlight turn off, and the pyro solenoid is opened
for an adequate length of time for a sizable fireball to shoot into the air.
Five seconds after the fireball, the show resets, and the timer begins before the next show.

The first show starts not long after the Arduino is powered up.

Between shows, the popper, strobe, spotlight, and pyro can be controlled by a separate set of
intermittent push buttons.
*/

// Global constants:
const long eventInterval = 300000;
const int outPin = 5;   // the relevant output pins are all 5 pins lower in number from the input pins for each object.

const int sirenRelay = 2;
const int pyroRelay = 3;
const int strobeRelay = 4;
const int popperRelay = 5;
const int spotlightRelay = 6;

const int pyroButton = 8;
const int strobeButton = 9;
const int popperButton = 10;
const int spotlightButton = 11;

const int popperTime = 45;    // air cyclinder takes this length of time to fully extend and contract.
const int sirenOnTime = 7500;
const int SirenWindDownTime = 4500;
const int pyroOnTime = 750;
const int on = LOW;
const int off = HIGH;

long nextShowTime = 0;
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
    nextShowTime = (millis() + 30000);  // I want the first show time to trigger 30 seconds after the Arduino is plugged in.
}

void loop() {
    long currentTime = millis();
    if (currentTime >= nextShowTime) {
        pyroShow();
        currentTime = millis();
        nextShowTime = (currentTime + eventInterval);
    }
    for(int i=8 ; i<=11 ; i++) {
        checkPush (i);
    }
}

void checkPush(int pinNumber) {
  int pushed = digitalRead(pinNumber);  // read input value
  if (pushed == HIGH) // check if the input is HIGH (button released)
    digitalWrite((pinNumber - outPin), LOW);  // turn relevant relay OFF
  else  // button has been pressed
    digitalWrite((pinNumber - outPin), HIGH);  // turn relevant relay ON
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
        if (( (d/3) % 2) == 0) {
            if (cycle == 0)
                cycle = 1;
            else
                cycle = 0;
            digitalWrite(spotlightRelay, cycle);
        }
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
        if ((randTime % 2) == 0) {
            if (cycle == 0)
                cycle = 1;
            else
                cycle = 0;
            digitalWrite(spotlightRelay, cycle);
        }
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
        if ((randTime % 2) == 0) {
            if (cycle == 0)
                cycle = 1;
            else
                cycle = 0;
            digitalWrite(spotlightRelay, cycle);
        }
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
