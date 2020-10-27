/*
 Author: Yunyue Li
 Date: 11.10.2020
 Assignment 2: The safe
 */

#include "Display.h"

#define BTN_INPUT 9
#define BTN_CONFIRM 8
#define LED_YELLOW 7
#define LED_GREEN 5
#define LED_RED 4
#define BUZZER 3
#define LDR A2

int num = 0;
String result = "";
String passcode = "2134";
int failedUnlockAttempts = 0;

int lastBtnInputState = HIGH;
int lastBtnConfirmState = HIGH;

bool hasDoorOpened = false;

enum STATE { LOCKED, UNLOCKED };

int previousState = STATE::LOCKED;
int currentState = STATE::LOCKED;

enum INPUT_STATE {
  OFF,
  DIGIT1,
  DIGIT2,
  DIGIT3,
  DIGIT4,
  CONFIRM
};

int currentInputState = INPUT_STATE::OFF;
int lastInputState = INPUT_STATE::OFF;

void setup() {
  Serial.begin(9600);

  pinMode(BTN_INPUT, INPUT_PULLUP);
  pinMode(BTN_CONFIRM, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Display.clear();
}

void loop() {
  bool isDoorOpen = map(analogRead(LDR), 0, 500, 0, 1); 

  /* check if the door is opened when it is locked, if so the alarm is triggered. And door state is detected and changed*/
  switch(currentState) {
    case STATE::LOCKED: {
      if (isDoorOpen) {
        if (!hasDoorOpened) {
          enableAlarm();
        }
        
        hasDoorOpened = true;
        return;
      } else if (hasDoorOpened) {
        hasDoorOpened = false;
      }

      bool btnConfirm = isButtonPressed(BTN_CONFIRM, lastBtnConfirmState);
      bool btnInput = isButtonPressed(BTN_INPUT, lastBtnInputState);

      /* setting comfirm button function*/

      /*change digit positions*/
      if (btnConfirm) {
        switch(currentInputState) {
          case INPUT_STATE::OFF:
            Display.show("----");
            delay(1000);
            break;
          case INPUT_STATE::DIGIT1:
          case INPUT_STATE::DIGIT2:
          case INPUT_STATE::DIGIT3:
          case INPUT_STATE::DIGIT4:
            result += num;
        }

        int nextInputState = ++currentInputState;

        /* compare entered code with predefined passcode*/
        switch(nextInputState) {
          case INPUT_STATE::CONFIRM:
            if (result == passcode) {
              unlockVault();
              currentInputState = INPUT_STATE::OFF;
            } else {
              if (++failedUnlockAttempts >= 3) {
                enableAlarm();
                failedUnlockAttempts = 0;
              }
              
              Display.show("----");
              currentInputState = INPUT_STATE::DIGIT1;
            }
            
            result = "";
            break;
        }
        
        num = 1;
      }
      
      /* setting input button function */
      if (btnInput) {    
        if (++num % 5 == 0) {
          num = 1;
        }
      }

      /* update the display*/
      if (btnConfirm || btnInput) {        
        switch(currentInputState) {
          case INPUT_STATE::DIGIT1:
          case INPUT_STATE::DIGIT2:
          case INPUT_STATE::DIGIT3:
          case INPUT_STATE::DIGIT4:
            Display.showCharAt(currentInputState - 1, '0'+ num);
        }
      }
      break;
    }
    
    /* when it is unlocked and the door is open, door state is changed. 
       when the door is close, it is locked again.
       only when the door is opened with correct passcode and then close, will it send notification (yellow led)
    */
    case STATE::UNLOCKED:
      if (isDoorOpen) {
        hasDoorOpened = true;
      } else if (hasDoorOpened) {
        currentState = STATE::LOCKED;
      }
      break;
    default:
      currentState = STATE::LOCKED;
  }

  if (currentState != previousState) {
    stateChangeNotification();
  }

  previousState = currentState;
}

bool isButtonPressed(int pinNumber, int &lastButtonState) {
  int buttonState = digitalRead(pinNumber);

  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
    
    if (buttonState == LOW) {
      return true;
    }
  }

  return false;
}

void enableAlarm() {
  Serial.println((String)"ALARM SAFE " + 1337);
  
  tone(BUZZER, 1000);

  for (int i = 0; i < 10; ++i) {
    digitalWrite(LED_RED, HIGH);
    delay(250);
    digitalWrite(LED_RED, LOW);
    delay(250);
  }

  noTone(BUZZER);
}


void unlockVault() {
  currentState = STATE::UNLOCKED;
  Display.show("OPEN");
  digitalWrite(LED_GREEN, HIGH);
  delay(5000);
  digitalWrite(LED_GREEN, LOW);
  Display.clear();
}

void stateChangeNotification() {
  tone(BUZZER, 1000);
  digitalWrite(LED_YELLOW, HIGH);
  delay(200);
  digitalWrite(LED_YELLOW, LOW);
  noTone(BUZZER);
}
