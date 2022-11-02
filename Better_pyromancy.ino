/*
This is a timer that activates a sequence of events (making up an entire show) 
based on a frequency, depending on how that event is set up:
    When it triggers each event, it will first activate the first pin to run for
    a preset length of time, turn off that pin and wait until another preset time passes, and then
    turn on the next identified pin, and repeat each of the above as identified in an array of values. 
    That specific show will then hibernate until the occurrence frequency time is once again reached.
Show Array legend:
    first value: show ID #
    second value: show frequency in milliseconds
    third value: length of time after show starts before turning first pin on
    fourth value: pin to set to HIGH for this event in the show
    fifth value: pin to set to LOW for this event in the show
    sixth value: length of time to keep first pin on before turning it off
    subsequent series of four values: repeat values 3-6 but for different events (pin combinations).

    
There is also a Show Name Array that is a character array containing the show name for each show organized by the id #
*/

// Includes:
#include "Arduino.h"

// Global constants:
long eventInterval = 420000;
int sirenRelay = 2;
int pyroRelay = 3;
int buttonPin = 4;
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
