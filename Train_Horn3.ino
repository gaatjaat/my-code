/*
  Train_Horn
 
  activates two banks of horns (pins 3 and 4) and an LED (pin 13)
  when the laser tripwire (pin 7) is triggered or pushbutton (pin 2) is pressed.
  
  There is a small delay between activating the two banks of horns
  to account for motor startup amperage draw.
  
  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground


  updated 2022
  by gaatjaat
  first modified 20 Sept 2020
  from DojoDave's Button sketch

*/

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;     // the number of the pushbutton pin
const int trainTrip = 7;      // the number of the train photoresistor pin
const int carTrip = 8;      // the number of the train photoresistor pin
const int oogaTrip = 9;      // the number of the train photoresistor pin
const int ledPin =  13;      // the number of the LED pin
const int trainHornA =  3;      // the number of the train horn bank A relay pin
const int trainHornB =  4;      // the number of the train horn bank B relay pin
const unsigned long hornMaxTimeMillis = 5000;

//Main loop states
const int LASER_ALIGNING = 1;
const int ARMED = 2;
const int TRAIN_HORN_ACTIVATED = 3;

int mainLoopState = LASER_ALIGNING;


// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int tripWireState = 0;       // variable for reading the photoresistor status
unsigned long hornActivationTime = 0;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // initialize the LED and relay pins as output:
  pinMode(ledPin, OUTPUT);
  digitalWrite(trainHornA, HIGH);
  digitalWrite(trainHornB, HIGH);
  pinMode(trainHornA, OUTPUT);
  pinMode(trainHornB, OUTPUT);
  // initialize the pushbutton and photoresistor pins as an input:
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trainTrip, INPUT_PULLUP);
}




void loop() {
    Serial.println("Start of loop");
    tripWireState = digitalRead(trainTrip); //read initial tripWireState
    buttonState = digitalRead(buttonPin); //read initial buttonState

    bool buttonPressed = false;
    bool photoResisterSeesLight = false;

    buttonPressed = buttonState == LOW;
    photoResisterSeesLight = tripWireState == LOW;


    

    
    if (mainLoopState == LASER_ALIGNING) {
      if (photoResisterSeesLight) {
        digitalWrite(ledPin, HIGH);
      }
      else {
        digitalWrite(ledPin, LOW);
      }
        Serial.println("Laser Aligning");

        if (buttonPressed && photoResisterSeesLight) {
            mainLoopState = ARMED;
            Serial.println("Laser Arming");
        }

    } else if (mainLoopState == ARMED) {
        digitalWrite(ledPin, LOW); 
        Serial.println("Laser Armed");

        if (!photoResisterSeesLight) {

            Serial.println("Laser tripped");
            digitalWrite(trainHornA, LOW); //turn on first horn
            delay(220); //small delay to account for motor startup amperage draw
            digitalWrite(trainHornB, LOW); //turn on second horn

            hornActivationTime = millis();

            mainLoopState = TRAIN_HORN_ACTIVATED;
        }

    } else if (mainLoopState == TRAIN_HORN_ACTIVATED) {
        digitalWrite(ledPin, LOW); 
        Serial.println("Horns on");

        bool hornOnTooLong = false;
        hornOnTooLong = millis() - hornActivationTime > hornMaxTimeMillis;
        if (hornOnTooLong) {
            Serial.println("Horn on too long");
            digitalWrite(trainHornA, HIGH);
            digitalWrite(trainHornB, HIGH);

            mainLoopState = LASER_ALIGNING;
        } else if (photoResisterSeesLight) {
            Serial.println("Waiting for laser alignment");
            digitalWrite(trainHornA, HIGH);
            digitalWrite(trainHornB, HIGH); 

            mainLoopState = ARMED;
        }
    }
}
