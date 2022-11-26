TARGETS=rainfall_seq rainfall_pt

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

rainfall_seq: rainfall_seq.cpp
	g++ -O3 -std=c++11 -Wall -o rainfall_seq rainfall_seq.cpp

rainfall_pt: rainfall_pt.cpp
	g++ -O3 -std=c++11 -Wall -pthread -o rainfall_pt rainfall_pt.cpp
