all: falling_bricks_test

falling_bricks_test: FallingBricks.cpp falling_bricks_test.cpp
	g++ -std=c++17 FallingBricks.cpp falling_bricks_test.cpp -lgtest -lgtest_main -lpthread -o falling_bricks_test

clean:
	rm -f falling_bricks_test
