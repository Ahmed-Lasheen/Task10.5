#include <Wire.h>

//initiate gyro variables
char imuAddress = 0x68; 
char imuPower = 0x6B; 
char gyroHigh = 0x47;
char gyroLow = 0x48; 

void setup() {
  Wire.begin();
  Serial.begin(9600);

  // Wake up the MPU6050 as it starts in sleep mode
  Wire.beginTransmission(imuAddress);
  Wire.write(imuPower);
  Wire.write(0);
  Wire.endTransmission(true);
}

void loop() {
  short Yaw = Gyro_Reading();
  
  delay(100);
}

short Gyro_Reading() {
  Wire.beginTransmission(imuAddress);
  Wire.write(gyroHigh); // Request the high byte of Z-axis gyro data
  Wire.requestFrom(imuAddress, 2, true);

  short gyroZ = Wire.read() << 8; //shifting high 8bits to left 8 times to the 8MSB
  gyroZ |= Wire.read(); // now its ready to get 8 bits in the 8LSB

  return gyroZ;
}
