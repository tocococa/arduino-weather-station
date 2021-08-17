#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <math.h>
#include <stdbool.h>

// Shared

volatile unsigned long ContactBounceTime;

// WIND
// wind speed (cup anemometer)
const int WindSensorPin = 3;
volatile unsigned int Rotations = 0;
float WindSpeed;
// wind direction (vane)
const int VanePin = A0;
int WindVane = 0;
int VaneOutput;
int WindDirection;
const int CompassDirections = 16;
const int VaneReading[CompassDirections] = {940, 890, 820, 785, 690, 630, 590, 455, 400, 285, 240, 180, 125, 90, 80};
//^check these numbers^
const float VaneDirection[CompassDirections] = {270.0, 305.0, 282.5, 0.0, 22.5, 337.5, 225.0, 247.5, 45.0, 22.5, 180.0, 202.5, 135.0, 157.5, 90.0, 112.5};

// RAIN (tipping bucket gauge)
const int RainGaugePin = 2;
volatile int Ticks = 0;
static int MaxTicks = 100;
float Rainfall;

// HUMIDITY+TEMP (DHT11)
const int DhtPin = 4;
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float Temperature;
float Humidity;


// PRESSURE (BMP180)
SFE_BMP180 pressure;
const float Altitude = 828.0;
int Press;

void setup(){
    Serial.begin(9600);
    Serial.println("REBOOT");
    Serial.println("FORMAT:");
    Serial.println("Temp,Humidity,Press,WindSpd,WindDir,Rain") //data format
    Serial.flush();
    Serial.end();
    //rainfall
    pinMode(RainGaugePin, INPUT);
    attachInterrupt(digitalPinToInterrupt(RainGaugePin), isr_tick, FALLING);
    //windspeed
    pinMode(WindSensorPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);
    //pressure
    if(!pressure.begin()){
        Serial.print("BMP180 Fail");
        while(1);
    }
    //temp+humidity
    dht.begin();
    //vane -> no interrupt, get data every VaneTimer seconds
}

void loop(){

}

void serialComm(){
    Serial.begin(9600);
    Serial.print(Temperature);Serial.print(",");
    Serial.print(Humidity);Serial.print(",");
    Serial.print(Press);Serial.print(",");
    Serial.print(WindSpeed);Serial.print(",");
    Serial.print(WindDirection);Serial.print(",");
    Serial.print(Rainfall);
    Serial.flush();
    Serial.end();
}

float readCup(){
    Rotations = 0;
    sei();
    delay(3000);
    cli();
    // convert to km/h using the formula V=P(2.4/T)
    // V = P(2.4/3) = P * 0.8
    WindSpeed = Rotations * 0.8;
}

int readVane(){
    int Readings;
    int Samples = 10;
    for(int i = 0; i < Samples, i++){
        Readings = Readings + analogRead(VanePin);
        delay(100);
    }
    int Average = Readings / Samples;z
}

double readPress(){
    double T,P,p0,a;
    char status = pressure.startTemperature();
    delay(status);
    status = pressure.getTemperature(T);
    if(status != 0){
        status = pressure.startPressure(3);
        if(status != 0){
            delay(status);
            status = pressure.getPressure(P,T);
            if(status != 0){
                return pressure.sealevel(P, Altitude);
            }
        }
    }
    return NAN;
}

float readTemp(){
    float t = dht.readTemperature();
    if(isnan(t)){
        return NAN;
    }
    return t;
}

float readHumid(){
    float h = dht.readHumidity();
    if(isnan(h)){
        return NAN;
    }
    return h;
}

//rotation interrupt for cup anemometer -> only check during readWindspeed
void isr_rotation(){
  if ((millis() - ContactBounceTime) > 15 ){  // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
}

//tick interrupt for tipping bucket rain gauge -> must always be listening
void isr_tick(){
    if ((millis() - ContactBounceTime) > 15 ){  // debounce the switch contact.
        Ticks++;
        ContactBounceTime = millis();
    }
}