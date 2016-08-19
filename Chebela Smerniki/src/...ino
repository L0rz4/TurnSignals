#include <Arduino.h>

#define TRIGGER_LEFT   8
#define TRIGGER_RIGHT  9
#define TRIGGER_HAZARD 10

#define TRIGGER_ERROR_LEFT_1 2
#define TRIGGER_ERROR_LEFT_2 3
#define TRIGGER_ERROR_LEFT_3 4

#define TRIGGER_ERROR_RIGHT_1 5
#define TRIGGER_ERROR_RIGHT_2 6
#define TRIGGER_ERROR_RIGHT_3 7

#define POWER_LEFT  11
#define POWER_RIGHT 12
#define TIMER_NORMAL  750
#define TIMER_ERROR 250


volatile bool bPowerLeft;
volatile bool bPowerRight;

volatile bool bTriggerLeft;
volatile bool bTriggerRight;
volatile bool bTriggerHazard;

volatile bool bErrorLeft;
volatile bool bErrorRight;


unsigned long elapsed = 0;

void setup()
{
  Serial.begin(115200);

  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

  bPowerLeft = false;
  bPowerRight = false;
  bTriggerLeft = false;
  bTriggerRight = false;
  bTriggerHazard = false;
  bErrorLeft = false;
  bErrorRight = false;

  //pinMode(POWER_LEFT, OUTPUT);
  pinMode(TRIGGER_LEFT, INPUT_PULLUP);
  pinMode(TRIGGER_RIGHT, INPUT_PULLUP);
  pinMode(TRIGGER_HAZARD, INPUT_PULLUP);
  pinMode(TRIGGER_ERROR_LEFT_1, INPUT_PULLUP);
  pinMode(TRIGGER_ERROR_LEFT_2, INPUT_PULLUP);
  pinMode(TRIGGER_ERROR_LEFT_3, INPUT_PULLUP);

  pinMode(TRIGGER_ERROR_RIGHT_1, INPUT_PULLUP);
  pinMode(TRIGGER_ERROR_RIGHT_2, INPUT_PULLUP);
  pinMode(TRIGGER_ERROR_RIGHT_3, INPUT_PULLUP);


  pinMode(POWER_LEFT, OUTPUT);
  pinMode(POWER_RIGHT, OUTPUT);
  //digitalWrite(POWER_LEFT, LOW);

  //DDRD = DDRD & 0b00000011;
  //DDRB = DDRB | 0b00011000;
}

SIGNAL(TIMER0_COMPA_vect){
  elapsed++;
  if(elapsed == TIMER_NORMAL || (elapsed == TIMER_ERROR && (bErrorLeft || bErrorRight)) ){
    bPowerLeft = false;
    bPowerRight = false;
  } else if(elapsed == TIMER_NORMAL*2 || (elapsed == TIMER_ERROR*2 && (bErrorLeft || bErrorRight)) ){
    bPowerLeft = true;
    bPowerRight = true;
    elapsed = 0;
  }

  if(elapsed == TIMER_ERROR){
    if(bTriggerHazard){
      bErrorLeft  = !((PIND >> PIND6) & 1);
      bErrorRight = !((PIND >> PIND7) & 1);
    } else if(bErrorLeft) {
      bErrorLeft  = !((PIND >> PIND6) & 1);
    } else if(bErrorRight){
      bErrorRight = !((PIND >> PIND7) & 1);
    }
  }
}


void loop() {
  bTriggerLeft = !((PINB  >> PINB0) & 1);
  bTriggerRight = !((PINB  >> PINB1) & 1);
  bTriggerHazard = !((PINB  >> PINB2) & 1);

  if(bTriggerLeft || bTriggerRight || bTriggerHazard) {
    if(bTriggerHazard) {
      PORTB ^= (-bPowerLeft ^ PORTB) & (1 << PINB3);
      PORTB ^= (-bPowerRight ^ PORTB) & (1 << PINB4);
      Serial.println("Hazard pressed");
    } else if(bTriggerLeft){
      PORTB &= ~(1 << PINB4);
      PORTB ^= (-bPowerLeft ^ PORTB) & (1 << PINB3);
      Serial.println("Left pressed");
    } else if(bTriggerRight) {
      PORTB &= ~(1 << PINB3);
      PORTB ^= (-bPowerRight ^ PORTB) & (1 << PINB4);
      Serial.println("Right pressed");
    }

    if(elapsed == 0){
      bPowerLeft = true;
      bPowerRight = true;
    }
  } else if(!bPowerLeft && !bPowerRight) {
    elapsed = 0;
    PORTB &= ~(1 << PINB3);
    PORTB &= ~(1 << PINB4);
  }

}
