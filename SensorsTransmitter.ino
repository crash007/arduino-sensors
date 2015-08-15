#include <DallasTemperature.h>
#include <OneWire.h>


//a=DHT 11 temp
//b=DHT 11 humid
//c=ds18b20 temp

#include <LowPower.h>
#include "DHT.h"
#include <VirtualWire.h>
#include <stdlib.h>

#define DHT_VCC 4
#define DHT_GND 6
#define DHTPIN 5

#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);



// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

#define DS18b20_VCC 11 
#define DS18b20_GND 10

const int led_pin = 13;

#define RF_TX 9
#define RF_VCC 8
#define RF_GND 7
#define RF_PTT_PIN 2
#define RF_RX_PIN 3

char buff[20];

void setup() {

  //RF
  pinMode(RF_VCC,OUTPUT);
  pinMode(RF_GND,OUTPUT);
  digitalWrite(RF_GND,LOW);
  digitalWrite(RF_VCC,HIGH);

  
  //DHT11 
  pinMode(DHT_VCC,OUTPUT);
  pinMode(DHT_GND,OUTPUT);
  digitalWrite(DHT_GND,LOW);
  
  //DS18b20
  pinMode(DS18b20_VCC,OUTPUT);
  pinMode(DS18b20_GND,OUTPUT);
  digitalWrite(DS18b20_VCC,HIGH);
  digitalWrite(DS18b20_GND,LOW);
  
  vw_set_tx_pin(RF_TX);
  vw_set_ptt_pin(RF_PTT_PIN);
  vw_set_rx_pin(RF_RX_PIN);
  vw_set_ptt_inverted(true); // Required for DR3100

  Serial.begin(9600);  // Debugging only
  Serial.println("setup");
  
  
  vw_setup(1000);   // Bits per sec
  
 LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

 sensors.begin();
  
}

void loop(void) {

  dht11ReadSend();
  //LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF); 
  ds18b20ReadSend();
  
  const char *msg = "Hello";
  digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
   vw_send((uint8_t *)msg, strlen(msg)); // Send control character 
   vw_wait_tx(); // Wait until the whole message is gone
   digitalWrite(led_pin, LOW);
 // LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 

}

void ds18b20ReadSend(){
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  float ds18b20temp = sensors.getTempCByIndex(0);

  char ds18b20tempChar[20]; 
  dtostrf(ds18b20temp,6, 2, ds18b20tempChar);
  //sprintf(buff,"DS18B20:%s",ds18b20tempChar);
 sprintf(buff,"c%s",ds18b20tempChar);

 Serial.println(ds18b20temp);
  Serial.print(buff);
  
  digitalWrite(led_pin, HIGH);
  vw_send((uint8_t *)buff, strlen(buff)); // Send control character 
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(led_pin, LOW);
  
}

void dht11ReadSend(){
  //start sensor
  digitalWrite(DHT_VCC,HIGH);
  dht.begin();
  
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); //Wait for the sensor
 
  int humidity = dht.readHumidity();
  int temp = dht.readTemperature();
  //float f = dht.readTemperature(true);
  digitalWrite(DHT_VCC,LOW); //shutdown sensor
  
  Serial.print("Humidity: ");
  Serial.println(humidity);
  
  
  // Check if any reads failed and exit early (to try again).
  if (!isnan(humidity) && !isnan(temp)) {
   
   //sprintf(buff,"DHT11:T:%d",temp);
   sprintf(buff,"a%d",temp);
   Serial.print(buff);
   digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
   vw_send((uint8_t *)buff, strlen(buff)); // Send control character 
   vw_wait_tx(); // Wait until the whole message is gone
   digitalWrite(led_pin, LOW);
   LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF); //Wait for the server
   
   //sprintf(buff,"DHT11:H:%d",humidity);
   sprintf(buff,"b%d",humidity);
   
   digitalWrite(led_pin, HIGH);
   vw_send((uint8_t *)buff, strlen(buff)); // Send
   vw_wait_tx(); // Wait until the whole message is gone
   digitalWrite(led_pin, LOW);
  
  }

  
}

