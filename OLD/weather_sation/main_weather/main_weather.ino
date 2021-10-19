#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <math.h>
#include <stdbool.h>

// Shared

volatile unsigned long ContactBounceTime;

// WIND
// wind speed
const int WindSensorPin = 3;
volatile unsigned int Rotations = 0;
float WindSpeed;
// wind direction
const int VanePin = A0;
int WindVane = 0;
int VaneOutput;
int WindDirection;

// RAIN
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
    dht.begin();
    Serial.println("REBOOT");
    Serial.println("FORMAT:");
    Serial.println("Temp,Humidity,Press,WindSpd,WindDir,Rain") //data format
    //rainfall
    pinMode(RainSensorPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(RainSensorPin), isr_tick, FALLING);
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

void serial_comm(){
    Serial.print(Temperature);Serial.print(",");
    Serial.print(Humidity);Serial.print(",");
    Serial.print(Press);Serial.print(",");
    Serial.print(WindSpeed);Serial.print(",");
    Serial.print(WindDirection);Serial.print(",");
    Serial.print(Rainfall);
}

float read_cup(){

}

int read_vane(){

}

float read_rain(){

}

float read_press(){

}

float read_temp(){

}

void isr_rotation(){

}

void isr_tick(){

}