

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


#define GYRO_SENS 16384

/* IMU Data */
double accX, accY, accZ,acc,accPrev;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;

double gyroXangle, gyroYangle; // Angle calculate using the gyro only
double compAngleX, compAngleY; // Calculated angle using a complementary filter

double offsetX=1184.1;
double offsetY=-415.20;
double offsetZ=4014.86;

  
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
  i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
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
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif

  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;

  timer = micros();
  
  int i;
  int d[1000];
  float meanAccX=0;
  float meanAccY=0;
  float meanAccZ=0;
  
  for(i=0; i<2000; i++)
  {
    while (i2cRead(0x3B, i2cData, 6));
    accX = ((i2cData[0] << 8) | i2cData[1]);
    accY = ((i2cData[2] << 8) | i2cData[3]);
    accZ = ((i2cData[4] << 8) | i2cData[5]);
    //meanAcc = (sqrt(accX * accX + accY * accY+accZ*accZ)+meanAcc)/2;
    meanAccX = (meanAccX+accX)/2;
    meanAccY = (meanAccY+accY)/2;
    meanAccZ = (meanAccZ+accZ)/2;
  }  
  
  //offset=meanAcc-GYRO_SENS;
  Serial.print("Mean: ");
  Serial.println(meanAccX);
  Serial.println(meanAccY);
  Serial.println(meanAccZ);
  accPrev=0;

}

void loop() {
//  /* Update all the values */
//  while (i2cRead(0x3B, i2cData, 14));
//  accX = ((i2cData[0] << 8) | i2cData[1]);
//  accY = ((i2cData[2] << 8) | i2cData[3]);
//  accZ = ((i2cData[4] << 8) | i2cData[5]);
//  tempRaw = (i2cData[6] << 8) | i2cData[7];
//  gyroX = (i2cData[8] << 8) | i2cData[9];
//  gyroY = (i2cData[10] << 8) | i2cData[11];
//  gyroZ = (i2cData[12] << 8) | i2cData[13];
//
//  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
//  timer = micros();
//
//  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
//  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
//  // It is then converted from radians to degrees
//
//  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
//  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
//
//  double gyroXrate = gyroX / 131.0; // Convert to deg/s
//  double gyroYrate = gyroY / 131.0; // Convert to deg/s
//
//
//  gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
//  gyroYangle += gyroYrate * dt;
//
//  compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
//  compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;
//
//
//  /* Print Data */
//#if 1 // Set to 1 to activate
//  acc = sqrt(accX * accX + accY * accY+accZ*accZ)/(GYRO_SENS+offset)*9.82*dt;
//
//  //if(acc > accPrev){
//  
//    Serial.print(acc); Serial.print("\t");
//    Serial.println(dt*1000);
//   //accPrev=acc;
//  //}
//  
//
//
////  Serial.print(gyroX); Serial.print("\t");
////  Serial.print(gyroY); Serial.print("\t");
////  Serial.print(gyroZ); Serial.print("\t");
//
////  Serial.print("\t");
//#endif
//
////  Serial.print(roll); Serial.print("\t");
////  Serial.print(gyroXangle); Serial.print("\t");
////  Serial.print(compAngleX); Serial.print("\t");
////  Serial.print(kalAngleX); Serial.print("\t");
////
////  Serial.print("\t");
////
////  Serial.print(pitch); Serial.print("\t");
////  Serial.print(gyroYangle); Serial.print("\t");
////  Serial.print(compAngleY); Serial.print("\t");
////  Serial.print(kalAngleY); Serial.print("\t");
//
//#if 0 // Set to 1 to print the temperature
//  Serial.print("\t");
//
//  double temperature = (double)tempRaw / 340.0 + 36.53;
//  Serial.print(temperature); Serial.print("\t");
//#endif
//
//  //Serial.print("\r\n");
//  delay(2);

}
