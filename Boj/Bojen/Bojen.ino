/*
  // March 2014 - TMRh20 - Updated along with High Speed RF24 Library fork
  // Parts derived from examples by J. Coliz <maniacbug@ymail.com>
*/
/**
 * Example for efficient call-response using ack-payloads 
 *
 * This example continues to make use of all the normal functionality of the radios including
 * the auto-ack and auto-retry features, but allows ack-payloads to be written optionally as well.
 * This allows very fast call-response communication, with the responding radio never having to 
 * switch out of Primary Receiver mode to send back a payload, but having the option to if wanting
 * to initiate communication instead of respond to a commmunication.
 */
 


#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 8 on the Arduino
#define ONE_WIRE_BUS 8

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 
RF24 radio(9,10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  

typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles


//Setup role
//

role_e role = role_ping_out;                                              // The role of the current running sketch
//role_e role = role_pong_back;

// A single byte to keep track of the data being sent back and forth
byte counter = 1;

struct bojenData{
  unsigned long _micros;
  float temp;
  float amplitude;
  float wavelength;
  float vcc;
  byte counter;
} data;

struct serverResponse{
  byte counter;
}response;


void setup(){

  Serial.begin(115200);
  printf_begin();
  //Serial.print(F("\n\rRF24/examples/pingpair_ack/\n\rROLE: "));
  Serial.println(role_friendly_name[role]);
  //Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  // Setup and configure rf radio

  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  //radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed

  data.counter =1;
  response.counter=1;
  
  if(role == role_ping_out){
    Serial.println("Setting up ping out role, bojen");
    radio.openWritingPipe(pipes[1]);        // Both radios listen on the same pipes by default, and switch when writing
    radio.openReadingPipe(1,pipes[0]);    
  }else{
    Serial.println("Setting up pong back role, server");
    radio.openWritingPipe(pipes[0]);        // Both radios listen on the same pipes by default, and switch when writing
    radio.openReadingPipe(1,pipes[1]);
  }
  
  
  radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  radio.setPALevel(RF24_PA_HIGH);
}

void loop(void) {

  if (role == role_ping_out){
    
    sensors.requestTemperatures();
    data.temp = sensors.getTempCByIndex(0);
    Serial.print("Temperature : ");
    Serial.println(data.temp);

    Serial.print("Now sending ");
    Serial.println(data.counter);
    
    radio.stopListening();                                  // First, stop listening so we can talk.
         
    unsigned long start_time = micros();                          // Take the time, and send it.  This will block until complete   
                                                            //Called when STANDBY-I mode is engaged (User is finished sending)
    if (!radio.write( &data, sizeof(data))){
      Serial.println(F("failed."));      
    }else{

      if(!radio.available()){ 
        Serial.println(F("Blank Payload Received.")); 
      }else{
        while(radio.available() ){
          unsigned long end_time = micros();
          radio.read( &response, sizeof(response) );
          //printf("Got response %d, round-trip delay: %lu microseconds\n\r",response.counter,end_time-start_time);
          Serial.print("Got response ");
          Serial.print(response.counter);
          Serial.print(" , round-trip delay: ");
          Serial.println(end_time-start_time);
          data.counter++;
        }
      }

    }
    // Try again later
    delay(1000);
  }

  // Pong back role.  Receive each packet, dump it out, and send it back

  if ( role == role_pong_back ) {
    byte pipeNo;
    
    while( radio.available(&pipeNo)){
      radio.read( &data, sizeof(data) );
      
      response.counter = data.counter;
      radio.writeAckPayload(pipeNo,&response, sizeof(response) );    
       Serial.print("Received packet counter : ");
      Serial.print(data.counter);
      Serial.print(" , temperature : ");
      Serial.println(data.temp);
      //response.counter++;
   }

     
 }

  // Change roles

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));

      role = role_ping_out;                  // Become the primary transmitter (ping out)
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      
       role = role_pong_back;                // Become the primary receiver (pong back)
       radio.openWritingPipe(pipes[1]);
       radio.openReadingPipe(1,pipes[0]);
       radio.startListening();
    }
    else if( c=='I')
    {
      radio.printDetails();
    }
    
  }
}
