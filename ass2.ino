/*
 Author: Yunyue Li
 Date: 01.10.2020
 Assignment 2: The safe
 */

#define BTN_INPUT 9
#define BTN_COMFIRM 8
#define LED_GREEN 5
#define LED_RED 4
#define BUZZER 3
#define LDR A2

#include "Display.h"
int pos0 = 0;
int pos1 = 1;
int pos2 = 2;
int pos3 = 3;

int comfirm = HIGH;
int lastComfirm = HIGH;

int input = HIGH;
int lastInput = HIGH;
int num = 0;

String passcode = "2134";
String result ="";


enum STATE {
  LOCKED,
  UNLOCKED
};

int previousState = STATE::LOCKED;
int currentState = STATE::LOCKED;

enum DIGITSTATE {
  OFF,
  DIGIT1,
  DIGIT2,
  DIGIT3,
  DIGIT4
};

int currentDigitState = DIGITSTATE::OFF;

int digit1 = 0;
int digit2 = 0;
int digit3 = 0;
int digit4 = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BTN_INPUT, INPUT_PULLUP);
  pinMode(BTN_COMFIRM, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Display.clear();
}

void loop() {
  bool isDoorOpen = map(analogRead(LDR), 0, 100, 0, 1);

  switch(currentState) {
    case STATE::LOCKED:
      if (isDoorOpen) {
        currentState = STATE::UNLOCKED;
        return;
      }

      break;
    case STATE::UNLOCKED:
      if (!isDoorOpen) {
        currentState = STATE::LOCKED;
        return;
      }

      if (currentState != previousState) {
        enableAlarm();
        delay(5000);
        disableAlarm();
      }
      
      break;
    default:
      currentState = STATE::LOCKED;
  }

  previousState = currentState;

  input = digitalRead(BTN_INPUT);

  if (input != lastInput) {
    if (input == LOW) {
     num += 1;
     if (num % 5 == 0) {
      num = 1;
     }
    }

    lastInput = input;
  }
  
  comfirm = digitalRead(BTN_COMFIRM);

  if (comfirm != lastComfirm) {
    if (comfirm == LOW) {
      if (currentDigitState == DIGITSTATE::OFF) {
        Display.show("----");
      } else if (currentDigitState == DIGITSTATE::DIGIT4) {
        Display.clear();
      }

      if (++currentDigitState > DIGITSTATE::DIGIT4) {
        currentDigitState = DIGITSTATE::DIGIT1;
      }     
    }

    lastComfirm = comfirm;
  }

  if (currentDigitState == DIGITSTATE::DIGIT1) {
    digit1 = num;
    Display.showDigitAt(pos0, num);
  } else if (currentDigitState == DIGITSTATE::DIGIT2) {
    digit2 = num;
    Display.showDigitAt(pos1, num);
  } else if (currentDigitState == DIGITSTATE::DIGIT3) {
    digit3 = num;
    Display.showDigitAt(pos2, num);
  } else if (currentDigitState == DIGITSTATE::DIGIT4) {
    digit4 = num;
    Display.showDigitAt(pos3, num);
  }

  result = String(digit1) + String(digit2) + String(digit3) + String(digit4);

  Serial.println(result);
  if (result == passcode) {
    Display.clear();
    delay(2000);
  }

}


 

void enableAlarm() {
  digitalWrite(LED_RED, HIGH);
  tone(BUZZER, 1);
}

void disableAlarm() {
  digitalWrite(LED_RED, LOW);
  noTone(BUZZER);
}

void reset() {
  
}
