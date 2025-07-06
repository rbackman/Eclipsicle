all: simulate_bricks simulate_strip

simulate_bricks: simulate_bricks.cpp ../tests/FallingBricks.cpp
	g++ -std=c++17 simulate_bricks.cpp ../tests/FallingBricks.cpp -o simulate_bricks

simulate_strip: simulate_strip.cpp ../src/stripState.cpp ../src/animations.cpp ../src/shared.cpp ../src/parameterManager.cpp
	g++ -std=c++17 -I../src -I../../FastLED/src -I../../ArduinoJson/src -Iarduino_stub -DUSE_LEDS simulate_strip.cpp ../src/stripState.cpp ../src/animations.cpp ../src/shared.cpp ../src/parameterManager.cpp ../../FastLED/src/FastLED.cpp ../../FastLED/src/hsv2rgb.cpp ../../FastLED/src/noise.cpp -o simulate_strip

clean:
	rm -f simulate_bricks simulate_strip
