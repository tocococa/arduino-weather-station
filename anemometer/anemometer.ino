
#include <math.h>

static int WindSensorPin = 3;      //The pin location of the anemometer sensor

volatile unsigned long Rotations = 0;  // cup rotation counter used in interrupt routine
volatile unsigned long ContactBounceTime;  // Timer to avoid contact bounce in interrupt routine

float WindSpeed;        // speed kph

void setup() {

  Serial.begin(9600);

  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

  Serial.println("Rotations\tKPH");

}

void loop() {

  Rotations = 0; //Set NbTops to 0 ready for calculations
  sei();         //Enables interrupts
  delay (3000);  //Wait 3 seconds to average
  cli();         //Disable interrupts

  // convert to km/h using the formula V=P(2.4/T)
  // V = P(2.4/3) = P * 0.8

  WindSpeed = Rotations * 0.8;

  Serial.print(Rotations);   Serial.print("\t\t");
  Serial.println(WindSpeed);

}

// This is the function that the interrupt calls to increment the rotation count
void isr_rotation ()   {

  if ((millis() - ContactBounceTime) > 15 ) {  // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }

}
