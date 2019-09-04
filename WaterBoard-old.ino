//TODO
// Add more states - eg. FILLING_IN_PROGRESS
// Add PCB Traces for extra switches

const int WAITINGFORBUTTON = 101;
const int WATERCREATION = 102;
const int WATERSHUTDOWN = 103;
const int CRITICALHIGH = 104;

const int buttonPin = 12;
const int ledPin = 13;
const int level_HH1_Pin = 11;
//const int level_HH2_Pin = A0;
const int level_H1_Pin = 10;
//const int level_H2_Pin = A1;
const int level_L1_Pin = 9;
//const int level_L2_Pin = A2;

const int mainSolenoidPin = 5;
const int flushSolenoidPin = 6;
const int saltSolenoidPin = 7;
const int auxSolenoidPin = 8;

byte flag;
int currentState;
unsigned long lastStateChange;
unsigned long timer;

void setNewState(int newState) {
  currentState = newState;
  lastStateChange =  millis();
}

void setup(){
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(mainSolenoidPin, OUTPUT);
  pinMode(flushSolenoidPin, OUTPUT);
  pinMode(saltSolenoidPin, OUTPUT);
  pinMode(auxSolenoidPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(level_HH1_Pin, INPUT_PULLUP);
//  pinMode(level_HH2_Pin, INPUT_PULLUP);
  pinMode(level_H1_Pin, INPUT_PULLUP);
//  pinMode(level_H2_Pin, INPUT_PULLUP);
  pinMode(level_L1_Pin, INPUT_PULLUP);
//  pinMode(level_L2_Pin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);
  digitalWrite(mainSolenoidPin, HIGH);
  digitalWrite(flushSolenoidPin, LOW);
  digitalWrite(saltSolenoidPin, HIGH);
  digitalWrite(auxSolenoidPin, HIGH);

  flag = 0;
  setNewState(WAITINGFORBUTTON);
}

void loop() {

  // Failsafe
  if(digitalRead(level_HH1_Pin) != LOW) {
    setNewState(CRITICALHIGH);
  }
//  if(digitalRead(level_HH2_Pin) != LOW) {
//    setNewState(CRITICALHIGH);
//  }
  

  switch (currentState) {
    case WAITINGFORBUTTON:
      // Just in case - Let's make DAMN sure the water is off.
      digitalWrite(mainSolenoidPin,HIGH);
      digitalWrite(flushSolenoidPin,LOW);
      digitalWrite(saltSolenoidPin,HIGH);
      digitalWrite(auxSolenoidPin,HIGH);
      // Monitor inputs without delay
      // Trigger Water creation process if button is pressed, or if water hits Low Level
      if(digitalRead(buttonPin) == LOW) {
        setNewState(WATERCREATION);
        break;
        }
      if(digitalRead(level_L1_Pin) == LOW) {
        setNewState(WATERCREATION);
        break;
        }
      // Standby Mode
      if ( (unsigned long)(millis() - timer) >= (10 * 1000)) {
        Serial.println("Water Level OK - Waiting for Button.");
        timer = millis();
      }
      break;

    case WATERCREATION:
       // Regular Shutdown
       if(digitalRead(level_H1_Pin) != LOW) {
         setNewState(WATERSHUTDOWN);
         break;
         }
//       if(digitalRead(level_H2_Pin) != LOW) {
//         setNewState(WATERSHUTDOWN);
//         break;
//         }
       // Let's make some RO/DI water!
       if(millis() - lastStateChange < 60000) {
         // open the Flush and Main Solenoids
         if (flag != 100) {
          flag = 100;
          Serial.println("Opening Both RODI Solenoids");
          Serial.println("Counting to 60sec"); 
          }
        digitalWrite(flushSolenoidPin,LOW);
        digitalWrite(mainSolenoidPin,LOW);
       }
       else {
        // This runs after ~60 seconds elapsed
        // Close the Flush Solenoid
        if (flag != 101) {
          flag = 101;
          Serial.println("Closing the flush Solenoid");
        }
        digitalWrite(mainSolenoidPin,LOW);
        digitalWrite(flushSolenoidPin, HIGH);
        }
        if(millis() - lastStateChange > 60000) {
          if ( (unsigned long)(millis() - timer) >= (10 * 1000)) {
          Serial.println("RO/DI is currently filling the tank");
          timer = millis();
          }
        }
      break;

    case WATERSHUTDOWN:
       // Finished making water
       if (flag != 200) {
         flag = 200;
         Serial.println("Finished filling drum.");
         Serial.println("Opening flush solenoid for 60 secs."); //Main supply solenoid should already be open.
         }
       if(millis() - lastStateChange < 60000) {
         digitalWrite(mainSolenoidPin,LOW);
         digitalWrite(flushSolenoidPin,LOW);
         }
       else {
         if (flag != 201) {
           flag = 201;
           Serial.println("All done.");
           Serial.println("Closing Main Solenoid");
           }
         digitalWrite(mainSolenoidPin, HIGH);
         setNewState(WAITINGFORBUTTON);
         }
       break;

    case CRITICALHIGH:
       // HOLY FUCK ITS ABOUT TO OVERFLOW
       // Time to panic - close everything
       Serial.println("PANIC PANIC - CLOSE EVERYTHING");
       digitalWrite(flushSolenoidPin, HIGH);
       digitalWrite(mainSolenoidPin, HIGH);
       digitalWrite(saltSolenoidPin, HIGH);
       // maybe change this state to an error-related action later
       Serial.println("System Halted");
       while (true) {
        digitalWrite(ledPin, HIGH);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);
       }
       break;
  }
}
