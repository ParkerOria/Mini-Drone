#include <Servo.h> // For controlling the ESCs

// Define the ESC objects for each motor
Servo esc1, esc2, esc3, esc4; // Front Left, Front Right, Back Left, Back Right

// FlySky Receiver Pins (PWM input channels)
// Left stick: throttle (vertical) and yaw (horizontal)
// Right stick: pitch (vertical) and roll (horizontal)
const int throttlePin = 3; // Throttle (Left Stick Vertical)
const int yawPin      = 4; // Yaw (Left Stick Horizontal)
const int pitchPin    = 2; // Pitch (Right Stick Vertical)
const int rollPin     = 5; // Roll (Right Stick Horizontal) 5 is the top right switch
void setup() {
  Serial.begin(9600); // For debugging (optional)

  // Attach ESCs to PWM output pins on the Arduino
  esc1.attach(9);  // ESC 1 (Front Left)
  esc2.attach(10); // ESC 2 (Front Right)
  esc3.attach(11); // ESC 3 (Back Left)
  esc4.attach(12); // ESC 4 (Back Right)
  
  calibrateESCs();
  // Set up receiver pins as inputs
  pinMode(throttlePin, INPUT);
  pinMode(yawPin,      INPUT);
  pinMode(pitchPin,    INPUT);
  pinMode(rollPin,     INPUT);
  
}

void loop() {
  // Read PWM signals from the receiver (expected range ~1000-2000 Âµs)
  int throttle = pulseIn(throttlePin, HIGH);
  int yaw      = pulseIn(yawPin,      HIGH);
  int pitch    = pulseIn(pitchPin,    HIGH);
  int roll     = pulseIn(rollPin,     HIGH);

  // Map the receiver inputs for control adjustments
  int rollAdjust  = map(roll,  1000, 2000, -50, 50);
  int pitchAdjust = map(pitch, 1000, 2000, -50, 50);
  int yawAdjust   = map(yaw,   1000, 2000, -50, 50);

  // Mixing for an X-configuration quadcopter:
  int esc1Value = constrain(throttle +       pitchAdjust + rollAdjust - yawAdjust, 1000, 2000);  // Front Left
  int esc2Value = constrain(throttle - 100 + pitchAdjust - rollAdjust + yawAdjust, 1000, 2000);  // Front Right
  int esc3Value = constrain(throttle -       pitchAdjust + rollAdjust + yawAdjust, 1000, 2000);  // Back Left
  int esc4Value = constrain(throttle - 100 - pitchAdjust - rollAdjust - yawAdjust, 1000, 2000);  // Back Right

  // Send the PWM values to the ESCs using microsecond pulses
  esc1.writeMicroseconds(esc1Value);
  esc2.writeMicroseconds(esc2Value);
  esc3.writeMicroseconds(esc3Value);
  esc4.writeMicroseconds(esc4Value);

  // Optional: Debug output to the Serial Monitor
  Serial.print("Throttle: "); Serial.print(throttle);
  Serial.print(" | Yaw: ");   Serial.print(yaw);
  Serial.print(" | Pitch: "); Serial.print(pitch);
  Serial.print(" | Roll: ");  Serial.println(roll);
  
  Serial.print("ESC1: ");    Serial.print(esc1Value);
  Serial.print(" | ESC2: "); Serial.print(esc2Value);
  Serial.print(" | ESC3: "); Serial.print(esc3Value);
  Serial.print(" | ESC4: "); Serial.println(esc4Value);
   
  delay(20); // Small delay for stability
}
void calibrateESCs() {
  Serial.println("Starting ESC calibration routine...");

  // Step 1: Send maximum throttle signal to all ESCs.
  Serial.println("Sending maximum throttle...");
  esc1.writeMicroseconds(2000);
  esc2.writeMicroseconds(2000);
  esc3.writeMicroseconds(2000);
  esc4.writeMicroseconds(2000);
  
  // Hold max throttle for a couple of seconds.
  delay(2000);
  
  // Step 2: Send minimum throttle signal.
  Serial.println("Switching to minimum throttle...");
  esc1.writeMicroseconds(1000);
  esc2.writeMicroseconds(1000);
  esc3.writeMicroseconds(1000);
  esc4.writeMicroseconds(1000);
  
  // Hold minimum throttle for a couple of seconds to complete calibration.
  delay(2000);
  
  Serial.println("Calibration complete.");
}