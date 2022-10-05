/*
  jumpscare_randomizer.ino

  Written more specifically for board: Arduino UNO (or equivalent)

  Upon initial startup, the tripwires are all disarmed. The first modes iterate through 
  the different lasers, to help lign them up. When the laser is aligned the LED lights up 
  saying it is ready to proceed to the next mode.
  Then once the button is pressed, the program advances through these modes:
    ooga_align
      Relevant horn beeps two short beeps to indicate that it is in this mode for this event's tripwire alignment.
      while in this mode, if the relevant laser is lined up (the difference in analog signal between the tripwire sensor 
      and the ambient light sensor is greater than a specific threshold), light up the LED. 
      Then, once that tripwire has been lined up for 5 seconds straight, flash the matching light twice
      to indicate that you can proceed to the next mode.
    car_align
      Ditto ooga align
    train_align
      Ditto ooga align
    test
      Listen to all tripwires
      When one of the tripwires is tripped, call trip, feeding it the event name it needs to execute.
    armed
      See details below
    trip
      When called from either test or armed modes, run the relevant event for the tripwire that was tripped.
      Have a timer to keep track of how long a tripwire is tripped, and proceed through the warn and exit_notify modes accordingly
    warn
      After a tripwire has been tripped for x time, start beeping the relevant horn at x interval.
    exit_notify 
      Once the warn mode has been active for x time, medium-length beep each horn in sequence, then reset back to align.

  For both test and armed modes: If at any time the system notices that a tripwire has been blocked or misaligned 
  (tripwire photoresistor reading values lower than pre-identified threshold above for longer than 5 seconds),
  then the system first enters the warn mode (10s), and eventually the exit_notify mode, and finally back to the align modes.
 
  Armed mode:
  Randomly selects one of several laser tripwires (pins 7-9) to listen to, 
  then once it is identified that the active laser tripwire has been tripped 
  (the amount of light being collected in the photoresistor drops significantly),
  the jumpscare at the same location as that tripwire is activated. 
  After a set time (currently 1 second), that jumpscare turns off, 
  and the program is looped back around to the start.
  
  There is a small delay between activating the two banks of train horns
  to prevent the motor startup amperage draw from overloading the transformer.
  
  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground



***** personal notes --- To be removed or update comments above later
Mode flow:

**Always keep track of the previous mode. Don't update this variable while in warn mode,
except in the case complete obstruction/misalignment of any of the tripwires is detected, 
whereupon mode will reset to 0.
Keep track of how long the program has been in current mode 
(to use with alignments, tripped and warn test cases).
Record the time when any of the laser tripwires get tripped

The case is selected based on what value mode (global variable) is set to
Mode is iteratively incremented up to armed case via press of the button. 
When in align cases, button will only increment mode if the relevant tripwire sensor has 
observed the laser for the required consecutive timeframe.
In test and armed cases, mode will update to tripped any time the tripwire sensor 
detects an interruption to the tripwire signal
The tripped case will update mode to previous mode if the tripped length is below the 
run_time threshold time.
The tripped case will update mode to warn if the tripped length exceeds the run_time 
threshold time.
The warn case will update mode to previous mode if the tripped length is below the 
obstruction threshold time.
The warn case will reset mode to 0 if the tripped length exceeds the 
obstruction threshold time.



  created 2022
  by gaatjaat
*/

// Variables
int mode = 0;
int previousMode;
int ambiant;
int trip = 1000;              // The light value I get when the laser is aligned. Initial value is intended to be plenty high above ambient light value
const int minLight = 900;     // To identify that the laser is aligned, the tripwire sensor needs to give a light value above this value
int atAverage;
const long onMillis = 600;
const long offMillis = 2000;
const int maxTime = 5000;     // a reasonable maximum allowable length of time to allow the tripwire to be tripped
const int exceedTime = 10000; // If the tripwire has been tripped for over this time, it can be reasonably assumed that either the laser is misaligned, or there is an obstruction.
long randNumber;
bool randomized = false;

// Output Pins
const int ledPin = 13;
const int oogaRelayPin = 3;
const int carRelayPin = 4;
const int trainRelay1Pin = 5;
const int trainRelay2Pin = 6;
const String  modeNames[8] = {"FUNCTION_TEST","ALIGN_OOGA_NOTIFY","ALIGN_OOGA","ALIGN_CAR_NOTIFY","ALIGN_CAR","ALIGN_TRAIN_NOTIFY","ALIGN_TRAIN","TEST","ARMED","TRIPPED","WARN","EXIT_NOTIFY"};

// Input Pins
const int modePin = 4; 
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
    string eventName;   // the event's name
    int eventId;        // event ID that provides little additional info on the event
    long timerMillis;     // timer for how long this event has been going
    long previousMillis;
    bool aligning = false;
    int myMode;
    int beepMode = 0;
    bool warnReached = false;

    // These maintain the current state
    int lightState;     // lightState used to set the light
    int hornState;      // hornState used to set the horn

    // Constructor - creates a Jumpscare
    // and initializes the member variables and state
  public:
    
    Jumpscare(string eventIs, int id, int trippin, int light, int hornA, int hornB)
    {
      eventName = eventIs;
      eventId = id;
      tripPin = trippin;
      lightPin = light;
      pinMode(lightPin, OUTPUT);
      hornPinA = hornA;
      pinMode(hornPinA, OUTPUT);
      hornPinB = hornB;
      pinMode(hornPinB, OUTPUT);

      lightState = LOW;
      hornState = LOW;
    }

    bool IsWarned()
    {
      return warnReached;
    }
    
    void WarnReached()
    {
      warnReached = true;
    }
    
    int GetHornState()
    {
      return hornState;
    }
    
    void HornOn()
    {
      hornState = HIGH;  // Turn it on
      digitalWrite(hornPinA, hornState);  // Update the horn
      delay(220); // Train horn has two banks, and there needs to be a small delay to account for startup amperage draw
      digitalWrite(hornPinB, hornState);  // Update the horn
    }
    
    void HornOff()
    {
      hornState = LOW;  // Turn it off
      digitalWrite(hornPinA, hornState);  // Update the horn
      digitalWrite(hornPinB, hornState);  // Update the horn
      randomized = false;
    }
    
    int GetLightState()
    {
      return lightState;
    }
    
    void LightOn()
    {
      lightState = HIGH;  // Turn it on
      digitalWrite(lightPin, lightState);  // Update the light
    }
    
    void LightOff()
    {
      lightState = LOW;  // Turn it off
      digitalWrite(lightPin, lightState);  // Update the light
      randomized = false;
    }
    
    bool CalibrateTrip()
    {
      ambiant = analogRead(ambiantPin);
      trip = analogRead(tripPin);
      atAverage = ambiant + ((trip - ambiant)/2);
      stats();
      return (trip >= minLight);
    }
    
    bool CheckTrip()
    {
      ambiant = analogRead(ambiantPin);
      trip = analogRead(tripPin);
      atAverage = ambiant + ((trip - ambiant)/2);
      stats();
      if (trip  < atAverage) {
        Serial.print("Wire tripped:");
        Serial.print(eventName);
      }

      return (trip  < atAverage);
    }
    
    void TimerReset()
    {
      timerMillis = millis();
    }
    
    bool MaxLength()
    {
      return ((millis() - timerMillis) >= maxTime);
    }
    
    bool Obstructed()
    {
      return ((millis() - timerMillis) >= exceedTime);
    }
    
    int GetId()
    {
      return eventId;
    }
    
     void BeepHorn()
    {
      switch (beepMode) {
        case 0:  //setup for beeping. This should only run once each time the system enters this state.
          beepMode++;
          this.HornOn();
          previousMillis = millis();
        break;
        
        case1:
          if (hornState == LOW) {
            beepMode = 2;
          } else {
            beepMode = 3;
          }
        break;
        
        case 2:
          if ((millis() - previousMillis) >= onMillis) {
            this.HornOff();
            beepMode = 1;
            previousMillis = millis();
          }
        break;
        
        case 3:
          if ((millis() - previousMillis) >= offMillis) {
            this.HornOn();
            beepMode = 1;
            previousMillis = millis();
          }
        break;
      }
    }

    void hardwareTest(){
      this.HornOn();
      delay(700);
      this.HornOff();
      delay(700);
      this.LightOn();
      delay(700);
      this.LightOff();
      delay(700);
    }

    void alignJumpscare() {
      if (!aligning) {
        this.HornOn();
        delay(200);
        this.HornOff();
        delay(200);
        this.HornOn();
        delay(200);
        this.HornOff();
        aligning = true;
      }
      if (this.CalibrateTrip()) {
        this.LightOn();
        digitalWrite(ledPin,HIGH);
        
      } else {
        digitalWrite(ledPin,LOW);
        this.LightOff();
        this.TimerReset();
      }
      
      if (this.MaxLength()){
        this.LightOff();
        delay(200);
        this.LightOn();
        delay(200);
        this.LightOff();
        delay(200);
        this.LightOn();
        delay(200);
        this.LightOff();
        myMode = myMode + 1;
        aligning = false;
      }
      stats();
    }

    void arm() {
      if (this.CheckTrip()) {
        previousMode = armMode;
        mode = 9;
        trippedWire = this.GetId();
        if (IsWarned()) {
          mode = 10;
          break;
        }
      } else {
        this.ResetAll();
      }
      }

    void ResetAll()
    {
      this.TimerReset();
      this.HornOff();
      this.LightOff();
      previousMillis = 0;
      beepMode = 0;
      warnReached = false;
    }
};


Jumpscare ooga("ooga", 0, 12, 13, 14, 15);
Jumpscare car("car", 1, 4, 5, 6, 7);
Jumpscare train("train", 2, 8, 9, 10, 11);

int randomizerTime = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(modePin, INPUT_PULLUP);
  Serial.begin(9600);
  randomizerTime = millis(); //set the randomizer timer
}




// Pull out the mode into Jumpscare class
// Simplify the logic of loop with each Jumpscare managing it's own mode state

void loop() {
  // Randomizer should be global with an enable/disable switch on each jumpscare
  // get random number between 0 and 2
  // if random number is 0, then ooga
  // if random number is 1, then car
  // if random number is 2, then train
  // only update randomizer every 5 seconds
  // check the randomizer to see if triggered
  if (randomizerTime + 5000 < millis()) {
    int randomNum = random(0, 3);
    ooga.enable(randomNum == 0);
    car.enable(randomNum == 1);
    train.enable(randomNum == 2);
  }
    //does something when the mode changes
  switch (mode) {
    case 0: //Hardware functionality test Mode
      stats();
      //turn on each relay in sequence, to verify functionality
      ooga.hardwareTest();
      car.hardwareTest();
      train.hardwareTest();
      mode = mode + 1;
    break;
    
    case 1: //Ooga tripwire align notify mode
      alignJumpscare(ooga);
    break;

    case 2: //Car tripwire align notify mode
      alignJumpscare(car);
    break;

    case 3: //Train tripwire align notify mode
      alignJumpscare(train);
    break;

    case 4: //test all jumpscares Mode
      ooga.Arm();
      car.Arm();
      train.Arm();
      
      if (digitalRead(modePin) == LOW) {
        mode = mode + 1;
      }
      stats();
    break;

    case 5: // Armed mode
      stats();
      if (!randomized) {
        randomSeed(analogRead(0));
        randNumber = random(300);
        Serial.println(randNumber);
        randomized = true;
      } else {
        if (randNumber < 100) {
          
        } else if (randNumber >= 100 && randNumber < 200) {
          
        } else if (randNumber >= 200) {
          
        }
      }

    break;
    
    case 6: //Trip Mode
      stats();
      
      if (trippedWire == 0) {
        ooga.HornOn();
        ooga.LightOn();
        if (ooga.MaxLength()) {
          mode = mode + 1;
          ooga.ResetAll();          // *****This will break everything with armed mode, as it will cause the randomizer to go through each time it reaches this point, thus telling the system to listen for a different tripwire.
          ooga.WarnReached();
          break;
        } 
      } else if (trippedWire == 1) {
        car.HornOn();
        car.LightOn();
        if (car.MaxLength()) {
          mode = mode + 1;
          car.ResetAll();
          car.WarnReached();
          break;
        }
      } else if (trippedWire == 2) {
        train.HornOn();
        train.LightOn();
        if (train.MaxLength()) {
          mode = mode + 1;
          train.ResetAll();
          train.WarnReached();
          break;
        } 
      }
      mode = previousMode;
    break;
    
    case 7: //Warn Mode
      stats();
      
      if (trippedWire == 0) {
        ooga.LightOn();
        ooga.BeepHorn();
        if (ooga.MaxLength()) {
          mode = mode + 1;
          ooga.ResetAll();
          break;
        } 
      } else if (trippedWire == 0) {
        car.LightOn();
        car.BeepHorn();
        if (car.MaxLength()) {
          mode = mode + 1;
          car.ResetAll();
          break;
        } 
      } else if (trippedWire == 0) {
        train.LightOn();
        train.BeepHorn();
        if (train.MaxLength()) {
          mode = mode + 1;
          train.ResetAll();
          break;
        } 
      }
      mode = previousMode;
    break;
    
    case 8: //Exit_notify Mode
      stats();
      
      mode = 1;
      ooga.HornOn();
      car.HornOn();
      train.HornOn();
      delay(700);
      ooga.ResetAll();
      car.ResetAll();
      train.ResetAll();
    break;
    }

    
  delay(1);                       // wait for a bit
}

//Writes stats to the Serial Port
void stats() {
  Serial.print("A:");
  Serial.print(ambiant);
  Serial.print(" T:");
  Serial.print(trip);
  Serial.print(" AT:");
  Serial.print(atAverage);
  Serial.print(" MODE:");
  Serial.print(modeNames[mode]);
  Serial.println("");
}
