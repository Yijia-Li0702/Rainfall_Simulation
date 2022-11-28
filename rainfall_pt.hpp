#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <climits>
#include <sys/time.h>
#include <iomanip>
#include "ctpl_stl.h"

using namespace std;
vector<vector<int>> elevation;
vector<vector<float>> rain_absorb;
float absorp_rate;
int time_steps;
int dimension;
struct timeval start_time;
struct timeval end_time;
int numThreads;
vector<vector<float>> rain_drops;
vector<vector<float>> new_rain_drops;
vector<vector<float>> temp_trickle;
vector<vector<pair<int, int>>> trickle_neighs_list;
vector<pair<int, int>> trickle_neighs;
int steps;

double calc_time(struct timeval start, struct timeval end) {
    double start_sec = (double)start.tv_sec*1000000.0 + (double)start.tv_usec;
    double end_sec = (double)end.tv_sec*1000000.0 + (double)end.tv_usec;

    if (end_sec < start_sec) {
        return 0;
    } else {
        return end_sec - start_sec;
    }
};

void print_error_msg() {
    cout << "Error!" << endl;
    cout << "Command : ./rainfall_seq <P> <M> <A> <N> <elevation_file>" << endl;
    cout << "P = # of parallel threads to use." << endl;
    cout << "M = # of simulation time steps during which a rain drop will fall on each landscape"
            "point.In other words, 1 rain drop falls on each point during the first M steps of the"
            "simulation." << endl;
    cout << "A = absorption rate (specified as a floating point number). The amount of raindrops"
            "that are absorbed into the ground at a point during a timestep." << endl;
    cout << "P = # of parallel threads to use." << endl;
    cout << "N = dimension of the landscape (NxN)" << endl;
    cout << "elevation_file = name of input file that specifies the elevation of each point." << endl;
}

void print_result(int total_steps, double total_time, vector<vector<float>> &rain_absorb, int dimension) {
    cout << "Rainfall simulation completed in " << total_steps << " time steps" << endl;
    // cout.precision(8);
    cout << "Runtime = " << fixed << total_time << " seconds" << endl;
    cout << endl;
    cout << "The following grid shows the number of raindrops absorbed at each point:" << endl;
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            cout.width(8);
            cout << defaultfloat << rain_absorb[i][j];
        }
        cout << endl;
    }
}

void print_debug(vector<vector<float>> &rain_absorb, int dimension) {
    cout << "The following grid shows the number of raindrops absorbed at each point:" << endl;
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            cout.width(8);
            cout << rain_absorb[i][j];
        }
        cout << endl;
    }
}

bool read_file(string elevation_file, vector<vector<int>> &elevation) {
    fstream fin("./" + elevation_file);
    string line;
    int i = 0, item = 0;
    if (fin) {
        while (getline(fin, line)) {
            int j = 0;
            stringstream s(line);
            while (s >> item) {
                elevation[i][j] = item;
                j++;
            }
            i++;
        }
    } else {
        cout << "File doesn't exist, please check your input" << endl;
        return false;
    }
    return true;
}

void check_input(int central_elevation, int &min_elevation, int neigh_elevation,
                    unordered_map<int, vector<pair<int, int>>> &map, int i, int j) {
    // if neighbour's elevation is smaller, add to map
    if (neigh_elevation < central_elevation) {
        if (map.find(neigh_elevation) != map.end()) {
            auto& pair_list = map[neigh_elevation];
            pair_list.emplace_back(i, j);
        } else {
            vector<pair<int, int>> vec;
            vec.emplace_back(i, j);
            map[neigh_elevation] = vec;
        }
        if (neigh_elevation < min_elevation) {
            min_elevation = neigh_elevation;
        }
    }
}

vector<pair<int, int>> trickle(int i, int j, int dimension, vector<vector<int>> &elevation) {
    vector<pair<int, int>> trickle_neighs;
    unordered_map<int, vector<pair<int, int>>> map;
    int central_elevation = elevation[i][j];
    int min_elevation = INT_MAX;
    // north
    if (i - 1 >= 0) {
        check_input(central_elevation, min_elevation, elevation[i - 1][j], map, i - 1, j);
    }
    // south
    if (i + 1 < dimension) {
        check_input(central_elevation, min_elevation, elevation[i + 1][j], map, i + 1, j);
    }
    // west
    if (j - 1 >= 0) {
        check_input(central_elevation, min_elevation, elevation[i][j - 1], map, i, j - 1);
    }
    // east
    if (j + 1 < dimension) {
        check_input(central_elevation, min_elevation, elevation[i][j + 1], map, i, j + 1);
    }
    if (min_elevation != INT_MAX) {
        trickle_neighs = map[min_elevation];
    } 
    return trickle_neighs;
}

vector<vector<pair<int, int>>> trickle_list(vector<vector<int>> &elevation, int dimension) {
    vector<vector<pair<int, int>>> result;
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            vector<pair<int, int>> trickle_neighs = trickle(i, j, dimension, elevation);
            result.push_back(trickle_neighs);
        }
    }
    return result;
}

// int rainfall(vector<vector<int>> &elevation, vector<vector<float>> &rain_absorb, float absorp_rate, 
//                 int time_steps, int dimension, struct timeval &start_time, struct timeval &end_time, int numThreads) {
int rainfall() {
    
    mutex lock;
    ctpl::thread_pool pool(numThreads);

    rain_drops = vector<vector<float>>(dimension, vector<float>(dimension, 0));
    new_rain_drops = vector<vector<float>>(dimension, vector<float>(dimension, 0));
    temp_trickle = vector<vector<float>>(dimension, vector<float>(dimension, 0));
    vector<vector<float>> zero_trickle(dimension, vector<float>(dimension, 0));
    //TODO: can parallize
    trickle_neighs_list = trickle_list(elevation, dimension);
    // vector<pair<int, int>> trickle_neighs;

    steps = 1; // total steps
    bool is_dry;
    gettimeofday(&start_time, NULL);
    while (true) {
        is_dry = true;
        future<void> future[numThreads]; 
        for(int i = 0; i < numThreads; i++) {
            future[i] = pool.push(eachTimeStep, lock, i);
        }
        for (int i = 0; i < numThreads; i++) {
            future[i].wait(); // synchronize all threads
        }
        // //Traverse over all landscape points
        // for (int i = 0; i < dimension; i++) {
        //     for (int j = 0; j < dimension; j++) {
        //         //1) Receive a new raindrop (if it is still raining) for each point.
        //         if (steps <= time_steps) {
        //             rain_drops[i][j]++;
        //         }
        //         //2) If there are raindrops on a point, absorb water into the point
        //         if (rain_drops[i][j] >= absorp_rate) {
        //             rain_absorb[i][j] += absorp_rate;
        //             rain_drops[i][j] -= absorp_rate;
        //         } else {
        //             rain_absorb[i][j] += rain_drops[i][j];
        //             rain_drops[i][j] = 0;
        //         }
        //         new_rain_drops[i][j] = rain_drops[i][j];
        //         if (rain_drops[i][j] == 0) {
        //             continue;
        //         }
        //         //3a) Calculate the number of raindrops that will next trickle to the lowest neighbor(s)
        //         float trickle_drops = (rain_drops[i][j] >= 1) ? 1 : rain_drops[i][j];
        //         // trickle_neighs = trickle(i, j, dimension, elevation);
        //         trickle_neighs = trickle_neighs_list[i * dimension + j];
        //         if (trickle_neighs.size() == 0) {
        //             continue;
        //         } else if (trickle_neighs.size() == 1) {
        //             auto neighbour = trickle_neighs[0];
        //             temp_trickle[neighbour.first][neighbour.second] += trickle_drops;
        //             new_rain_drops[i][j] -= trickle_drops;
        //         } else {
        //             for (auto neighbour : trickle_neighs) {
        //                 temp_trickle[neighbour.first][neighbour.second] +=  1.0 * trickle_drops / trickle_neighs.size();
        //             }
        //             new_rain_drops[i][j] -= trickle_drops;
        //         }
        //     }
        // }
        // //Make a second traversal over all landscape points
        // //3b) For each point, use the calculated number of raindrops that will trickle to the
        // //lowest neighbor(s) to update the number of raindrops at each lowest neighbor, if applicable.
        // for (int i = 0; i < dimension; i++) {
        //     for (int j = 0; j < dimension; j++) {
        //         new_rain_drops[i][j] += temp_trickle[i][j];
        //         if (new_rain_drops[i][j] > 0) {
        //             is_dry = false;
        //         }
        //     }
        // }
        //Make a second traversal over all landscape points
        //3b) For each point, use the calculated number of raindrops that will trickle to the
        //lowest neighbor(s) to update the number of raindrops at each lowest neighbor, if applicable.
        for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                new_rain_drops[i][j] += temp_trickle[i][j];
                if (new_rain_drops[i][j] > 0) {
                    is_dry = false;
                }
            }
        }

        if (is_dry && steps > time_steps) {
            gettimeofday(&end_time, NULL);
            return steps;
        }
        steps++;
        temp_trickle = zero_trickle;
        rain_drops = new_rain_drops;
    }
}

    void eachTimeStep(int id, mutex &lock, int threadID) {
        int blockSize = dimension / numThreads;
        for (int i = threadID * blockSize; i < min((threadID + 1)* blockSize, dimension) ; i++) {
            for (int j = 0; j < dimension; j++) {
                //1) Receive a new raindrop (if it is still raining) for each point.
                if (steps <= time_steps) {
                    rain_drops[i][j]++;
                }
                //2) If there are raindrops on a point, absorb water into the point
                if (rain_drops[i][j] >= absorp_rate) {
                    rain_absorb[i][j] += absorp_rate;
                    rain_drops[i][j] -= absorp_rate;
                } else {
                    rain_absorb[i][j] += rain_drops[i][j];
                    rain_drops[i][j] = 0;
                }
                new_rain_drops[i][j] = rain_drops[i][j];
                if (rain_drops[i][j] == 0) {
                    continue;
                }
                //3a) Calculate the number of raindrops that will next trickle to the lowest neighbor(s)
                float trickle_drops = (rain_drops[i][j] >= 1) ? 1 : rain_drops[i][j];
                // trickle_neighs = trickle(i, j, dimension, elevation);
                
                trickle_neighs = trickle_neighs_list[i * dimension + j];
                if(i == threadID * blockSize || i == min((threadID + 1)* blockSize - 1, dimension - 1)) {
                    lock.lock();
                }
                if (trickle_neighs.size() == 0) {
                    continue;
                } else if (trickle_neighs.size() == 1) {
                    auto neighbour = trickle_neighs[0];
                    temp_trickle[neighbour.first][neighbour.second] += trickle_drops;
                    new_rain_drops[i][j] -= trickle_drops;
                } else {
                    for (auto neighbour : trickle_neighs) {
                        temp_trickle[neighbour.first][neighbour.second] +=  1.0 * trickle_drops / trickle_neighs.size();
                    }
                    new_rain_drops[i][j] -= trickle_drops;
                }
                if(i == threadID * blockSize || i == min((threadID + 1)* blockSize - 1, dimension - 1)) {
                    lock.unlock();
                }
            }
        }
        
}