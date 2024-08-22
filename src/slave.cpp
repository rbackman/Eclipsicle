#ifdef SLAVE_BOARD

#ifdef USE_LEDS

#include "ledManager.h"

#endif

#include "meshnet.h"

#include "sensors.h"
#include "serial.h"
#include "shared.h"
#ifdef USE_MOTOR
#include "motors.h"
#endif
#include <vector>
#include <string>
#ifdef USE_DISPLAY
#include <Adafruit_SSD1306.h>
#endif

#ifdef USE_ACCELEROMETER
#include <Adafruit_MPU6050.h>
#endif
#ifdef USE_SENSORS
#include <Adafruit_Sensor.h>
#endif
#include <Wire.h>

#ifdef USE_ACCELEROMETER
Adafruit_MPU6050 mpu;
#endif
const int buttonPin = 0;    // GPIO 0 is often used for the boot button
int lastButtonState = HIGH; // Assume button starts unpressed
bool showAccel = false;

#ifdef USE_DISPLAY
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
#endif

SerialManager *serialManager = new SerialManager(120, "Slave");
ParameterManager *parameterManager;

#ifdef USE_LEDS
LEDManager *ledManager;
#endif

MeshnetManager *meshManager;

#ifdef USE_MOTOR
MotorManager *motorManager;
#endif

#ifdef USE_LEDS
ImageHandler imageHandler = [](image_message msg)
{
  if (isVerbose())
    Serial.println("image received:  " + String(msg.numBytes));
  ledManager->setLEDImage(msg);
};

TextHandler textHandler = [](text_message msg)
{
  Serial.println("Text received: " + String(msg.text));

  ledManager->handleLEDCommand(msg.text);
};
#endif

#ifdef USE_SENSORS
SensorHandler sensorHandler = [](sensor_message msg)
{
#ifdef USE_MOTOR
  if (msg.name == "Slider5")
  {
    motorManager->setMotor(msg.value * 180 / 255);
  }
  else
  {
    if (isVerbose())
    {
      Serial.println("Sensor received: " + String(msg.name) + " " + String(msg.value));
    }
    ledManager->respondToSensor(msg);
  }
#else
  if (isVerbose())
  {
    Serial.println("Sensor received: " + String(getSensorName(msg.sensorId)) + " " + String(msg.value));
  }
#endif
#ifdef USE_LEDS
  // ledManager->respondToSensor(msg);
#endif
};

#endif

ParameterHandler parameterHandler = [](parameter_message msg)
{
  if (msg.paramID == PARAM_DISPLAY_ACCEL)
  {
    showAccel = msg.value != 0;
  }

  parameterManager->respondToParameterMessage(msg);
  ledManager->respondToParameterMessage(msg);
};

void setup()
{
  Serial.begin(115200);
  parameterManager = new ParameterManager("Slave", {PARAM_DISPLAY_ACCEL});

  if (isVerbose())
  {
    sanityCheckParameters();
  }
  ledManager = new LEDManager();

#ifdef USE_DISPLAY
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);

  // while (!Serial)
  //   delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 found display!");
#endif

#ifdef USE_ACCELEROMETER
  // setup accelerometer
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    // while (1)
    // {
    //   delay(10);
    // }
  }

  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("MPU6050 Found!");

  // setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
#endif

#ifdef USE_MOTOR
  motorManager = new MotorManager();
#endif

  meshManager = new MeshnetManager();

  meshManager->setImageHandler(imageHandler);
  meshManager->setTextHandler(textHandler);
  meshManager->setParameterHandler(parameterHandler);

#ifdef USE_SENSORS
  meshManager->setSensorHandler(sensorHandler);
#endif

  Serial.println("Free memory Setup: ");
  Serial.println(ESP.getFreeHeap());
}

void processCmd(String command)
{

  // ledManager->handleLEDCommand(command);
  // meshManager->handleMeshCommand(command);
  // checkLEDCommand(command);
  // checkSensorCommand(command);

  // if (command.startsWith("?"))
  // {
  //   String subCommand = command.substring(1);

  //   if (subCommand == "led")
  //   {
  //     Serial.println("led <pixel> <hue>");
  //   }
  //   else if (subCommand == "rand")
  //   {
  //     Serial.println("rand <on> <off> <on2> <min> <max>");
  //   }
  //   else if (subCommand == "anim")
  //   {
  //     Serial.println("anim <speed>");
  //   }
  //   else if (subCommand == "cent")
  //   {
  //     Serial.println("cent");
  //   }
  //   else if (subCommand == "verb")
  //   {
  //     Serial.println("verb");
  //   }
  //   else if (subCommand == "loop")
  //   {
  //     Serial.println("loop");
  //   }
  //   else if (subCommand == "strobe")
  //   {
  //     Serial.println("strobe <delay>");
  //   }
  //   else if (subCommand == "delay")
  //   {
  //     Serial.println("delay <delay>");
  //   }
  //   else if (subCommand == "brightness")
  //   {
  //     Serial.println("brightness <brightness>");
  //   }
  //   else if (subCommand == "dist")
  //   {
  //     Serial.println("dist");
  //   }
  //   else if (subCommand == "rand")
  //   {
  //     Serial.println("rand");
  //   }
  //   else if (subCommand == "rain")
  //   {
  //     Serial.println("rain");
  //   }
  //   else
  //   {
  //     Serial.println("Commands:");
  //     Serial.println("rain");
  //     Serial.println("idle");
  //     Serial.println("anim");
  //     Serial.println("rand");
  //     Serial.println("dist");
  //     Serial.println("brightness");
  //     Serial.println("delay");
  //     Serial.println("strobe");
  //     Serial.println("loop");
  //     Serial.println("verb");
  //     Serial.println("cent");
  //     Serial.println("anim");
  //     Serial.println("rand");
  //     Serial.println("led");
  //   }
  // }

  // if (cmd.startsWith("led"))
  // {
  //   int pixel = String(strtok(NULL, " ")).toInt();
  //   int hue = String(strtok(NULL, " ")).toInt();

  //   colorFromHSV(color, hue / 255.0, 1, 1);
  //   if (pixel >= 0 && pixel < NUM_LEDS)
  //   {
  //     Serial.println("Setting pixel " + String(pixel) + " to ");
  //     strip.setPixelColor(pixel, color.r, color.g, color.b);
  //     strip.show();
  //   }
  // }
  // else if (cmd.startsWith("cent"))
  // {
  //   centered = !centered;
  //   Serial.println("Centered" + centered);
  // }
  // else if (cmd.startsWith("rand"))
  // {
  //   ledState = LED_STATE_RANDOM;

  //   randomOn = String(strtok(NULL, " ")).toInt();
  //   randomOff = String(strtok(NULL, " ")).toInt();
  //   randomOn2 = String(strtok(NULL, " ")).toInt();
  //   randMin = String(strtok(NULL, " ")).toInt();
  //   randMax = String(strtok(NULL, " ")).toInt();

  //   Serial.println("Switched to RANDOM mode");
  // }

  // else if (cmd.startsWith("rain"))
  // {
  //   ledState = LED_STATE_RAINBOW;
  //   Serial.println("Switched to RAINBOW mode");
  // }
  // else if (cmd.startsWith("loop"))
  // {
  //   loopAnim = !loopAnim;
  // }
  // else if (cmd.startsWith("dist"))
  // {
  //   ledState = LED_STATE_DISTANCE_REACTIVE;
  //   Serial.println("Switched to DISTANCE_REACTIVE mode");
  // }
  // else if (cmd.startsWith("idle"))
  // {
  //   ledState = LED_STATE_IDLE;
  //   Serial.println("Switched to IDLE mode");
  //   for (size_t i = 0; i < NUM_LEDS; i++)
  //   {
  //     strip.setPixelColor(i, 0);
  //   }
  // }
  // else if (cmd.startsWith("anim"))
  // {
  //   scrollSpeed = String(strtok(NULL, " ")).toInt();
  //   ledState = LED_STATE_IMAGE_ANIMATED;
  //   currentRow = 0;
  //   Serial.println("Switched to IMAGE mode scroll: " + String(scrollSpeed));
  // }

  // else if (cmd.startsWith("strobe"))
  // {
  //   strobeDelay = String(strtok(NULL, " ")).toInt();
  // }
  // else if (cmd.startsWith("delay"))
  // {
  //   delayTime = String(strtok(NULL, " ")).toInt();
  // }

  // else if (cmd.startsWith("brightness"))
  // {
  //   brightness = String(strtok(NULL, " ")).toInt();
  //   Serial.println("Setting brightness to: " + String(brightness));
  //   strip.setBrightness(brightness);
  //   strip.show();
  // }
  // else if (cmd.startsWith("verb"))
  // {
  //   verbose = !verbose;
  //   Serial.println("Verbose: " + String(verbose));
  // }
}
int lastDisplayUpdate = 0;
int displayFrameRate = 10;
void loop()
{

  int buttonState = digitalRead(buttonPin);

  // Check if button state has changed from high to low (button press)
  if (buttonState == LOW && lastButtonState == HIGH)
  {

    ledManager->toggleMode();
    Serial.print("Switched to mode: ");
    Serial.println(ledManager->getStripState());
  }

  lastButtonState = buttonState; // Update the last button state

#ifdef USE_ACCELEROMETER
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  int curentTime = millis();
  if (curentTime - lastDisplayUpdate > 1000 / 60) // 1000 / displayFrameRate)
  {
    lastDisplayUpdate = curentTime;

    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;
    float magnitude = sqrt(ax * ax + ay * ay + az * az);
    ax /= magnitude;
    ay /= magnitude;
    az /= magnitude;

    // Calculate the angle in the x-y plane
    float angle = atan2(ay, ax); // Radians

    // Map the angle to an LED index
    float gravityPosition = ((angle + PI) / (2 * PI));

    ledManager->setGravityPosition(gravityPosition);

#ifdef USE_DISPLAY
    display.clearDisplay();
    display.setCursor(0, 0);
    if (showAccel)
    {

      display.print("Accel ");
      display.print(a.acceleration.x, 1);
      display.print(",");
      display.print(a.acceleration.y, 1);
      display.print(",");
      display.print(a.acceleration.z, 1);
      display.println("");

      display.print("Gyro");
      display.print(g.gyro.x, 1);
      display.print(",");
      display.print(g.gyro.y, 1);
      display.print(",");
      display.print(g.gyro.z, 1);
      display.println("");
    }
    display.print(ledManager->getStripState());

    display.display();
#endif
  }

  // display.clearDisplay();
  // display.setCursor(0, 0);

  // // Serial.print("Accelerometer ");
  // // Serial.print("X: ");
  // // Serial.print(a.acceleration.x, 1);
  // // Serial.print(" m/s^2, ");
  // // Serial.print("Y: ");
  // // Serial.print(a.acceleration.y, 1);
  // // Serial.print(" m/s^2, ");
  // // Serial.print("Z: ");
  // // Serial.print(a.acceleration.z, 1);
  // // Serial.println(" m/s^2");

  // display.println("Accelerometer - m/s^2");
  // display.print(a.acceleration.x, 1);
  // display.print(", ");
  // display.print(a.acceleration.y, 1);
  // display.print(", ");
  // display.print(a.acceleration.z, 1);
  // display.println("");

  // // Serial.print("Gyroscope ");
  // // Serial.print("X: ");
  // // Serial.print(g.gyro.x, 1);
  // // Serial.print(" rps, ");
  // // Serial.print("Y: ");
  // // Serial.print(g.gyro.y, 1);
  // // Serial.print(" rps, ");
  // // Serial.print("Z: ");
  // // Serial.print(g.gyro.z, 1);
  // // Serial.println(" rps");

  // display.println("Gyroscope - rps");
  // display.print(g.gyro.x, 1);
  // display.print(", ");
  // display.print(g.gyro.y, 1);
  // display.print(", ");
  // display.print(g.gyro.z, 1);
  // display.println("");

  // display.display();
  // delay(100);
#endif
  serialManager->updateSerial();
  if (serialManager->stringAvailable())
  {
    String command = serialManager->readString();
    Serial.println("Command: " + command);
    processCmd(command);
#ifdef USE_LEDS
    ledManager->handleLEDCommand(command);
#endif

#ifdef USE_MOTOR
    if (motorManager->handleMotorCommand(command))
#else
    if (false)
#endif
    {
      Serial.println("Motor command handled");
    }
    // #ifdef USE_LEDS
    //     else if (ledManager->handleLEDCommand(command))
    //     {
    //       // Serial.println("Led command handled");
    //     }

    // #endif

    // else
    // {
    //   if (isVerbose())
    //     Serial.println("Command not handled" + command);
    // }
  }
#ifdef USE_LEDS
  ledManager->update();
#endif

  delay(20);
};
#endif