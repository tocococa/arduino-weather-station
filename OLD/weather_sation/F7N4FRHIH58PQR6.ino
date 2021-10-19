/*
description:     Basic Rain Gauge with arduino with serial monitoring
                 Reports the daily-rain and rain-in-last-hour in inches
acknowledgement: part of the code copied and modified from
                 http://www.instructables.com/id/Arduino-Weather-Station-Part3-Rain/
liscence:        GNU GPL. https://www.gnu.org/licenses/gpl.html
*/

#include "RTClib.h"
#include <Wire.h>
#define RainPin 2                         // The Rain input is connected to digital pin 2 on the arduino


bool bucketPositionA = false;             // one of the two positions of tipping-bucket               
const double bucketAmount = 0.01610595;   // inches equivalent of ml to trip tipping-bucket
double dailyRain = 0.0;                   // rain accumulated for the day
double hourlyRain = 0.0;                  // rain accumulated for one hour
double dailyRain_till_LastHour = 0.0;     // rain accumulated for the day till the last hour          
bool first;                               // as we want readings of the (MHz) loops only at the 0th moment 

RTC_Millis rtc;                           // software RTC time


void setup(void) {
  Serial.begin(9600);                            // start the serial port
  rtc.begin(DateTime(__DATE__, __TIME__));       // start the RTC
  pinMode(RainPin, INPUT);                       // set the Rain Pin as input.
  delay(4000);                                   // i'm slow in starting the seiral monitor (not necessary)
  Serial.println("Ready!!!");                    // not necessary too
}


void loop(void){
  DateTime now = rtc.now();
    
  // ++++++++++++++++++++++++ Count the bucket tips ++++++++++++++++++++++++++++++++
  if ((bucketPositionA==false)&&(digitalRead(RainPin)==HIGH)){
    bucketPositionA=true;
    dailyRain+=bucketAmount;                               // update the daily rain
  }
  
  if ((bucketPositionA==true)&&(digitalRead(RainPin)==LOW)){
    bucketPositionA=false;  
  } 
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  if(now.minute() != 0) first = true;                     // after the first minute is over, be ready for next read
  
  if(now.minute() == 0 && first == true){
 
    hourlyRain = dailyRain - dailyRain_till_LastHour;      // calculate the last hour's rain
    dailyRain_till_LastHour = dailyRain;                   // update the rain till last hour for next calculation
    
    // facny display for humans to comprehend
    Serial.println();
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":  Total Rain for the day = ");
    Serial.print(dailyRain,8);                            // the '8' ensures the required accuracy
    Serial.println(" inches");
    Serial.println();
    Serial.print("     :  Rain in last hour = ");
    Serial.print(hourlyRain,8);
    Serial.println(" inches");
    Serial.println();
    
    first = false;                                        // execute calculations only once per hour
  }
  
  if(now.hour()== 0) {
    dailyRain = 0.0;                                      // clear daily-rain at midnight
    dailyRain_till_LastHour = 0.0;                        // we do not want negative rain at 01:00
  }  
}                                                        // end of loop
