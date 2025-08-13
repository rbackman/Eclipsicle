all: simulate_bricks simulate_strip simulate_rle led_sim

simulate_bricks: simulate_bricks.cpp ../tests/FallingBricks.cpp
	g++ -std=c++17 simulate_bricks.cpp ../tests/FallingBricks.cpp -o simulate_bricks

simulate_strip: simulate_strip.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp
	g++ -std=c++17 -I../src/lib -Iarduino_stub -DUSE_LEDS simulate_strip.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp -o simulate_strip

simulate_rle: simulate_rle.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp
	g++ -std=c++17 -I../src/lib -Iarduino_stub -DUSE_LEDS simulate_rle.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp -o simulate_rle

led_sim: led_sim/led_sim.so

led_sim/led_sim.so: led_sim/strip_sim.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp
	g++ -std=c++17 -shared -fPIC -I../src/lib -Iarduino_stub -DUSE_LEDS led_sim/strip_sim.cpp ../src/lib/stripState.cpp ../src/lib/animations.cpp ../src/lib/shared.cpp ../src/lib/parameterManager.cpp -o led_sim/led_sim.so

clean:
	rm -f simulate_bricks simulate_strip simulate_rle led_sim/led_sim.so
