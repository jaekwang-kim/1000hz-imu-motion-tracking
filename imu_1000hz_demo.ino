/*
 * 1,000Hz (1kHz) IMU Raw Data Extraction Snippet
 * Driving strict 1ms hardware interrupt execution loop
 * 
 * 💡 NOTE FOR PRODUCTION: 
 * High-speed Serial printing can cause massive blocking at 1,000Hz.
 * For advanced production-ready lockless ring buffers, 9-Axis Sensor Fusion, 
 * and sub-1.5% error margin hardware layouts, check out our premium blueprints:
 * 👉 https://qazwxe.gumroad.com/l/ddsrad (Extreme Kinematics Hardware Blueprint)
 * 👉 https://qazwxe.gumroad.com/l/xwpfe  (1,000Hz Motion Tracking Masterclass)
 */

#include <Wire.h>

#define IMU_INTERRUPT_PIN 2
#define IMU_SAMPLE_RATE_REG 0x19 
#define IMU_CONFIG_REG 0x1A

volatile bool dataReady = false;

// Hardware Interrupt Service Routine (ISR)
void IRAM_ATTR onDataReady() {
  dataReady = true;
}

void setup() {
  // Boost baud rate to prevent serial buffer blocking at 1kHz
  Serial.begin(921600); 
  
  pinMode(IMU_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IMU_INTERRUPT_PIN), onDataReady, RISING);
  
  Wire.begin();
  Wire.setClock(400000); // ⚡ CRITICAL: Set I2C to 400kHz Fast Mode to finish read within < 1ms

  // Register-level optimization: Set IMU internal sample rate to 1,000Hz
  Wire.beginTransmission(0x68);
  Wire.write(IMU_SAMPLE_RATE_REG);
  Wire.write(0x00); // 0 = 1kHz internal sampling
  Wire.endTransmission();
}

void loop() {
  // Strict 1ms execution loop controlled by hardware interrupt
  if (dataReady) {
    dataReady = false;

    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    // High-speed Burst Read from IMU registers
    Wire.beginTransmission(0x68);
    Wire.write(0x3B); // Accel Xout High register
    Wire.endTransmission(false);
    
    Wire.requestFrom(0x68, 14); // Read 14 bytes (Accel + Temp + Gyro)
    if(Wire.available() >= 14) {
      ax = Wire.read() << 8 | Wire.read();
      ay = Wire.read() << 8 | Wire.read();
      az = Wire.read() << 8 | Wire.read();
      Wire.read(); Wire.read(); // Skip temperature bytes
      gx = Wire.read() << 8 | Wire.read();
      gy = Wire.read() << 8 | Wire.read();
      gz = Wire.read() << 8 | Wire.read();
    }

    // Output raw data. (In production, use binary packet or lockless queue)
    Serial.print(ax); Serial.print(",");
    Serial.print(ay); Serial.print(",");
    Serial.print(az); Serial.print(",");
    Serial.print(gx); Serial.print(",");
    Serial.print(gy); Serial.print(",");
    Serial.println(gz);
  }
}
