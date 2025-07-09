all: simulate_bricks simulate_strip simulate_rle

simulate_bricks: simulate_bricks.cpp ../tests/FallingBricks.cpp
	g++ -std=c++17 simulate_bricks.cpp ../tests/FallingBricks.cpp -o simulate_bricks

simulate_strip: simulate_strip.cpp ../src/stripState.cpp ../src/animations.cpp ../src/shared.cpp ../src/parameterManager.cpp
	g++ -std=c++17 -I../src -Iarduino_stub -DUSE_LEDS simulate_strip.cpp ../src/stripState.cpp ../src/animations.cpp ../src/shared.cpp ../src/parameterManager.cpp -o simulate_strip

simulate_rle: simulate_rle.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp
	g++ -std=c++17 -I../src/lib -Iarduino_stub -DUSE_LEDS simulate_rle.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp -o simulate_rle

clean:
	rm -f simulate_bricks simulate_strip simulate_rle
