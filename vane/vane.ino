const int vaneAnalogPin = A0;  // Analog input pin that the potentiometer is attached to

int windVane = 0;           // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
int direction = 0;          // direction of vane

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  // read the analog in value:
  windVane = analogRead(vaneAnalogPin);
  Serial.println(windVane);
  // map it to the range of the analog out:
  outputValue = map(windVane, 0, 1023, 0, 255);
  // change the analog out value:
  int temp;

  if( windVane > 940){
    Serial.print("heading = W");
  } //W
	else if(windVane > 880){
    Serial.print("heading = NW");
  } //NW
	else if(windVane > 820){
    Serial.print("heading = WNW");
  } //WNW
	else if(windVane > 780){
    Serial.print("heading = N");
  }  //N
	else if(windVane > 695){
    Serial.print("heading = NNW");
  } //NNW
	else if(windVane > 625){
    Serial.print("heading = SW");
  } //SW
	else if(windVane > 593){
    Serial.print("heading = WSW");
  } //WSW
	else if(windVane > 455){
    Serial.print("heading = NE");
  }  //NE
	else if(windVane > 400){
    Serial.print("heading = NNE");
  }  //NNE
	else if(windVane > 282){
    Serial.print("heading = S");
  }  //S
	else if(windVane > 240){
    Serial.print("heading = SSW");
  }  //SSW
	else if(windVane > 182){
    Serial.print("heading = SE");
  }  //SE
	else if(windVane > 125){
    Serial.print("heading = SSE");
  }  //SSE
	else if(windVane > 93){
    Serial.print("heading = E");
  }  //E
	else if(windVane > 85){
    Serial.print("heading = ENE");
  }  //ESE
  else if(windVane > 67){
    Serial.print("heading = ESE");
  }
	else temp = 5;
  // print the results to the Serial Monitor:
  Serial.print("\t sensor = ");
  Serial.print(windVane);
  Serial.print("\t output = ");
  Serial.println(outputValue);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2000);
}