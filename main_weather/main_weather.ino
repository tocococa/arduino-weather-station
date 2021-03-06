#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <math.h>
#include <stdbool.h>

// Shared

volatile unsigned long ContactBounceTime;
const unsigned long RefreshRate = 900000;  // milis
unsigned long LastRefresh = 0.0;
unsigned long TimeNow;

// WIND
// wind speed (cup anemometer)
const int WindSensorPin = 3;
volatile unsigned int Rotations = 0;
float WindSpeed;
// wind direction (vane)
const int VanePin = A0;
int WindVane = 0;
int VaneOutput;
char* WindDirection;
const int CompassDirections = 16;
const int VaneReading[CompassDirections] = {940, 880, 820, 780, 695, 625, 593, 455, 400, 280, 240, 182, 125, 89, 85, 67};
const float VaneDirection[CompassDirections] = {270.0, 305.0, 282.5, 0.0, 22.5, 337.5, 225.0, 247.5, 45.0, 22.5, 180.0, 202.5, 135.0, 157.5, 90.0, 112.5};
char*  CardinalDirection[CompassDirections] = {"E", "SE", "ESE", "S", "SSE", "NE", "ENE", "SW", "SSW", "N", "NNE", "NW", "NNW", "W", "WSW", "WNW"};

// RAIN (tipping bucket gauge)
const int RainGaugePin = 2;
volatile int Ticks = 0;
static int MaxTicks = 100;
float Rainfall;

// HUMIDITY+TEMP (DHT11)
const int DhtPin = 4;
#define DHTTYPE DHT11
DHT dht(DhtPin, DHTTYPE);
float Temperature;
float Humidity;

// PRESSURE (BMP180)
SFE_BMP180 pressure;
const float Altitude = 828.0;
double Press;

// LED
int led = 13;

void setup(){
    Serial.begin(9600);
    Serial.println("REBOOT");
    Serial.println("FORMAT:");
    Serial.println("Temp,Humidity,Press,WindSpd,WindDir,Rain"); //data format
    // temp in C, hmdty in %, press in mb, windspd in km/s
    // winddir in deg, rain in mm
    Serial.flush();
    Serial.end();
    //led 
    pinMode(led, OUTPUT);
    //rainfall
    pinMode(RainGaugePin, INPUT);
    attachInterrupt(digitalPinToInterrupt(RainGaugePin), isr_tick, FALLING);
    //windspeed
    pinMode(WindSensorPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, RISING);
    sei(); // enable interrupts
    //temp+humidity
    dht.begin();
    //pressure -> Sensor is shot?
    if(!pressure.begin()){
        serialDebug("BMP180 Fail");
        while(1);
    }
    //vane -> no interrupt, get data every VaneTimer seconds
}

void loop(){
    TimeNow = millis();
    if((TimeNow - LastRefresh) > RefreshRate){// poll all sensors every RefreshRate milis
        LastRefresh = TimeNow;
        Temperature = readTemp();
        Humidity = readHumid();
        Press = readPress();
        WindSpeed = readCup();
        WindDirection = readVane();
        Rainfall = getRainfall();
        serialComm();
    }
    // rain uses an interrupt to trigger
}

void serialComm(){
    Serial.begin(9600);
    Serial.print(Temperature);Serial.print(",");
    Serial.print(Humidity);Serial.print(",");
    Serial.print(Press);Serial.print(",");
    Serial.print(WindSpeed);Serial.print(",");
    Serial.print(WindDirection);Serial.print(",");
    Serial.println(Rainfall);
    Serial.flush();
    Serial.end();
}

void serialDebug(char* err){
    Serial.begin(9600);
    Serial.println(err);
    Serial.flush();
    Serial.end();
}

float readCup(){
    Rotations = 0;
    delay(3000);
    // convert to km/h using the formula V=P(2.4/T)
    // V = P(2.4/3) = P * 0.8
    return Rotations * 0.8;
}

char* readVane(){
    /* int Readings;  -> FIX THIS
    int Samples = 10;
    for(int i = 0; i < Samples; i++){
        int Reading = analogRead(VanePin);
        Readings = Readings + Reading;
        delay(10);
    }
    Readings = (Readings / Samples); // get average */
    int Reading = analogRead(VanePin);
    for (int i = 0; i < CompassDirections; i++){
        if(Reading > VaneReading[i]){
            return CardinalDirection[i];
        }
    }
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
                // returns relative pressure in milibar
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
    return t; // temperature in Celsius
}

float readHumid(){
    float h = dht.readHumidity();
    if(isnan(h)){
        return NAN;
    }
    return h; // relative humidity
}

float getRainfall(){
    float Rainfall = Ticks * 0.2794;
    Ticks = 0;
    return Rainfall; // rainfall in mm since last sample
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