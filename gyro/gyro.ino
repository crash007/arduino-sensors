/* Copyright (C) 2012 Kristian Lauszus, TKJ Electronics. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com
 */

#include <Wire.h>

//#define RES 16384
#define RES 16384

/* IMU Data */
double accX, accY, accZ,acc;
double gyroX, gyroY, gyroZ;
double speed=0;
int16_t tempRaw;

double gyroXangle, gyroYangle; // Angle calculate using the gyro only
double compAngleX, compAngleY; // Calculated angle using a complementary filter
double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter

double offsetX=0, offsetY=0, offsetZ=0;
  
uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

// TODO: Make calibration routine

void setup() {
  Serial.begin(115200);
  Wire.begin();
#if ARDUINO >= 157
  Wire.setClock(400000UL); // Set I2C frequency to 400kHz
#else
  TWBR = ((F_CPU / 400000UL) - 16) / 2; // Set I2C frequency to 400kHz
#endif

  i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  //i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  i2cData[3] = 0b00000000;
  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode

  while (i2cRead(0x75, i2cData, 1));
  if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    while (1);
  }

  delay(100); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (i2cData[0] << 8) | i2cData[1];
  accY = (i2cData[2] << 8) | i2cData[3];
  accZ = (i2cData[4] << 8) | i2cData[5];

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;

  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;

  for(int i=0; i<2000; i++){
    
    while (i2cRead(0x3B, i2cData, 6));
    accX = (i2cData[0] << 8) | i2cData[1];
    accY = (i2cData[2] << 8) | i2cData[3];
    accZ = (i2cData[4] << 8) | i2cData[5];

    offsetX = (offsetX+accX)/2;
    offsetY = (offsetY+accY)/2;
    offsetZ = (offsetZ+accZ)/2;
  }

  offsetZ-=RES;

  Serial.print("OffsetX: ");
  Serial.println(offsetX);
  Serial.print("OffsetY: ");
  Serial.println(offsetY);
  Serial.print("OffsetZ: ");
  Serial.println(offsetZ);

  delay(3000);
  
  timer = micros();


}

void loop() {
  /* Update all the values */
  while (i2cRead(0x3B, i2cData, 14));
  accX = ((i2cData[0] << 8) | i2cData[1]);
  accY = ((i2cData[2] << 8) | i2cData[3]);
  accZ = ((i2cData[4] << 8) | i2cData[5]);
  tempRaw = (i2cData[6] << 8) | i2cData[7];
  gyroX = (i2cData[8] << 8) | i2cData[9];
  gyroY = (i2cData[10] << 8) | i2cData[11];
  gyroZ = (i2cData[12] << 8) | i2cData[13];

  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees

  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;

  double gyroXrate = gyroX / 131.0; // Convert to deg/s
  double gyroYrate = gyroY / 131.0; // Convert to deg/s


  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
  gyroYangle += gyroYrate * dt;

  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
  compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;


  accX = accX-offsetX;
  accY = accY-offsetY;
  //Serial.print(accZ);
  //Serial.print("\t");
  
  accZ -= offsetZ;
  acc = sqrt(accX * accX + accY * accY+accZ*accZ)/RES;
  
//  Serial.print(accX);
//  Serial.print("\t");
//  Serial.print(accY);
//  Serial.print("\t");
 // Serial.print(accZ);
 // Serial.print("\t");   
  //Serial.print(acc); 
  //Serial.print("\t");

  Serial.print((acc-1));
  Serial.print("\t");

  //speed=speed+(acc-1)*9.82*dt;
  //Serial.print(speed );
  //Serial.print("\t");
  
  Serial.println(dt*1000);
  


#if 0 // Set to 1 to print the temperature
  Serial.print("\t");

  double temperature = (double)tempRaw / 340.0 + 36.53;
  Serial.print(temperature); Serial.print("\t");
#endif

  //Serial.print("\r\n");
  delay(18);

}
