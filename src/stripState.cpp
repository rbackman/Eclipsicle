

#include "FastLED.h"
#include "stripState.h"

int val = 0;
int minr = 0;
int maxr = 0;

led color;

StripState::StripState(CRGB row[], LED_STATE state, int numLEDS, int LED_PIN, int STRIP_INDEX, bool invert) : numLEDS(numLEDS), stripIndex(STRIP_INDEX), invert(invert)
{
    // ledRow = new LedRow(numLEDS);
    leds = row;

    ledState = state;
    spawnType.acceleration = 0;
    spawnType.active = false;
    spawnType.life = 60;
    spawnType.position = 0;
    spawnType.velocity = 2;
    spawnType.hueStart = 280;
    spawnType.hueEnd = 360;
    spawnType.brightness = 255;
    spawnType.width = 10;
}

void StripState::toggleMode()
{
    ledState = (LED_STATE)(((int)ledState + 1) % LED_STATE_COUNT);
    Serial.println("LED State: ");
    Serial.println(getLedStateName(ledState));
    this->clearPixels();
}

// void StripState::respondToSensor(sensor_message sensor)
// {

//     if (sensor.name == "Button1")
//     {
//         if (sensor.value == 1)
//         {
//             Serial.println("Button 1 pressed");
//             toggleMode();
//         }
//     }
//     if (ledState == LED_STATE_PARTICLES)
//     {
//         if (sensor.name == "Button3")
//         {
//             if (sensor.value == 1)
//             {
//                 Serial.println("Button 3 pressed");
//                 spawParticle();
//             }
//         }
//         if (sensor.name == "Button4")
//         {
//             if (sensor.value == 1)
//             {
//                 accelMode = !accelMode;
//             }
//         }
//         if (accelMode)
//         {
//             if (sensor.name == "Slider1")
//             {
//                 if (sensor.value > 0)
//                 {
//                     spawnType.velocity = map(sensor.value, 0, 255, 1, 10);
//                 }
//             }
//             if (sensor.name == "Slider2")
//             {
//                 if (sensor.value > 0)
//                 {
//                     spawnType.acceleration = map(sensor.value, 0, 255, -2, 20);
//                 }
//             }

//             if (sensor.name == "Slider3")
//             {
//                 if (sensor.value > 0)
//                 {
//                     spawnRate = map(sensor.value, 0, 255, 0, 20);
//                 }
//             }
//             if (sensor.name == "Slider4")
//             {
//                 if (sensor.value > 0)
//                 {
//                     spawnType.randomDrift = map(sensor.value, 0, 255, 0, 100);
//                 }
//             }
//         }
//         else
//         {
//             if (sensor.name == "Slider1")
//             {
//                 spawnType.width = map(sensor.value, 0, 255, 1, 20);
//             }

//             if (sensor.name == "Slider2")
//             {
//                 if (sensor.value > 0)
//                 {
//                     spawnType.hueStart = sensor.value;
//                 }
//             }
//             if (sensor.name == "Slider3")
//             {
//                 if (sensor.value > 0)
//                 {
//                     spawnType.hueEnd = sensor.value;
//                 }
//             }
//         }
//     }
//     if (ledState == LED_STATE_RAINBOW)
//     {
//         if (sensor.name == "Slider1")
//         {
//             scrollSpeed = sensor.value;
//         }

//         else if (sensor.name == "Slider3")
//         {
//             soundScale = sensor.value;
//         }
//     }
//     // else if (ledState == LED_STATE_RANDOM)
//     // {
//     //     if (sensor.name == "Slider1")
//     //     {
//     //         randomParams.randomOn = map(sensor.value, 255, 0, 0, 100);
//     //     }
//     //     if (sensor.name == "Slider2")
//     //     {
//     //         randomParams.randomOff = map(sensor.value, 255, 0, 0, 100);
//     //     }
//     //     if (sensor.name == "Slider3")
//     //     {
//     //         randomParams.randomMin = sensor.value;
//     //     }
//     //     if (sensor.name == "Slider4")
//     //     {
//     //         randomParams.randomMax = sensor.value;
//     //     }
//     // }
//     // else if (ledState == LED_STATE_IMAGE)
//     // {
//     //     if (sensor.name == "Slider1")
//     //     {
//     //         scrollPos = map(sensor.value, 0, 255, 0, ledMatrix->size());
//     //         LedRow row = (*ledMatrix)[scrollPos];

//     //         for (int i = 0; i < NUM_LEDS_PER_STRIP; i++)
//     //         {

//     //             if (i >= row.size())
//     //             {
//     //                 // Serial.println("Error: i out of bounds");
//     //                 return;
//     //             }

//     //             setPixel(i, row[i]);
//     //         }
//     //     }
//     //     else if (sensor.name == "Slider4")
//     //     {
//     //         brightness = map(sensor.value, 0, 255, 0, 255);
//     //         setBrightness(brightness);
//     //     }
//     // }
//     else if (ledState == LED_STATE_SLIDER)
//     {

//         //  int position = sensorManager->getSensorValue("Slider1", 1, 144);
//         // int width = (int)(sensorManager->getSensorValue("Slider2", 1, 144) * soundScale);
//         // float hueshift = ((float)sensorManager->getSensorValue("Slider3", 0, 360));
//         // float repeat = ((float)floor(sensorManager->getSensorValue("Slider4", 0, 100)) / 50.0);
//         // float brightness = ((float)sensorManager->getSensorValue("Slider5", 0, 255)) / 255.0;
//         // printf("sensor: %s position: %d, width: %d, hueshift: %f, repeat: %f, brightness: %f\n", sensor.name, position, width, hueshift, repeat);
//         // if (sensor.name == "Slider1")
//         // {
//         //     width = map(sensor.value, 0, 255, 144, 1);
//         // }
//         // if (sensor.name == "Slider2")
//         // {
//         //     if (centered)
//         //     {
//         //         sliderParams.position = map(sensor.value, 0, 255, -numLEDS / 2, numLEDS / 2);
//         //     }
//         //     else
//         //     {
//         //         position = map(sensor.value, 0, 255, 0, numLEDS);
//         //     }
//         //     position += gravityPosition;
//         // }
//         // if (sensor.name == "Slider3")
//         // {
//         //     hueshift = map(sensor.value, 0, 255, 0, 360);
//         // }
//         // if (sensor.name == "Slider4")
//         // {
//         //     repeat = map(sensor.value, 0, 255, 0, 100) / 50.0;
//         // }

//         // if (width == 0)
//         // {
//         //     width = 1;
//         // }
//     }

//     else if (ledState == LED_STATE_POINT_CONTROL)
//     {
//         printf("point control %s  %d   \n", sensor.name, sensor.value);
//         if (sensor.name == "Slider1")
//         {
//             // set single led for strip 1

//             for (int i = 0; i < MAX_LEDS_PER_STRIP; i++)
//             {

//                 if (i == sensor.value)
//                 {
//                     colorFromHSV(color, 0, 1, 1);
//                     setPixel(i, color);
//                 }
//                 else
//                 {
//                     clearPixel(i);
//                 }
//             }
//         }
//         if (sensor.name == "Slider2")
//         {

//             // set single led for strip 2
//             for (int i = 0; i < MAX_LEDS_PER_STRIP; i++)
//             {

//                 if (i == sensor.value)
//                 {
//                     colorFromHSV(color, 0, 1, 1);
//                     setPixel(i, color);
//                 }
//                 else
//                 {
//                     clearPixel(i);
//                 }
//             }
//         }
//     }
// }

void StripState::respondToParameter(parameter_message parameter)
{

    switch (parameter.paramID)
    {
    case PARAM_VELOCITY:
        spawnType.velocity = (float)parameter.value / 10.0;
        break;
    case PARAM_PARTICLE_RESET:
        spawnType.acceleration = 0;
        spawnType.velocity = 0.5;
        spawnType.hueStart = 280;
        spawnType.hueEnd = 360;
        spawnType.brightness = 255;
        spawnType.width = 10;
        spawnType.life = -1;
        spawnType.randomDrift = 0;
        timeScale = 1;
        cycle = false;

        break;
    case PARAM_ACCELERATION:
        spawnType.acceleration = parameter.value / 100.0;
        break;
    case PARAM_MAX_SPEED:
        spawnType.maxSpeed = parameter.value / 10.0;
        break;
    case PARAM_TIME_SCALE:
        timeScale = parameter.value / 10.0;
        break;

    case PARAM_PARTICLE_WIDTH:
        spawnType.width = parameter.value;
        break;
    case PARAM_PARTICLE_FADE:
        spawnType.fade = parameter.value;
        break;
    case PARAM_BRIGHTNESS:
        spawnType.brightness = parameter.value;
        break;
    case PARAM_SHOOT:
        spawnParticle();
        break;
    case PARAM_HUE:
        spawnType.hueStart = parameter.value;
        break;
    case PARAM_HUE_END:
        spawnType.hueEnd = parameter.value;
        break;
    case PARAM_SPAWN_RATE:
        spawnRate = parameter.value;
        break;
    case PARAM_RANDOM_DRIFT:
        spawnType.randomDrift = parameter.value;
        break;
    case PARAM_SLIDER_HUE:
        sliderParams.hueshift = parameter.value;
        break;
    case PARAM_SLIDER_POSITION:
        sliderParams.position = parameter.value * numLEDS / 255.0;
        break;
    case PARAM_SLIDER_REPEAT:
        sliderParams.repeat = parameter.value;
        break;
    case PARAM_SLIDER_WIDTH:
        sliderParams.width = parameter.value;
        break;
    case PARAM_SLIDER_GRAVITY:
        sliderParams.useGravity = parameter.value > 0;
        break;
    case PARAM_RANDOM_MAX:
        randomParams.randomMax = parameter.value;
        break;
    case PARAM_RANDOM_MIN:
        randomParams.randomMin = parameter.value;
        break;
    case PARAM_RANDOM_OFF:
        randomParams.randomOff = parameter.value;
        break;
    case PARAM_RANDOM_ON:
        randomParams.randomOn = parameter.value;
        break;
    case PARAM_INVERT:
        invert = parameter.value > 0;
        break;
    case PARAM_CENTERED:
        centered = parameter.value > 0;
        break;
    case PARAM_BLACK_AND_WHITE:
        blackAndWhite = parameter.value > 0;
        break;
    case PARAM_LOOP_ANIM:
        loopAnim = parameter.value > 0;
        break;
    case PARAM_MULTIPLIER:
        multiplier = parameter.value / 100.0;
        break;
    case PARAM_CYCLE:

        cycle = parameter.value > 0;
        break;
    case PARAM_ACCEL_MODE:
        accelMode = parameter.value > 0;
        break;
    case PARAM_SCROLL_SPEED:
        scrollSpeed = parameter.value;
        break;
    case PARAM_SOUND_SCALE:
        soundScale = parameter.value;
        break;

    default:
        Serial.println("Unknown parameter" + String(getParameterName(parameter.paramID)));
        break;
    }
}
void StripState::updateRandomParticles()
{

    if (random(0, 100) > 90)
    {
        float vel = random(10, 50) / 10.0;
        if (abs(vel) < 0.5)
        {
            vel = random(1, 2);
        }
        // random(0, 360), random(0, 255)
        int startHue = random(0, 360);
        int endHue = startHue + random(-60, 60);
        spawnParticle(0, vel, startHue, endHue, 200, 6, 60);
    }
    updateParticles();
}

void StripState::fadeParticleTail(int position, int width, int hueStart, int hueEnd, int brightness, float fadeSpeed, int direction)
{
    for (int j = 0; j < width; j++)
    {
        int index = position - j * direction;
        if (index >= 0 && index < numLEDS)
        {
            const int hue = interpolate(hueStart, hueEnd, (float)j / width);

            // Logarithmic decay for brightness
            float fadeFactor = log(1 + fadeSpeed * j) / log(1 + fadeSpeed * width);
            int adjustedBrightness = brightness * (1.0 - fadeFactor);

            colorFromHSV(color, hue / 360.0, 1.0, adjustedBrightness / 255.0);
            setPixel(index, color);
        }
    }
}
void StripState::updateParticles()
{

    for (int i = 0; i < 10; i++)
    {
        auto particle = &particles[i];
        if (particle->active)
        {


            if (ledState == LED_STATE_PARTICLES)
            {
                particle->hueStart = spawnType.hueStart;
                particle->hueEnd = spawnType.hueEnd;
                particle->brightness = spawnType.brightness;
                particle->fade = spawnType.fade;
                particle->width = spawnType.width;
                particle->life = spawnType.life;
                particle->randomDrift = spawnType.randomDrift;
                particle->acceleration = spawnType.acceleration;
                particle->maxSpeed = spawnType.maxSpeed;
                particle->velocity = spawnType.velocity;
            }

                
            if (particle->randomDrift != 0)
            {
                // roll the dice to see if it should switch direction of acceleration
                if (random(0, 100) > particle->randomDrift)
                {
                    particle->acceleration = -particle->acceleration;
                }
            }

            particle->position += particle->velocity * timeScale;
            particle->velocity += particle->acceleration * timeScale;
            if (particle->velocity > particle->maxSpeed)
            {
                particle->velocity = particle->maxSpeed;
            }
            if (particle->velocity < -particle->maxSpeed)
            {
                particle->velocity = -particle->maxSpeed;
            }

            if (particle->life != -1 && timeScale != 0)
            {
                particle->life--;
                if (particle->life <= 0)
                {
                    particle->active = false;
                }
            }

            int width = particle->width;
            if (particle->maxSpeed != 0)
            {
                width = (int)(particle->width * abs(particle->velocity) / particle->maxSpeed);
            }
            if (width < 1)
            {
                width = 1;
            }
            if (width > particle->width)
            {
                width = particle->width;
            }

            if (particle->velocity < -10)
            {
                particle->velocity = -10;
            }
            if (particle->velocity > 10)
            {
                particle->velocity = 10;
            }
            if (particle->randomDrift > 0)
            {
                if (random(0, 100) < particle->randomDrift / 10.0)
                {
                    particle->acceleration = -particle->acceleration;
                }
            }

            if (particle->position < -width)
            {
                if (cycle)
                    particle->position = numLEDS + width;
                else
                    particle->active = false;
            }
            else if (particle->position >= numLEDS + width)
            {
                if (cycle)
                    particle->position = -width;
                else
                    particle->active = false;
            }
            else
            {

                int direction = particle->velocity > 0 ? 1 : -1;

                fadeParticleTail(particle->position, width, particle->hueStart, particle->hueEnd, particle->brightness, particle->fade, direction);
            }
        }
    
      
    }
}
String StripState::getStripState()
{
    return getLedStateName(ledState);
}
void StripState::update()
{

    // Depending on what ledState is, you'll update the LEDs accordingly
    if (isVerbose())
    {
        // Serial.println("Updating LEDs" + String(getLedStateName(ledState)));
    }

    switch (ledState)
    {
    case LED_STATE_IDLE:
    {
        clearPixels();
    }

    break;
    case LED_STATE_PARTICLES:
    {

        clearPixels();
        if (spawnRate > 0 && timeScale != 0)
        {
            int ranVal = random(0, 100);

            if (ranVal < spawnRate)
            {

                spawnParticle();
            }
        }
        updateParticles();
    }
    break;
    case LED_STATE_RANDOM_PARTICLES:
    {
        clearPixels();
        updateRandomParticles();
    }
    break;
    case LED_STATE_SLIDER:
    {

        int centerPos = 0;
        if (sliderParams.useGravity)
            centerPos = (sliderParams.position + gravityPosition) % numLEDS;
        else
            centerPos = sliderParams.position % numLEDS;

        for (int i = 0; i < numLEDS; i++)
        {
            int distanceFromCenter = abs(i - centerPos);
            // check for overflow distance since it is a loop
            if (distanceFromCenter > numLEDS / 2)
            {
                distanceFromCenter = numLEDS - distanceFromCenter;
            }
            if (centered)
            {
                distanceFromCenter = abs(i - numLEDS / 2 - centerPos);
            }
            if (invertLEDs)
            {
                distanceFromCenter = sliderParams.width - distanceFromCenter;
            }

            // If pixel is outside the width, clear it
            if (distanceFromCenter > sliderParams.width * multiplier)
            {
                clearPixel(i);
            }
            else
            {

                // Calculate the hue for each pixel, based on its position relative to the center
                // The hue cycles 'repeat' times over the width of the strip

                float hue = fmod(sliderParams.hueshift + distanceFromCenter * sliderParams.repeat / 20.0 * (360.0 / sliderParams.width), 360.0);
                float fade = 1 - (distanceFromCenter / (sliderParams.width * multiplier));
                colorFromHSV(color, hue / 360.0, 1, fade);
                setPixel(i, color);
            }
        }
    }
    break;

    case LED_STATE_RAINBOW:
    {

        scrollPos += scrollSpeed;

        for (int i = 0; i < numLEDS; i++)
        {

            int val = i + scrollPos;
            colorFromHSV(color, float(val) / float(numLEDS), 1, 1);
            setPixel(i, color);
        }
    }
    break;

    // case LED_STATE_GRAVITY_WAVE:
    // {

    //     for (int i = 0; i < numLEDS; i++)
    //     {
    //         int distance = abs(i - gravityPosition);
    //         if (distance > numLEDS / 2)
    //         {
    //             distance = numLEDS - distance;
    //         }
    //         if (distance > 10)
    //         {
    //             clearPixel(i);
    //         }
    //         else
    //         {
    //             float val = 1 - (distance / numLEDS) * 10;
    //             colorFromHSV(color, val, 1, 0.5);
    //             setPixel(i, color);
    //         }
    //     }
    // }
    // break;
    case LED_STATE_RANDOM:
    {
        val = 0;
        minr = min(randomParams.randomMin, randomParams.randomMax);
        maxr = max(randomParams.randomMin, randomParams.randomMax);
        for (int i = 0; i < numLEDS; i++)
        {
            if (random(0, 100) > randomParams.randomOn)
            {
                if (random(0, 100) > randomParams.randomOff)
                {
                    val = random(minr, maxr);
                    colorFromHSV(color, float(val) / float(255), 1, 1);
                    setPixel(i, color);
                }
                else
                {
                    clearPixel(i);
                }
            }
        }
    }
    break;

    default:
        break;
    }

    // for (int i = 0; i < strips.size(); i++)
    // {
    //     strips[i]->show();
    // }

    // if (safeLight)
    // {
    //     setPixel(MAX_LEDS_PER_STRIP - 1, 255, 255, 255);
    // }
}

bool StripState::respondToText(String command)
{

    bool verbose = isVerbose();

    if (command.startsWith("menu:"))
    {
        String menuName = command.substring(5);

        if (menuName == "Particles")
        {

            ledState = LED_STATE_PARTICLES;
        }
        else if (menuName == "Random")
        {

            ledState = LED_STATE_RANDOM;
        }
        else if (menuName == "Slider")
        {

            ledState = LED_STATE_SLIDER;
        }
        else if (menuName == "Rainbow")
        {

            ledState = LED_STATE_RAINBOW;
        }
        else if (menuName == "Idle")
        {

            ledState = LED_STATE_IDLE;
        }
        else if (menuName == "RndParticles")
        {
            ledState = LED_STATE_RANDOM_PARTICLES;
        }

        else
        {
            return false;
        }
        Serial.printf("Set LED State:%d  %s", ledState, getLedStateName(ledState));
        return true;
    }
    return false;
}

void StripState::clearPixels()
{

    for (int i = 0; i < numLEDS; i++)
    {
        clearPixel(i);
    }
}

void StripState::setPixel(int index, led color)
{
    if (invert)

        leds[numLEDS - index - 1] = CRGB(color.r, color.g, color.b);

    else
        leds[index] = CRGB(color.r, color.g, color.b);
}
void StripState::setPixel(int index, int r, int g, int b)
{
    if (invert)

        leds[numLEDS - index - 1] = CRGB(r, g, b);

    else
        leds[index] = CRGB(r, g, b);
}
void StripState::clearPixel(int index)
{

    if (invert)
        leds[numLEDS - index - 1] = CRGB(0, 0, 0);
    else
        leds[index] = CRGB(0, 0, 0);
}
