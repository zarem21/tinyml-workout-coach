#include <Arduino_BMI270_BMM150.h>

// Sample rate (Hz)
const int SAMPLE_RATE_HZ = 100;
const int SAMPLE_INTERVAL_MS = 1000 / SAMPLE_RATE_HZ;

unsigned long lastSampleTime_ms = 0;

void setup() {
  // Init serial
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Started");

  // Init IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");

  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  // CSV header
  Serial.println("timestamp_ms,ax,ay,az,gx,gy,gz");
}

void loop() {
  unsigned long currTime_ms = millis();

  // Fixed output rate
  if (currTime_ms - lastSampleTime_ms >= SAMPLE_INTERVAL_MS) {
    lastSampleTime_ms = currTime_ms;

    float ax, ay, az;
    float gx, gy, gz;

    // Read IMU
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(ax, ay, az);
      IMU.readGyroscope(gx, gy, gz);

      // Print CSV row
      Serial.print(currTime_ms);
      Serial.print(",");

      Serial.print(ax); Serial.print(",");
      Serial.print(ay); Serial.print(",");
      Serial.print(az); Serial.print(",");

      Serial.print(gx); Serial.print(",");
      Serial.print(gy); Serial.print(",");
      Serial.println(gz);
    }
  }
}