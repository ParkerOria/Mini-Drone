#include <SFE_BMP180.h>
#include <Wire.h>
#include <Servo.h>

/*FIXME: 
take this https://www.instructables.com/ESC-Programming-on-Arduino-Hobbyking-ESC/
esc readytosky 30 amp arduino < ESC
Readytosky Brushless Motors for F330 F450 F550 S500 S550 X525 DJI < motor
*/

// You will need to create an SFE_BMP180 object, here called "pressure":

SFE_BMP180 pressure;

//////////////////////////////////////////
//      Controller                      //
int throttlePin = 2;   // Pin for throttle (PWM signal)
int rollPin = 3;       // Pin for roll (PWM signal)
int pitchPin = 4;      // Pin for pitch (PWM signal)
int yawPin = 5;        // Pin for yaw (PWM signal)

// FLYSKY OUTPUT pt 2 //
int enA = 5;
int in1 = 2;
int in2 = 3;
int enB = 6;
int in3 = 4;
int in4 = 7;

int receiver_pins[] = {A0, A1, A2, A3, A4, A5};
int receiver_values[] = {0, 0, 0, 0, 0, 0};
int res_min = 950;
int res_max = 2020;

int working_range = 255; // motor driver range

boolean prt = true;

int mode = 0;
//-1 = transmitter not connected or out of range
// 0 = transmitter connected and ready
// 1 = slow speed mode
// 2 = high speed mode
//////////////////////////////////////////

//////////////////////////////////////////
//            ESC                       //
Servo myESC;  // Create an ESC object
int servoPin = 9; // Define the signal pin (e.g., 9)
int value = 0; 
//////////////////////////////////////////

#define ALTITUDE 61.0 // Altitude of Fullerton

void setup()
{
  Serial.begin(9600);
  Serial.println("REBOOT");

  ////////////////////////////
  //       Controller       //
  pinMode(throttlePin, INPUT);
  pinMode(rollPin, INPUT);
  pinMode(pitchPin, INPUT);
  pinMode(yawPin, INPUT);

  // FLYSKY OUTPUT pt 2 //
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  Serial.begin(115200);
  ////////////////////////////

  ////////////////////////////
  //        ESC             //
  myESC.attach(servoPin); // Attach the ESC to the pin
  // Optional: Send a "stop" signal to arm the ESC
  myESC.writeMicroseconds(value); // Or your ESC's neutral value
  
  delay(7000); // Delay to allow the ESC to recognize the signal
  ////////////////////////////

  // Initialize the sensor (it is important to get calibration values stored on the device).

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}

void loop()
{
  char status;
  double T,P,p0,a;

  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  /////////////////////////////////////
  //          CONTROLLER             //
  long throttle = pulseIn(throttlePin, HIGH);  // Read throttle PWM signal
  long roll = pulseIn(rollPin, HIGH);          // Read roll PWM signal
  long pitch = pulseIn(pitchPin, HIGH);        // Read pitch PWM signal
  long yaw = pulseIn(yawPin, HIGH);            // Read yaw PWM signal

  Serial.print("Throttle: ");
  Serial.print(throttle);
  Serial.print(" Roll: ");
  Serial.print(roll);
  Serial.print(" Pitch: ");
  Serial.print(pitch);
  Serial.print(" Yaw: ");
  Serial.println(yaw);
  
  delay(100);  // Delay for stability

  // FLYSKY CONTROLLER OUTPUT pt 2 //
    receive();

  int m1 = 0;
  int m2 = 0;

  int rot = receiver_values[0];

  if (mode == 1) {
    m1 = receiver_values[1] / 2 + (rot)/1.5;
    m2 = receiver_values[1] / 2 - (rot)/1.5;

  } else if (mode == 2) {

    m1 = receiver_values[1] + rot / 1.75;
    m2 = receiver_values[1] - rot / 1.75;
  }

  Serial.println(m1);
  Serial.println(m2);
  
  mpower(1,  m1);
  mpower(2,  m2);
  ////////////////////////////////////

  ////////////////////////////////////
  //          ESC                   //
  // Example: Control motor speed with a potentiometer
  int potVal = analogRead(A0); // Read potentiometer value
  int pwmVal = map(potVal, 0, 1023, 1100, 1900); // Map to PWM range
  myESC.writeMicroseconds(pwmVal); // Send PWM signal to ESC

  if(Serial.available()) {
    value = Serial.parseInt(); 
  }

  delay(20); // Small delay
  ////////////////////////////////////

  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      // Serial.print("temperature: ");
      // Serial.print(T,2);
      // Serial.print(" deg C, ");
      // Serial.print((9.0/5.0)*T+32.0,2);
      // Serial.println(" deg F");
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of 
        // pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current 
          // altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");

        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  delay(5000);  // Pause for 5 seconds.
}

int rp = 0;

/////////////////////////////////
// FLYSKY CONTROLLER Pt 2      //
void receive() {
  receiver_values[rp] = map(pulseIn (receiver_pins[rp], HIGH), res_min, res_max, -1 * working_range, working_range);
  rp++;
  if (rp == 6){
    rp = 0;
  }
  boolean activevalues = true;
  for (int i = 0; i < 6; i++) {
    if (prt) {
      Serial.print("CH");
      Serial.print(i);
      Serial.print(" : ");
      Serial.print(receiver_values[i]);
      Serial.print(",\t");
    }
    if (receiver_values[i] < -500) {
      activevalues = false;
    }
  }
  mode = 0;
  if (!activevalues) {
    mode = -1;
  } else if (receiver_values[4] > -100) {
    mode = 2;
  } else if (receiver_values[5] > -100) {
    mode = 1;
  }
  if (prt) {
    Serial.println("");
  }
}

void mpower(int motor,  int spd) {
  int rotation = 0;
  if (spd > 0) {
    rotation = 1;
  } else if (spd < 0) {
    rotation = -1;
    spd *= -1;
  }
  if (spd > 255) {
    spd = 255;
  }
  int pwm;
  int pA;
  int pB;
  if (motor == 1) {
    pwm = enA;
    pA = in1;
    pB = in2;
  } else if (motor == 2) {
    pwm = enB;
    pA = in3;
    pB = in4;
  } else {
    return;
  }
  if (rotation == 0) {
    digitalWrite(pA, LOW);
    digitalWrite(pB, LOW);
  } else if (rotation == 1) {
    digitalWrite(pA, HIGH);
    digitalWrite(pB, LOW);
  } else if (rotation == -1) {
    digitalWrite(pA, LOW);
    digitalWrite(pB, HIGH);
  }
  analogWrite(pwm, spd);
}
//////////////////////////
