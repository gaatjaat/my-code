/*
This is just a simple timer that activates a sequence of events every 15 minutes:
When it triggers the event, it will first activate an air raid siren to run for
a preset length of time, then let the air raid siren spin down, and then
release a fireball.
*/

// Global constants:
long eventInterval = 300000;
int sirenRelay = 3;
int pyroRelay = 4;
int sirenOnTime = 6000;
int SirenWindDownTime = 3000;
int pyroOnTime = 1500;

void setup() {
    pinMode(sirenRelay, OUTPUT);
    pinMode(pyroRelay, OUTPUT);
    digitalWrite(sirenRelay, LOW);
    digitalWrite(pyroRelay, LOW);
}

void loop() {
    digitalWrite(sirenRelay, HIGH);   // Turn on the air raid siren
    delay(sirenOnTime);               // Stay on for this pre-identified length of time
    digitalWrite(sirenRelay, LOW);    // Turn the air raid siren off
    delay(SirenWindDownTime);         // Wait for the siren to spin down most of the way
    
    digitalWrite(pyroRelay, HIGH);    // Open the propane solenoid
    delay(pyroOnTime);                // Leave the solenoid open for this pre-identified length of time
    digitalWrite(pyroRelay, LOW);     // Close the propane solenoid
    delay(eventInterval);             // Let the pre-identified length of time pass. When initially coded, this was every 15 minutes.
}
