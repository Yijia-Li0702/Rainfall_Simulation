TARGETS=rainfall_seq rainfall_pt

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

rainfall_seq: rainfall_seq.cpp
	g++ -O3 -std=c++14 -Wall -Werror -o rainfall_seq rainfall_seq.cpp

rainfall_pt: rainfall_pt.cpp
	g++ -g -O3 -std=c++14 -Wall -pthread -o rainfall_pt rainfall_pt.cpp -lpthread

