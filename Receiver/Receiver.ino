// receiver.pde
//
// Simple example of how to use VirtualWire to receive messages
// Implements a simplex (one-way) receiver with an Rx-B1 module
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: receiver.pde,v 1.3 2009/03/30 00:07:24 mikem Exp $

#include <VirtualWire.h>

const int led_pin = 13;
const int transmit_pin = 12;
const int transmit_en_pin = 3;


#define RF_VCC 8
#define RF_GND 5
#define RF_RECEIVE 7


void setup()
{
    //RF
    pinMode(RF_VCC,OUTPUT);
    pinMode(RF_GND,OUTPUT);
    
    digitalWrite(RF_VCC,HIGH);
    digitalWrite(RF_GND,LOW);
   
    delay(1000);
    Serial.begin(9600);	// Debugging only
    Serial.println("setup");

    // Initialise the IO and ISR
    vw_set_tx_pin(transmit_pin);
    vw_set_rx_pin(RF_RECEIVE);
    vw_set_ptt_pin(transmit_en_pin);
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(1000);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running

    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin,LOW);
}

void loop()
{
  digitalWrite(led_pin, LOW);
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
    	int i;
    
            digitalWrite(led_pin, HIGH); // Flash a light to show received good message
    	// Message with a good checksum received, dump it.
    	Serial.print("Got: ");
    	
    	for (i = 0; i < buflen; i++)
    	{
    	    //Serial.print(buf[i], HEX);
          Serial.write(buf[i]);
    	    //Serial.print(' ');
    	}
    	Serial.println();
            digitalWrite(led_pin, LOW);
    }
}
