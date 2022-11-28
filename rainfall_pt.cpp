#include "rainfall_pt.hpp"

int P;
int M;
float A;
int N;

int steps;
bool is_dry;
vector<vector<float>> rain_drops;
vector<vector<float>> new_rain_drops;
vector<vector<float>> temp_trickle;
vector<vector<float>> zero_trickle;
vector<vector<pair<int, int>>> trickle_neighs_list;
vector<vector<float>> rain_absorb;
vector<vector<std::unique_ptr<mutex>>> locks;

double calc_time(struct timeval start, struct timeval end);
void print_error_msg();
void print_result(int total_steps, double total_time, vector<vector<float>> &rain_absorb, int dimension);
bool read_file(string elevation_file, vector<vector<int>> &elevation);

void init_locks() {
    for (int i = 0; i < N; i++) {
        vector<std::unique_ptr<mutex>> row_locks;
        for (int j = 0; j < N; j++) {
            auto lock = make_unique<mutex>();
            row_locks.push_back(std::move(lock));
        }
        locks.push_back(std::move(row_locks));
    }
}

void first_traverse(int thread_id) {
    vector<pair<int, int>> trickle_neighs;
    int start = thread_id * (N / P);
    int end = (thread_id + 1) * (N / P);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            // rain fall
            if (steps <= M) {
                rain_drops[i][j]++;
            }
            // absorb into the ground
            if (rain_drops[i][j] >= A) {
                rain_absorb[i][j] += A;
                rain_drops[i][j] -= A;
            } else {
                rain_absorb[i][j] += rain_drops[i][j];
                rain_drops[i][j] = 0;
            }
            new_rain_drops[i][j] = rain_drops[i][j];
            // get the trickle neighbours list
            if (rain_drops[i][j] == 0) {
                continue;
            }
            float trickle_drops = (rain_drops[i][j] >= 1) ? 1 : rain_drops[i][j];
            trickle_neighs = trickle_neighs_list[i * N + j];
            if (trickle_neighs.size() == 0) {
                continue;
            } else if (trickle_neighs.size() == 1) {
                auto neighbour = trickle_neighs[0];
                locks[neighbour.first][neighbour.second]->lock();
                temp_trickle[neighbour.first][neighbour.second] += trickle_drops;
                locks[neighbour.first][neighbour.second]->unlock();
                new_rain_drops[i][j] -= trickle_drops;
            } else {
                for (auto neighbour : trickle_neighs) {
                    locks[neighbour.first][neighbour.second]->lock();
                    temp_trickle[neighbour.first][neighbour.second] +=  1.0 * trickle_drops / trickle_neighs.size();
                    locks[neighbour.first][neighbour.second]->unlock();
                }
                new_rain_drops[i][j] -= trickle_drops;
            }
        }
    }
}

void second_traverse(int thread_id) {
    int start = thread_id * (N / P);
    int end = (thread_id + 1) * (N / P);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < N; j++) {
            new_rain_drops[i][j] += temp_trickle[i][j];
            if (new_rain_drops[i][j] > 0) {
                is_dry = false;
            }
        }
    }
}

void rainfall(vector<vector<int>> &elevation, struct timeval &start_time, struct timeval &end_time) {
    rain_drops = vector<vector<float>>(N, vector<float>(N, 0));
    new_rain_drops = vector<vector<float>>(N, vector<float>(N, 0));
    temp_trickle = vector<vector<float>>(N, vector<float>(N, 0));
    zero_trickle = vector<vector<float>>(N, vector<float>(N, 0));
    trickle_neighs_list = trickle_list(elevation, N);
    init_locks();

    steps = 1; // total steps
    vector<thread> thread_pool;
    gettimeofday(&start_time, NULL);
    while (true) {
        is_dry = true;
        for (int i = 0; i < P; i++) {
            thread_pool.push_back(thread(first_traverse, i));
        }
        for (int i = 0; i < P; i++) {
            thread_pool[i].join();
        }
        thread_pool.clear();
        for (int i = 0; i < P; i++) {
            thread_pool.push_back(thread(second_traverse, i));
        }
        for (int i = 0; i < P; i++) {
            thread_pool[i].join();
        }
        thread_pool.clear();
        if (is_dry && steps > M) {
            gettimeofday(&end_time, NULL);
            break;
        }
        steps++;
        temp_trickle = zero_trickle;
        rain_drops = new_rain_drops;
    }
}

int main(int argc, char *argv[]) {
    // Error format
    if (argc != 6) {
        print_error_msg();
        return EXIT_FAILURE;
    }
    // P = # of parallel threads to use. ignored in sequential code
    P = stoi(argv[1]);
    // M = # of simulation time steps during which a rain drop will fall on each landscape point. 
    M = stoi(argv[2]);
    // A = absorption rate
    A = stof(argv[3]);
    // N = dimension of the landscape (NxN)
    N = stoi(argv[4]);
    // elevation_file = name of input file that specifies the elevation of each point.
    string elevation_file = argv[5];
    
    vector<vector<int>> elevation(N, vector<int>(N, 0));
    if (!read_file(elevation_file, elevation)) {
        return EXIT_FAILURE;
    }

    rain_absorb = vector<vector<float>>(N, vector<float>(N, 0));
    struct timeval start_time, end_time;
    rainfall(elevation, start_time, end_time);
    double total_time = calc_time(start_time, end_time);
    print_result(steps, total_time / 1000000, rain_absorb, N);
    return EXIT_SUCCESS;
}
