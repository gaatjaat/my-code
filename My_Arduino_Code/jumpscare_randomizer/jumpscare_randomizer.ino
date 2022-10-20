/*
  jumpscare_randomizer.ino

  Caters to board: Arduino UNO (or equivalent)

  Upon initial startup, a hardware function test is ran. The next three modes iterate through 
  the different lasers, to help lign them up. When the laser is aligned the relevant light lights up, 
  along with the LED on the board. After the laser has remained ligned up for five consecutive seconds, 
  the relevant light flashes on and off twice, saying alignment on that tripwire is complete. 
  The program then automatically proceeds to the next mode.
  Then once the button is pressed, the program advances through these modes:
    ooga_align
      Relevant horn beeps two short beeps to indicate that it is in this mode for this event's tripwire alignment.
      while in this mode, if the relevant laser is lined up (the difference in analog signal between the tripwire sensor 
      and the ambient light sensor is greater than a specific threshold), the LED and the matching light turns on. 
      Then, once that tripwire has been lined up for 5 seconds straight, flash the matching light twice
      to indicate alignment on that tripwire is complete.
      The program then automatically proceeds to the next mode.
    car_align
      Ditto ooga align
    train_align
      Ditto ooga align
    test
      Listen to all tripwires
      When any one of the tripwires is tripped, run the tripped flow from within the jumpscare class, as per details below.
    armed
      See details below
    trip
      When called from either test or armed modes, run the relevant event for the tripwire that was tripped (lights and horn).
      Have a timer to keep track of how long a tripwire is tripped, and proceed through the warn and exit_notify modes accordingly
    warn
      After a tripwire has been tripped for x time, start beeping the relevant horn at x interval.
    exit_notify 
      Once the warn mode has been active for x time, medium-length beep each horn in sequence, then reset back to align.

  For both test and armed modes: If at any time the system notices that a tripwire has been blocked or misaligned 
  (tripwire photoresistor reading values lower than pre-identified threshold above for longer than 3 seconds),
  then the system first enters the warn mode (10s), and eventually the exit_notify mode, and finally back to the align modes.
 
  Armed mode:
  Randomly selects one of several laser tripwires (pins A1-A3) to listen to, 
  then once it is identified that the active laser tripwire has been tripped 
  (the amount of light being collected in the photoresistor drops significantly),
  the jumpscare at the same location as that tripwire is activated. 
  After a set time (currently 2 seconds), that jumpscare turns off, --not yet implemented
  and the program is looped back around to the start.
  
  **Special Note: There is a small delay between activating the two banks of train horns
  to prevent the motor startup amperage draw from overloading the transformer.
  
  The circuit:
  - See schematics in attached image file.

  created 2022
  by gaatjaat
*/

// Includes
#include "Arduino.h"

// Variables
int mode = 0;
int ambiant;
int trip = 1000;              // The light value I get when the laser is aligned. Initial value is intended to be plenty high above ambient light value
const int minLight = 900;     // To identify that the laser is aligned, the tripwire sensor needs to give a light value above this value
int atAverage;
const long onMillis = 600;
const long offMillis = 2000;
const int maxTime = 3000;     // a reasonable maximum allowable length of time to allow the tripwire to be tripped
const int exceedTime = 10000; // If the tripwire has been tripped for over this time, it can be reasonably assumed that either the laser is misaligned, or there is an obstruction.
unsigned long randomizerTime = 0;
int trippedWire;
int relayOff = HIGH;
int relayOn = LOW;

// Output Pins
const int ledPin = 3;
const int oogaLightPin = 8;
const int carLightPin = 9;
const int trainLightPin = 10;
const int oogaHornPin = 4;
const int carHornPin = 5;
const int trainHornAPin = 6;
const int trainHornBPin = 7;
const char* eventNames[] = {"ooga", "car", "train"};
const char*  modeNames[] = {"FUNCTION_TEST", "ALIGN_OOGA", "ALIGN_CAR", "ALIGN_TRAIN", "TEST", "ARMED", "EXIT_NOTIFY"};

// Input Pins
const int modePin = 2; 
const int ambiantPin = A0;
const int oogaTripPin = A1;
const int carTripPin = A2;
const int trainTripPin = A3;


class Jumpscare
{
  protected:
    // Class Member Variables
    // These are initialized at startup
    int tripPin;        // the tripwire sensor pin number 
    int lightPin;       // the light relay pin number
    int hornPinA;       // the bank A horn relay pin number
    int hornPinB;       // the bank B horn relay pin number
    const char* eventName;   // the event's name
    int eventId;        // event ID that provides little additional info on the event
    long timerMillis;     // timer for how long this event has been going
    long previousMillis;
    bool aligning;
    bool isEnabled;
    bool proceed;
    int myTrip;
    bool tripped;
    int lightState;     // lightState used to set the light
    int hornState;      // hornState used to set the horn

  public:
    // Constructor - creates a Jumpscare
    // and initializes the member variables and state
    Jumpscare(const char* eventIs, int id, int trippin, int light, int hornA, int hornB){
      eventName = eventIs;
      eventId = id;
      tripPin = trippin;
      lightPin = light;
      pinMode(lightPin, OUTPUT);
      hornPinA = hornA;
      pinMode(hornPinA, OUTPUT);
      hornPinB = hornB;
      pinMode(hornPinB, OUTPUT);
      aligning = false;
      isEnabled = false;
      lightState = relayOff;
      hornState = relayOff;
      digitalWrite(hornPinA, hornState);
      digitalWrite(hornPinB, hornState);
      digitalWrite(lightPin, lightState);
    }

    int GetHornState(){
      return hornState;
    }
    
    void HornOn(){
      Serial.print("The current hornState is: ");
      Serial.print(eventName);
      Serial.print(": ");
      Serial.println(hornState);
      if (hornState == relayOff){
        hornState = relayOn;  // Turn it on
        digitalWrite(hornPinA, hornState);  // Update the horn
        delay(220); // Train horn has two banks, and there needs to be a small delay to account for startup amperage draw
        digitalWrite(hornPinB, hornState);  // Update the horn
      }
    }
    
    void HornOff(){
      hornState = relayOff;  // Turn it off
      digitalWrite(hornPinA, hornState);  // Update the horn
      digitalWrite(hornPinB, hornState);  // Update the horn
    }
    
    int GetLightState(){
      return lightState;
    }
    
    void LightOn(){
      Serial.print("The current lightState is: ");
      Serial.print(eventName);
      Serial.print(": ");
      Serial.println(lightState);
      if (lightState == relayOff) {
        lightState = relayOn;  // Turn it on
        digitalWrite(lightPin, lightState);  // Update the light
      }
    }
    
    void LightOff(){
      lightState = relayOff;  // Turn it off
      digitalWrite(lightPin, lightState);  // Update the light
    }
    
    bool CalibrateTrip(){
      ambiant = analogRead(ambiantPin);
      trip = analogRead(tripPin);
      atAverage = ambiant + ((trip - ambiant)/2);
      return (trip >= minLight);
    }
    
    bool CheckTrip(){
      ambiant = analogRead(ambiantPin);
      myTrip = analogRead(tripPin);
      atAverage = ambiant + ((trip - ambiant)/2);
      if (myTrip  < atAverage) {
        Serial.print("Wire tripped:");
        Serial.print(eventName);
      }
      return (myTrip < atAverage);
    }

    void Enabled(bool enabl){
      isEnabled = enabl;
    }
    
    void TimerReset(){
      timerMillis = millis();
    }
    
    bool MaxLength(){
      return ((millis() - timerMillis) >= maxTime);
    }
    
    bool Obstructed(){
      return ((millis() - timerMillis) >= exceedTime);
    }
    
    int GetId(){
      return eventId;
    }

    void HardwareTest(){
      this->HornOn();
      delay(700);
      this->HornOff();
      delay(1000);
      this->LightOn();
      delay(700);
      this->LightOff();
      delay(1000);
    }

    void AlignJumpscare() {
      if (!aligning) {
        this->HornOn();
        delay(200);
        this->HornOff();
        delay(200);
        this->HornOn();
        delay(200);
        this->HornOff();
        aligning = true;
      }
      if (this->CalibrateTrip()) {
        this->LightOn();
        digitalWrite(ledPin,HIGH);
        
      } else {
        digitalWrite(ledPin,LOW);
        this->LightOff();
        this->TimerReset();
      }
      
      if (this->MaxLength()){
        digitalWrite(ledPin,LOW);
        this->LightOff();
        delay(500);
        this->LightOn();
        delay(500);
        this->LightOff();
        delay(500);
        this->LightOn();
        delay(500);
        this->LightOff();
        delay(1000);
        mode++;
        aligning = false;
      }
    }

    void Arm(){
      if (this->CheckTrip()) {
        if (!tripped){
          tripped = true;
          trippedWire = this->GetId();
          Serial.print("Jumpscare that is tripped:");
          Serial.print(trippedWire);
          Serial.println();
        }
        if (mode == 4 || (mode == 5 && isEnabled)){
          Serial.println("This is either mode 4 or it is mode 5 AND this tripwire is enabled.");
          proceed = true;
        }	
        if (GetHornState() == relayOff && proceed == true){
          this->HornOn();
        }
        if (GetLightState() == relayOff && proceed == true){
          this->LightOn();
        }
        if (this->MaxLength()) {
          Serial.println("The current tripwire has been tripped for over the maxTime limit.");
          Serial.println("You should now be hearing some beeps");
          previousMillis = millis();
          if (hornState == relayOn) {
            if ((millis() - previousMillis) >= onMillis) {
              this->HornOff();
              previousMillis = millis();
            }
          } else {
            if ((millis() - previousMillis) >= offMillis) {
              this->HornOn();
              previousMillis = millis();
            }
          }			
        }
        if (this->Obstructed()) {
          Serial.println("The current tripwire has been tripped for over the exceedTime limit.");
          Serial.println("You are now being returned to align the tripwires");
          delay(1000);
          tripped = false;
          this->ResetAll();
          mode = 6;
        }
      } else {	
        if (tripped){
          tripped = false;
          Serial.println("The tripwire is no longer tripped.");
          this->ResetAll();
        }
      }
    }

    void ResetAll(){
      this->TimerReset();
      this->HornOff();
      this->LightOff();
      previousMillis = 0;
    }
};

Jumpscare ooga(eventNames[0], 0, oogaTripPin, oogaLightPin, oogaHornPin, oogaHornPin);
Jumpscare car(eventNames[1], 1, carTripPin, carLightPin, carHornPin, carHornPin);
Jumpscare train(eventNames[2], 2, trainTripPin, trainLightPin, trainHornAPin, trainHornBPin);

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(modePin, INPUT_PULLUP);
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  Serial.println("start");
  randomizerTime = millis(); //set the randomizer timer
}

void loop() {
   switch (mode) {
    case 0: //Hardware functionality test Mode
      stats();
      //turn on each relay in sequence, to verify functionality
      digitalWrite(ledPin,HIGH);
      if (digitalRead(modePin) == LOW) {
				Serial.println("The button has been pressed.");
        digitalWrite(ledPin,LOW);
        delay(1000);
        ooga.HardwareTest();
        car.HardwareTest();
        train.HardwareTest();
        delay(1000);
        mode++;
      }
    break;
    
    case 1: //Ooga tripwire align notify mode
      stats();
      ooga.AlignJumpscare();
    break;

    case 2: //Car tripwire align notify mode
      stats();
      car.AlignJumpscare();
    break;

    case 3: //Train tripwire align notify mode
      stats();
      train.AlignJumpscare();
    break;

    case 4: //test all jumpscares Mode
      ooga.Arm();
      car.Arm();
      train.Arm();
      if (digitalRead(modePin) == LOW) {
				//Serial.println("the button has been pressed.");
				car.HornOn();
				delay(300);
				car.HornOff();
				delay(1000);
        mode++;
      }
      stats();
    break;

    case 5: // Armed mode
      stats();
      if ((randomizerTime + 5000) < millis()) {
        randomSeed(analogRead(A5));
        int randomNum = random(0, 3);
        Serial.print("Current random number:");
        Serial.print(randomNum);
        Serial.println();
        ooga.Enabled(randomNum == 0);
        car.Enabled(randomNum == 1);
        train.Enabled(randomNum == 2);
        randomizerTime = millis(); //set the randomizer timer
      }
      ooga.Arm();
      car.Arm();
      train.Arm();
    break;

    case 6: //Exit_notify Mode
      stats();
      mode = 1;
      ooga.HornOn();
      delay(700);
      car.HornOn();
      delay(700);
      train.HornOn();
      delay(1000);
      ooga.ResetAll();
      car.ResetAll();
      train.ResetAll();
      delay(2000);
    break;
    }
  delay(1);                       // wait for a bit
}

//Writes stats to the Serial Port
void stats() {
  Serial.print("A:");
  Serial.print(ambiant);
  Serial.println();
  Serial.print(" T:");
  Serial.print(trip);
  Serial.println();
  Serial.print(" AT:");
  Serial.print(atAverage);
  Serial.println();
  Serial.print(" MODE:");
  Serial.print(modeNames[mode]);
  Serial.println();
  Serial.print("Tripped wire:");
  Serial.print(trippedWire);
  Serial.println();
  Serial.println("");
}