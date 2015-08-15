/*
 LiquidCrystal Library - Hobbytronics
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints "Hobbytronics" to the LCD
 and shows the time.
 This sketch is based on the Arduino sample sketch at 
 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 but with modifications  to the LCD contrast to make it 
 adjustable via software
 
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD R/W pin to Ground 
 * LCD VO pin (pin 3) to PWM pin 9
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2

 */

// include the library code:
#include <LiquidCrystal.h>

/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

 This example code is in the public domain.
 */
#define TIME_SEC 60 
#define LED_PIN 13

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  // declare pin 9 to be an output:
  pinMode(9, OUTPUT);  
  analogWrite(9, 100);   
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Moist: ");
   lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print("Reference: ");
  
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int dryValue = analogRead(A0);
  int dryRefValue = analogRead(A1);

  int moist = 100- 100L * dryValue / 1023;
  int ref = 100- 100L * dryRefValue / 1023;
  // print out the value you read:
  Serial.print("Dry value: ");
  Serial.println(dryValue);
  
  Serial.print("Ref limit: ");
  Serial.println(dryRefValue);
  Serial.println(moist);
 Serial.println(ref);
  lcd.clear();
  lcd.print("Moist: ");
   lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print("Reference: ");
  lcd.setCursor(7, 0);
  lcd.print(moist);

  lcd.setCursor(11, 1);
  lcd.print(ref);


  if(dryValue >= dryRefValue){
    digitalWrite(LED_PIN,HIGH);
  }else{
    digitalWrite(LED_PIN,LOW);
  }

  
  delay(500);        // delay in between reads for stability
}
