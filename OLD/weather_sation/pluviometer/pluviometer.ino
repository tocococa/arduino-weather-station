
#include <math.h>

static int RainSensorPin = 2;    //The pin location of the anemometer sensor

static int MaxTicks = 100;

volatile unsigned long Ticks = 0;  // tipping bucket counter used in interrupt routine
volatile unsigned long ContactBounceTime;  // Timer to avoid contact bounce in interrupt routine

float Rainfall;        // rainfall in mm

void setup() {

  Serial.begin(9600);

  pinMode(RainSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(RainSensorPin), isr_tick, FALLING);

  Serial.println("Ticks\t mm");

}

void loop() {

  sei();         //Enables interrupts
  delay(3000);  //Wait 3 seconds to average
  cli();         //Disable interrupts

  Rainfall = Ticks * 0.2794;  // 0.2794 -> capacity of each bucket

  Serial.print(Ticks);
  Serial.print("\t");
  Serial.println(Rainfall);
  if(Ticks > MaxTicks){
    Ticks = 0;
  }

}

// This is the function that the interrupt calls to increment the rotation count
void isr_tick ()   {

  if ((millis() - ContactBounceTime) > 15 ) {  // debounce the switch contact.
    Ticks++;
    ContactBounceTime = millis();
  }

}
