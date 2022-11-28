#include "rainfall_pt.hpp"
extern vector<vector<int>> elevation;
extern vector<vector<float>> rain_absorb;
extern float absorp_rate;
extern int time_steps;
extern int dimension;
extern struct timeval start_time;
extern struct timeval end_time;
extern int numThreads;

double calc_time(struct timeval start, struct timeval end);
void print_error_msg();
void print_result(int total_steps, double total_time, vector<vector<float>> &rain_absorb, int dimension);
bool read_file(string elevation_file, vector<vector<int>> &elevation);
int rainfall(vector<vector<int>> &elevation, vector<vector<float>> &rain_absorb, float absorp_rate, 
                int time_steps, int dimension, struct timeval &start_time, struct timeval &end_time);

int main(int argc, char *argv[]) {
    // Error format
    if (argc != 6) {
        print_error_msg();
        return EXIT_FAILURE;
    }
    // P = # of parallel threads to use. ignored in sequential code
    numThreads = stoi(argv[1]);
    // M = # of simulation time steps during which a rain drop will fall on each landscape point. 
    int M = stoi(argv[2]);
    // A = absorption rate
    float A = stof(argv[3]);
    // N = dimension of the landscape (NxN)
    int N = stoi(argv[4]);
    // elevation_file = name of input file that specifies the elevation of each point.
    string elevation_file = argv[5];
    
    elevation = vector<vector<int>>(N, vector<int>(N, 0));
    if (!read_file(elevation_file, elevation)) {
        return EXIT_FAILURE;
    }

    rain_absorb = vector<vector<float>>(N, vector<float>(N, 0));
    // struct timeval start_time, end_time;
    // int total_steps = rainfall(elevation, rain_absorb, A, M, N, start_time, end_time, numThreads);
    int total_steps = rainfall();
    double total_time = calc_time(start_time, end_time);
    print_result(total_steps, total_time / 1000000, rain_absorb, N);
    return EXIT_SUCCESS;
}