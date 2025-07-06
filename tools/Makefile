all: simulate_bricks

simulate_bricks: simulate_bricks.cpp ../tests/FallingBricks.cpp
	g++ -std=c++17 simulate_bricks.cpp ../tests/FallingBricks.cpp -o simulate_bricks

clean:
	rm -f simulate_bricks
