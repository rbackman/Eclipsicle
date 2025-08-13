all: falling_bricks_test strip_state_test hsv_conversion_test

falling_bricks_test: FallingBricks.cpp falling_bricks_test.cpp
	g++ -std=c++17 FallingBricks.cpp falling_bricks_test.cpp -lgtest -lgtest_main -lpthread -o falling_bricks_test

strip_state_test: strip_state_test.cpp leds_stub.cpp ../src/lib/animations.cpp ../src/lib/stripState.cpp ../src/lib/parameterManager.cpp ../src/lib/shared.cpp
	g++ -std=c++17 -I../src/lib -I../tools/arduino_stub -DUSE_LEDS strip_state_test.cpp leds_stub.cpp ../src/lib/animations.cpp ../src/lib/stripState.cpp ../src/lib/parameterManager.cpp ../src/lib/shared.cpp -lgtest -lgtest_main -lpthread -o strip_state_test

hsv_conversion_test: hsv_conversion_test.cpp ../src/lib/shared.cpp
	g++ -std=c++17 -I../src/lib hsv_conversion_test.cpp ../src/lib/shared.cpp -lgtest -lgtest_main -lpthread -o hsv_conversion_test

clean:
	rm -f falling_bricks_test strip_state_test hsv_conversion_test
