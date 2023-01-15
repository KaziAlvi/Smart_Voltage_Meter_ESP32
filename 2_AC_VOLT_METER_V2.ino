#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

char ssid[] = "KFG_EMS_WIFI";
char pass[] = "Kfg#2!EMS@Passw0rd";

//SERVER*********************************************
#define INFLUX "http://172.16.253.155:8086/write?db=kfg_ems"
#define DELAY 10000
String metrics;
void sendReadings(String post_data)
{
  String dburl = INFLUX;
  HTTPClient http;
  http.begin(dburl);
  http.POST(post_data);
  http.end();
}

WebServer server(80);

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  digitalWrite(2, HIGH);
  digitalWrite(25, HIGH);
  digitalWrite(26, HIGH);
  digitalWrite(21, HIGH);
  digitalWrite(5, HIGH);
  delay(500);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    //    delay(500);
  }
  Serial.println(WiFi.localIP());
  digitalWrite(25, LOW);
  delay(50);
  digitalWrite(26, LOW);
  delay(50);
  digitalWrite(21, LOW);
  delay(50);
  digitalWrite(5, LOW);
  delay(50);
}

/* 1- AC Voltage Measurement */

int VoltageAnalogInputPin = 34;             // Which pin to measure voltage Value (Pin A0 is reserved for button function)
int VoltageAnalogInputPin2 = 36;

float voltageSampleRead  = 0;               /* to read the value of a sample in analog including voltageOffset1 */
float voltageLastSample  = 0;               /* to count time for each sample. Technically 1 milli second 1 sample is taken */
float voltageSampleSum   = 0;               /* accumulation of sample readings */
float voltageSampleCount = 0;               /* to count number of sample. */
float voltageMean ;                         /* to calculate the average value from all samples, in analog values*/
float RMSVoltageMean ;                      /* square roof of voltageMean without offset value, in analog value*/
float adjustRMSVoltageMean;
float FinalRMSVoltage;                      /* final voltage value with offset value*/

float voltageSampleRead2  = 0;               /* to read the value of a sample in analog including voltageOffset1 */
float voltageLastSample2  = 0;               /* to count time for each sample. Technically 1 milli second 1 sample is taken */
float voltageSampleSum2   = 0;               /* accumulation of sample readings */
float voltageSampleCount2 = 0;               /* to count number of sample. */
float voltageMean2 ;                         /* to calculate the average value from all samples, in analog values*/
float RMSVoltageMean2 ;                      /* square roof of voltageMean without offset value, in analog value*/
float adjustRMSVoltageMean2;
float FinalRMSVoltage2;

/* 1.1- AC Voltage Offset */

float voltageOffset1 = 0.00 ;         // to Offset deviation and accuracy. Offset any fake current when no current operates.//
float voltageOffset2 = 0.00;          // too offset value due to calculation error from squared and square root.//

float voltageOffset1_2 = 0.00 ;
float voltageOffset2_2 = 0.00;

void setup()
{
  Serial.begin(115200);                             /* In order to see value in serial monitor */
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  pinMode(2, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(21, OUTPUT);
  Serial.print("IoT Energy");
  Serial.print("Meter");
  delay(2000);
}

void loop()
{
  /* 1- AC Voltage Measurement */
  if (micros() >= voltageLastSample + 1000 )                                                                     /* every 0.2 milli second taking 1 reading */
  {
    voltageSampleRead = (analogRead(VoltageAnalogInputPin) - 4096) + voltageOffset1;                           /* read the sample value including offset value*/
    voltageSampleSum = voltageSampleSum + sq(voltageSampleRead) ;                                             /* accumulate total analog values for each sample readings*/
    voltageSampleCount = voltageSampleCount + 1;                                                              /* to move on to the next following count */
    voltageLastSample = micros() ;                                                                            /* to reset the time again so that next cycle can start again*/
  }
  if (micros() >= voltageLastSample2 + 1000 )                                                                     /* every 0.2 milli second taking 1 reading */
  {
    voltageSampleRead2 = (analogRead(VoltageAnalogInputPin2) - 4096) + voltageOffset1_2;                           /* read the sample value including offset value*/
    voltageSampleSum2 = voltageSampleSum2 + sq(voltageSampleRead2) ;                                             /* accumulate total analog values for each sample readings*/
    voltageSampleCount2 = voltageSampleCount2 + 1;                                                              /* to move on to the next following count */
    voltageLastSample2 = micros() ;                                                                            /* to reset the time again so that next cycle can start again*/
  }

  if (voltageSampleCount == 1000)                                                                               /* after 4000 count or 800 milli seconds (0.8 second), do the calculation and display value*/
  {
    voltageMean = voltageSampleSum / voltageSampleCount;                                                      /* calculate average value of all sample readings taken*/
    RMSVoltageMean = (sqrt(voltageMean)) * 1.5;                                                               // The value X 1.5 means the ratio towards the module amplification.
    adjustRMSVoltageMean = RMSVoltageMean + voltageOffset2;                                                   /* square root of the average value including offset value */                                                                                                                                                       /* square root of the average value*/
    FinalRMSVoltage = RMSVoltageMean + voltageOffset2;                                                        /* this is the final RMS voltage*/
    if (FinalRMSVoltage <= 2.5)                                                                               /* to eliminate any possible ghost value*/
    {
      FinalRMSVoltage = 0;
    }
    float FinalRMSVoltage1 = FinalRMSVoltage / 10;
    Serial.print(" The Voltage RMS value is 1: ");
    Serial.print(FinalRMSVoltage1);
    Serial.println(" V ");
    String metrics = "SEM_VOLT,device=EMS2,sensor=Sensor01 value=" + String(FinalRMSVoltage1, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(1000);
    voltageSampleSum = 0;                                                                                     /* to reset accumulate sample values for the next cycle */
    voltageSampleCount = 0;                                                                                   /* to reset number of sample for the next cycle */
    if (FinalRMSVoltage1 > 200) {
      digitalWrite(25, HIGH);
      delay(250);
      digitalWrite(25, LOW);
    } else if (FinalRMSVoltage1 < 190) {
      digitalWrite(25, LOW);
      digitalWrite(21, HIGH);
      delay(250);
      digitalWrite(21, LOW);
    } else {
      digitalWrite(25, HIGH);
      digitalWrite(21, HIGH);
    }

  }

  if (voltageSampleCount2 == 1000)                                                                               /* after 4000 count or 800 milli seconds (0.8 second), do the calculation and display value*/
  {
    voltageMean2 = voltageSampleSum2 / voltageSampleCount2;                                                      /* calculate average value of all sample readings taken*/
    RMSVoltageMean2 = (sqrt(voltageMean2)) * 1.5;                                                               // The value X 1.5 means the ratio towards the module amplification.
    adjustRMSVoltageMean2 = RMSVoltageMean2 + voltageOffset2_2;                                                   /* square root of the average value including offset value */                                                                                                                                                       /* square root of the average value*/
    FinalRMSVoltage2 = RMSVoltageMean2 + voltageOffset2_2;                                                        /* this is the final RMS voltage*/
    if (FinalRMSVoltage2 <= 2.5)                                                                               /* to eliminate any possible ghost value*/
    {
      FinalRMSVoltage2 = 0;
    }
    float FinalRMSVoltage_2 = FinalRMSVoltage2 / 10;
    Serial.print(" The Voltage RMS value is 2: ");
    Serial.print(FinalRMSVoltage_2);
    Serial.println(" V ");
    String metrics = "SEM_VOLT,device=EMS2,sensor=Sensor02 value=" + String(FinalRMSVoltage_2, 2);
    sendReadings(metrics);
    Serial.println(metrics);
    delay(1000);
    voltageSampleSum2 = 0;                                                                                     /* to reset accumulate sample values for the next cycle */
    voltageSampleCount2 = 0;                                                                                   /* to reset number of sample for the next cycle */
    if (FinalRMSVoltage2 > 200) {
      digitalWrite(26, HIGH);
      delay(250);
      digitalWrite(26, LOW);
    } else if (FinalRMSVoltage2 < 190) {
      digitalWrite(26, LOW);
      digitalWrite(5, HIGH);
      delay(250);
      digitalWrite(5, LOW);
    } else {
      digitalWrite(26, HIGH);
      digitalWrite(5, HIGH);
    }
  }
}
