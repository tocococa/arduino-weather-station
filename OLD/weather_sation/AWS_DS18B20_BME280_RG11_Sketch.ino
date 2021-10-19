#include "cactus_io_DS18B20.h" 
#include "cactus_io_BME280_I2C.h"

#define Bucket_Size 0.01   // bucket size to trigger tip count
#define RG11_Pin 3         // digital pin RG11 connected to
#define TX_Pin 8           // used to indicate web data tx
#define DS18B20_Pin 9      // DS18B20 Signal pin on digital 9 

volatile unsigned long tipCount;    // bucket tip counter used in interrupt routine 
volatile unsigned long contactTime; // Timer to manage any contact bounce in interrupt routine
volatile float totalRainfall;       // total amount of rainfall detected
bool txState;            // current led state for tx rx led

// Create DS18B20, BME280 object 
DS18B20 ds(DS18B20_Pin); // on digital pin 9
BME280_I2C bme; // I2C using address 0x77

void setup() { 

  txState = HIGH;
  tipCount = 0; 
  totalRainfall = 0; 
  
  ds.readSensor(); 
  
  Serial.begin(9600); 
  Serial.println("cactus.io | Weather Station DS18B20, BME280 Sensor Test"); 
  Serial.println("DS Temp\t\tBME Temp\tHumdity\t\tPressure\tRainfall"); 

  if (!bme.begin()) { 
    Serial.println("Could not find BME280 sensor, check wiring!"); 
    while (1); 
  } 

  pinMode(TX_Pin,OUTPUT);
  pinMode(RG11_Pin, INPUT); 
  attachInterrupt(digitalPinToInterrupt(RG11_Pin), isr_rg, FALLING);
  sei();// Enable Interrupts 
} 

void loop() { 

  txState = !txState;         // toggle the led state
  
  ds.readSensor(); 

  digitalWrite(TX_Pin,txState);

  Serial.print(ds.getTemperature_C()); Serial.print(" *C\t"); 
  Serial.print(bme.getTemperature_C()); Serial.print(" *C\t"); 
  Serial.print(bme.getHumidity()); Serial.print(" %\t\t"); 
  Serial.print(bme.getPressure_MB()); Serial.print(" mb\t");
  Serial.print(totalRainfall); Serial.println(" mm");

  // Add a 2 second delay. 
  delay(2000); //just here to slow down the output.   
  digitalWrite(TX_Pin,txState);
}

// Interrupt handler routine that is triggered when the rg-11 detects rain
void isr_rg() {

  if((millis() - contactTime) > 15 ) { // debounce of sensor signal 
    tipCount++;
    totalRainfall = tipCount * Bucket_Size;
    contactTime = millis(); 
  } 
} 
// end of rg-11 rain detection interrupt handler