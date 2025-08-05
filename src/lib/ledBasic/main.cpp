#ifdef LED_BASIC

#include <Arduino.h>
#include <FastLED.h>
#include "fx/1d/demoreel100.h"
#include "BasicInterpreter.h"

#define STRIP_PIN 5
#define NUM_LEDS 305 + 32

#define ONBOARD_PIN 48 // Big board r1: 48, nano: 21

#define FRAMES_PER_SECOND 120
#define BRIGHTNESS 128

CRGB leds[NUM_LEDS];
CRGB onboard[1];

fl::DemoReel100Ptr demoReel = fl::DemoReel100Ptr::New(NUM_LEDS);

// BASIC LED Controller instance
BasicLEDController *basicController;

// Virtual Strip Manager for layered animations
VirtualStripManager *stripManager;

CRGB Wheel(short WheelPos)
{
  if (WheelPos < 85)
  {
    return CRGB(
        (WheelPos * 3),
        (255 - WheelPos * 3),
        0);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return CRGB(
        (255 - WheelPos * 3),
        0,
        (WheelPos * 3));
  }
  else
  {
    WheelPos -= 170;
    return CRGB(
        0,
        (WheelPos * 3),
        (255 - WheelPos * 3));
  }
}

void DoubleRainbowForever()
{
  int ob = 0;

  while (1)
  {
    uint16_t i, j, k;
    k = 256;
    bool a = false;

    for (j = 0; j < 256; j++)
    {
      a = false;
      k--;

      for (i = 0; i < NUM_LEDS; i += 1)
      {
        a = !a;
        if (a)
          leds[i] = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
        else
          leds[i] = Wheel(((i * 256 / NUM_LEDS) + k) & 255);
      }

      FastLED.show();
      delay(20);

      if (j % 10 == 0)
      {
        onboard[0] = Wheel(ob);
        FastLED.show();

        ob += 1;
        if (ob > 255)
          ob = 0;
      }
    }
  }
}

bool test1 = true;
void ColorOrderTest()
{
  if (test1)
  {
    leds[0] = CRGB::White;

    leds[1] = CRGB::Red;

    leds[2] = CRGB::Green;
    leds[3] = CRGB::Green;

    leds[4] = CRGB::Blue;
    leds[5] = CRGB::Blue;
    leds[6] = CRGB::Blue;

    leds[7] = CRGB::White;

    for (int i = 8; i < NUM_LEDS; i += 1)
    {
      leds[i] = CRGB::Black;
    }

    onboard[0] = CRGB::Red;
    FastLED.show();
    delay(1000);

    onboard[0] = CRGB::Green;
    FastLED.show();
    delay(2000);

    onboard[0] = CRGB::Blue;
    FastLED.show();
    delay(2000);
  }
  else
  {
    onboard[0] = CRGB::White;

    for (int i = 0; i < NUM_LEDS; i += 1)
    {
      leds[i] = CRGB::White;
    }

    FastLED.show();
    delay(2000);
  }
  test1 = !test1;
}

void FastLEDDemoLoop()
{
  // Run the DemoReel100 draw function
  demoReel->draw(fl::Fx::DrawContext(millis(), leds));

  onboard[0] = Wheel((millis() / 100) % 255).scale8(64);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

// Example BASIC program for rainbow animation with parameters
const char *rainbowProgram = R"(
setup
  brightness(128)
  clear()
end

loop(time)
  for i = 0 to numled()-1
    h = (i * 360 / numled() + time / speed) % 360
    sethsv(i, h, saturation, brightness_level)
  next
  show()
end
)";

// Example BASIC program for sine wave animation with parameters
const char *sineWaveProgram = R"(
setup
  brightness(128)
  clear()
end

loop(time)
  for i = 0 to numled()-1
    wave = sin(i * frequency + time / speed) * amplitude + 128
    if color_mode == 0
      setled(i, wave, 0, 255 - wave)
    else
      setled(i, 0, wave, 255 - wave)
    end
  next
  show()
end
)";

// Example BASIC program for breathing effect with parameters
const char *breathingProgram = R"(
setup
  brightness(128)
  clear()
end

loop(time)
  breath = (sin(time / speed) + 1) * intensity
  if color_scheme == 0
    fill(breath, 0, 255 - breath)
  else
    if color_scheme == 1
      fill(0, breath, 255 - breath)
    else
      fill(breath, breath, 0)
    end
  end
  show()
end
)";

// Example BASIC program for double rainbow effect with parameters
const char *doubleRainbowProgram = R"(
setup
  brightness(128)
  clear()
end

loop(time)
  for i = 0 to numled()-1
    if i % 2 == 0
      // Even LEDs - forward rainbow
      h = (i * hue_spread / numled() + time / speed) % 360
    else
      // Odd LEDs - reverse rainbow
      h = (360 - (i * hue_spread / numled() + time / speed)) % 360
    end
    sethsv(i, h, saturation, brightness_level)
  next
  show()
end
)";

// Example BASIC program for Matrix digital rain effect with parameters
const char *matrixProgram = R"(
PARAM source ENUM "left,center,right" 1
PARAM flicker NUMBER 0 100 20 5
PARAM decay NUMBER 1 20 5 1
PARAM speed NUMBER 1 10 3 1

setup
  DIM led_count
  led_count = GET_LED_COUNT()

  CREATE_STRIP("flame", 0, led_count, 0)
  SET_BLEND_MODE("flame", 0) 

  DIM palette[15]
  palette[0] = 50
  palette[1] = 0
  palette[2] = 0 
  palette[3] = 255
  palette[4] = 64
  palette[5] = 0
  palette[6] = 255
  palette[7] = 128
  palette[8] = 0
  palette[9] = 255
  palette[10] = 255
  palette[11] = 128
  palette[12] = 255
  palette[13] = 255
  palette[14] = 255
end

LOOP
  FOR i = 0 TO led_count - 1
    DIM r
    r = GET_VIRTUAL_LED_R("flame", i)

    r = r - decay
    IF r < 0 THEN r = 0
    SET_VIRTUAL_LED("flame", i, r, 0, 0)
  NEXT

  DIM center
  IF source = 0 THEN
    center = 0
  ELSEIF source = 1 THEN
    center = led_count / 2
  ELSE
    center = led_count - 1
  END

  FOR i = 0 TO led_count - 1
    DIM dist
    dist = ABS(i - center)
    DIM chance
    chance = MAX(0, flicker - dist * 2)

    IF RANDOM(100) < chance THEN
      DIM heat
      heat = RANDOM(256)
      SET_VIRTUAL_LED("flame", i, heat, 0, 0)
    END
  NEXT

  FOR i = 0 TO led_count - 1
    DIM heat
    heat = GET_VIRTUAL_LED_R("flame", i)

    DIM idx
    idx = MIN(4, heat / 64)

    DIM r, g, b
    r = palette[idx * 3 + 0]
    g = palette[idx * 3 + 1]
    b = palette[idx * 3 + 2]

    r = r * heat / 255
    g = g * heat / 255
    b = b * heat / 255

    SET_LED(i, r, g, b)
  NEXT

  SHOW()
END
)";

// Virtual strip demo programs
const char *backgroundStarsProgram = R"(
setup
  brightness(100)
  clear()
end

loop(time)
  // Slowly twinkling background stars
  for i = 0 to numled()-1
    if random(1000) < 3
      twinkle = random(150) + 50
      setled(i, twinkle, twinkle, 255)
    else
      // Fade existing stars
      fadeAmount = random(10) + 5
      // Simplified fade - just dim a bit
      if random(100) < 20
        setled(i, 0, 0, 0)
      end
    end
  next
  show()
end
)";

const char *movingCometsProgram = R"(
setup
  brightness(200)
  clear()
  cometPos = 0
  cometSpeed = 0.3
end

loop(time)
  clear()
  
  // Move comet
  cometPos = cometPos + cometSpeed
  if cometPos >= numled() + 8
    cometPos = -8
  end
  
  // Draw comet with tail
  for i = 0 to 7
    pos = floor(cometPos) - i
    if pos >= 0 and pos < numled()
      intensity = 255 - i * 30
      setled(pos, intensity, intensity/2, 0)
    end
  next
  
  show()
end
)";

const char *pulsingCenterProgram = R"(
setup
  brightness(150)
  clear()
end

loop(time)
  clear()
  
  // Pulsing from center
  pulse = (sin(time / 500) + 1) * 127
  center = numled() / 2
  
  for i = 0 to center
    distance = abs(i - center)
    intensity = max(0, pulse - distance * 20)
    setled(center + i, 0, intensity, 0)
    setled(center - i, 0, intensity, 0)
  next
  
  show()
end
)";

void runBasicProgram()
{
  basicController->runLoop(millis());
}

void runVirtualStrips()
{
  stripManager->runAllLoops(millis());
  stripManager->renderToPhysical();
  FastLED.show();
}

void setupVirtualStripDemo()
{
  Serial.println("Setting up virtual strip demo...");

  // Create background stars layer (full strip, lowest Z-order)
  VirtualStrip *stars = stripManager->createStrip(0, NUM_LEDS, 0, BLEND_ADD);
  stars->loadProgram(String(backgroundStarsProgram));

  // Create moving comets on first half (additive blending, higher Z-order)
  VirtualStrip *comets = stripManager->createStrip(0, NUM_LEDS / 2, 10, BLEND_ADD);
  comets->loadProgram(String(movingCometsProgram));

  // Create pulsing center effect on second half (additive blending, highest Z-order)
  VirtualStrip *pulse = stripManager->createStrip(NUM_LEDS / 4, NUM_LEDS / 2, 20, BLEND_ADD);
  pulse->loadProgram(String(pulsingCenterProgram));

  // Run all setups
  stripManager->runAllSetups();

  Serial.println("Virtual strip demo ready!");
  Serial.println("Background stars: Full strip, Z=0, Additive");
  Serial.println("Moving comets: First half, Z=10, Additive");
  Serial.println("Pulsing center: Center region, Z=20, Additive");
}

void setupOverlappingDemo()
{
  Serial.println("Setting up overlapping regions demo...");

  // Clear existing strips
  stripManager->removeAllStrips();

  // Left side rainbow
  VirtualStrip *leftRainbow = stripManager->createStrip(0, NUM_LEDS * 2 / 3, 0, BLEND_REPLACE);
  leftRainbow->loadProgram(String(rainbowProgram));

  // Right side matrix (overlapping with rainbow)
  VirtualStrip *rightMatrix = stripManager->createStrip(NUM_LEDS / 3, NUM_LEDS * 2 / 3, 10, BLEND_ADD);
  rightMatrix->loadProgram(String(matrixProgram));

  stripManager->runAllSetups();

  Serial.println("Overlapping demo ready!");
  Serial.println("Left rainbow: 0 to 2/3, Z=0, Replace");
  Serial.println("Right matrix: 1/3 to end, Z=10, Additive");
}

void displayParameters()
{
  std::vector<Parameter> params = basicController->getAllParameters();
  if (params.empty())
  {
    Serial.println("No parameters available for current program.");
    return;
  }

  Serial.println("\n=== Program Parameters ===");
  for (const Parameter &param : params)
  {
    Serial.print("• ");
    Serial.print(param.name);
    Serial.print(": ");
    Serial.print(param.getStringValue());

    if (param.type == PARAM_NUMBER)
    {
      Serial.print(" (range: ");
      Serial.print(param.minValue);
      Serial.print(" - ");
      Serial.print(param.maxValue);
      Serial.print(", step: ");
      Serial.print(param.stepValue);
      Serial.print(")");
    }
    else if (param.type == PARAM_BOOLEAN)
    {
      Serial.print(" (boolean)");
    }
    else if (param.type == PARAM_ENUM)
    {
      Serial.print(" (options: ");
      for (int i = 0; i < param.enumValues.size(); i++)
      {
        if (i > 0)
          Serial.print(", ");
        Serial.print(param.enumValues[i]);
      }
      Serial.print(")");
    }
    Serial.println();
  }
  Serial.println("==========================\n");
}

void loadExampleProgram(int programIndex)
{
  const char *program = nullptr;
  bool useVirtualStrips = false;

  // Clear existing parameters
  basicController->clearParameters();

  switch (programIndex)
  {
  case 0:
    program = rainbowProgram;
    Serial.println("Loading rainbow program...");
    // Add parameters for rainbow
    basicController->addParameter(Parameter("speed", 20.0, 5.0, 100.0, 1.0));
    basicController->addParameter(Parameter("saturation", 255.0, 0.0, 255.0, 5.0));
    basicController->addParameter(Parameter("brightness_level", 255.0, 10.0, 255.0, 5.0));
    break;
  case 1:
  {
    program = sineWaveProgram;
    Serial.println("Loading sine wave program...");
    // Add parameters for sine wave
    basicController->addParameter(Parameter("speed", 200.0, 50.0, 1000.0, 10.0));
    basicController->addParameter(Parameter("frequency", 0.39, 0.1, 2.0, 0.05)); // PI/8 ≈ 0.39
    basicController->addParameter(Parameter("amplitude", 127.0, 50.0, 255.0, 5.0));
    std::vector<String> colorModes = {"Blue-Red", "Green-Red"};
    basicController->addParameter(Parameter("color_mode", colorModes, 0));
    break;
  }
  case 2:
  {
    program = breathingProgram;
    Serial.println("Loading breathing program...");
    // Add parameters for breathing
    basicController->addParameter(Parameter("speed", 1000.0, 200.0, 5000.0, 100.0));
    basicController->addParameter(Parameter("intensity", 127.0, 50.0, 255.0, 5.0));
    std::vector<String> schemes = {"Blue-Red", "Green-Red", "Yellow"};
    basicController->addParameter(Parameter("color_scheme", schemes, 0));
    break;
  }
  case 3:
    program = doubleRainbowProgram;
    Serial.println("Loading double rainbow program...");
    // Add parameters for double rainbow
    basicController->addParameter(Parameter("speed", 30.0, 10.0, 100.0, 1.0));
    basicController->addParameter(Parameter("hue_spread", 180.0, 90.0, 360.0, 10.0));
    basicController->addParameter(Parameter("saturation", 255.0, 100.0, 255.0, 5.0));
    basicController->addParameter(Parameter("brightness_level", 255.0, 50.0, 255.0, 5.0));
    break;
  case 4:
    program = matrixProgram;
    Serial.println("Loading Matrix digital rain program...");
    // Add parameters for Matrix effect
    basicController->addParameter(Parameter("speed", 3.0, 1.0, 10.0, 1.0));
    basicController->addParameter(Parameter("density", 30.0, 5.0, 80.0, 5.0));
    basicController->addParameter(Parameter("tail_length", 4.0, 2.0, 10.0, 1.0));
    basicController->addParameter(Parameter("spawn_rate", 2000.0, 500.0, 5000.0, 100.0));
    break;
  case 5:
    Serial.println("Loading virtual strip layered demo...");
    stripManager->removeAllStrips();
    setupVirtualStripDemo();
    useVirtualStrips = true;
    break;
  case 6:
    Serial.println("Loading overlapping regions demo...");
    setupOverlappingDemo();
    useVirtualStrips = true;
    break;
  default:
    program = rainbowProgram;
    Serial.println("Loading default rainbow program...");
    // Default rainbow parameters
    basicController->addParameter(Parameter("speed", 20.0, 5.0, 100.0, 1.0));
    basicController->addParameter(Parameter("saturation", 255.0, 0.0, 255.0, 5.0));
    basicController->addParameter(Parameter("brightness_level", 255.0, 10.0, 255.0, 5.0));
    break;
  }

  if (!useVirtualStrips && program)
  {
    // Clear virtual strips when using single program mode
    stripManager->removeAllStrips();

    if (basicController->loadProgram(String(program)))
    {
      Serial.println("Program loaded successfully!");
      basicController->runSetup();
      displayParameters();
    }
    else
    {
      Serial.println("Failed to load program!");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("ESP32 BASIC LED Controller with Virtual Strips Starting...");

  FastLED.addLeds<WS2812B, STRIP_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812B, ONBOARD_PIN, GRB>(onboard, 1);

  FastLED.setBrightness(BRIGHTNESS);

  leds[0] = CRGB::ForestGreen;
  onboard[0] = CRGB::Blue;

  FastLED.show();

  // Initialize BASIC interpreter
  basicController = new BasicLEDController(leds, NUM_LEDS);

  // Initialize Virtual Strip Manager
  stripManager = new VirtualStripManager(leds, NUM_LEDS);

  // Load and run the default rainbow program
  loadExampleProgram(0);

  Serial.println("Setup complete! Use Serial Monitor to change programs:");
  Serial.println("Send '0' for Rainbow");
  Serial.println("Send '1' for Sine Wave");
  Serial.println("Send '2' for Breathing");
  Serial.println("Send '3' for Double Rainbow");
  Serial.println("Send '4' for Matrix Digital Rain");
  Serial.println("Send '5' for Virtual Strip Layered Demo");
  Serial.println("Send '6' for Overlapping Regions Demo");
  Serial.println("Send 'p' to show current parameters");
  Serial.println("Send 'param_name=value' to adjust parameters");
}

void loop()
{
  // Check for serial input to change programs or adjust parameters
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Check if it's a program number
    if (input.length() == 1 && isdigit(input[0]))
    {
      int programIndex = input.toInt();
      if (programIndex >= 0 && programIndex <= 6)
      {
        loadExampleProgram(programIndex);
      }
    }
    // Check if it's parameter display command
    else if (input == "p" || input == "P")
    {
      displayParameters();
    }
    // Check if it's a parameter assignment
    else if (input.indexOf('=') > 0)
    {
      int equalPos = input.indexOf('=');
      String paramName = input.substring(0, equalPos);
      String paramValue = input.substring(equalPos + 1);

      paramName.trim();
      paramValue.trim();

      Parameter *param = basicController->getParameter(paramName);
      if (param)
      {
        if (param->type == PARAM_BOOLEAN)
        {
          bool value = (paramValue == "true" || paramValue == "1" || paramValue == "on");
          basicController->setParameterValue(paramName, Value(value ? 1.0 : 0.0));
          Serial.print("Set ");
          Serial.print(paramName);
          Serial.print(" = ");
          Serial.println(value ? "true" : "false");
        }
        else if (param->type == PARAM_NUMBER)
        {
          double value = paramValue.toDouble();
          basicController->setParameterValue(paramName, Value(value));
          Serial.print("Set ");
          Serial.print(paramName);
          Serial.print(" = ");
          Serial.println(value);
        }
        else if (param->type == PARAM_ENUM)
        {
          // Try to find the enum value by name
          int index = -1;
          for (int i = 0; i < param->enumValues.size(); i++)
          {
            if (param->enumValues[i].equalsIgnoreCase(paramValue))
            {
              index = i;
              break;
            }
          }
          // If not found by name, try by index
          if (index == -1)
          {
            index = paramValue.toInt();
            if (index < 0 || index >= param->enumValues.size())
            {
              index = -1;
            }
          }

          if (index >= 0)
          {
            basicController->setParameterValue(paramName, Value((double)index));
            Serial.print("Set ");
            Serial.print(paramName);
            Serial.print(" = ");
            Serial.println(param->enumValues[index]);
          }
          else
          {
            Serial.print("Invalid value for ");
            Serial.print(paramName);
            Serial.print(". Valid options: ");
            for (int i = 0; i < param->enumValues.size(); i++)
            {
              if (i > 0)
                Serial.print(", ");
              Serial.print(param->enumValues[i]);
            }
            Serial.println();
          }
        }
      }
      else
      {
        Serial.print("Parameter '");
        Serial.print(paramName);
        Serial.println("' not found. Send 'p' to see available parameters.");
      }
    }

    // Clear the serial buffer
    while (Serial.available())
    {
      Serial.read();
    }
  }

  // Check if we have active virtual strips
  if (stripManager->getStripCount() > 0)
  {
    // Run virtual strip system
    runVirtualStrips();
  }
  else
  {
    // Run single BASIC program
    runBasicProgram();
  }

  // Update onboard LED
  onboard[0] = Wheel((millis() / 100) % 255).scale8(64);

  // Keep framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // Alternative programs (comment out the BASIC program above to use these)
  // FastLEDDemoLoop();
  // ColorOrderTest();
  // DoubleRainbowForever();
}

#endif