#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <climits>
#include <sys/time.h>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;

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

