/*
This is just a simple timer that activates a sequence of events every 15 minutes:
When it triggers the event, it will first activate an air raid siren to run for
a preset length of time, then let the air raid siren spin down, and then
release a fireball.
*/

// Global constants:
long eventInterval = 900000;
int sirenRelay = 2;
int pyroRelay = 3;
int sirenOnTime = 7500;
int SirenWindDownTime = 4500;
int pyroOnTime = 1200;
int on = LOW;
int off = HIGH;

void setup() {
    pinMode(sirenRelay, OUTPUT);
    pinMode(pyroRelay, OUTPUT);
    digitalWrite(sirenRelay, off);
    digitalWrite(pyroRelay, off);
}

void loop() {
    digitalWrite(sirenRelay, on);   // Turn on the air raid siren
    delay(sirenOnTime);               // Stay on for this pre-identified length of time
    digitalWrite(sirenRelay, off);    // Turn the air raid siren off
    delay(SirenWindDownTime);         // Wait for the siren to spin down most of the way
    
    digitalWrite(pyroRelay, on);    // Open the propane solenoid
    delay(pyroOnTime);                // Leave the solenoid open for this pre-identified length of time
    digitalWrite(pyroRelay, off);     // Close the propane solenoid
    delay(eventInterval);             // Let the pre-identified length of time pass. When initially coded, this was every 15 minutes.
}
