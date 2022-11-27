#include "rainfall_seq.hpp"

using namespace std;

double calc_time(struct timespec start, struct timespec end) {
    double start_sec =
        (double)start.tv_sec * 1000000000.0 + (double)start.tv_nsec;
    double end_sec = (double)end.tv_sec * 1000000000.0 + (double)end.tv_nsec;

    if (end_sec < start_sec) {
        return 0;
    } else {
        return end_sec - start_sec;
    }
}

void printMsg() {
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

bool readFile(string elevation_file, vector<vector<int>> &elevation) {
    fstream fin("./" + elevation_file);
    string line;
    int r = 0, item = 0;
    if (fin) {
        while (getline(fin, line)) {
            stringstream s(line);
            while (s >> item) {
                elevation[r].push_back(item);
            }
        }
        r++;
    }
    else {
        cout << "File doesn't exist, please check your input" << endl;
        return false;
    }
    return true;
}

void checkInput(int central_elevation, int &min_elevation, int neigh_elevation,
                    unordered_map<int, vector<pair<int, int>>> &map, int i, int j) {
    // if neighbour's elevation is smaller, add to map
    if (neigh_elevation < central_elevation) {
        if (map.find(neigh_elevation) != map.end()) {
            auto pair_list = map[neigh_elevation];
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

void trickle(int i, int j, int dimension, vector<vector<int>> &elevation, 
                vector<vector<float>> &trickle, vector<vector<float>> &rain_drops) {
    // create a hashmap<int, list> store elevation and coordinate
    // create an list to store elevations
    // sort the list
    // record the number of lowest coordinate, go hashmap to find the coordinate
    // update trickle
    vector<pair<int, int>> trickle_neighs;
    unordered_map<int, vector<pair<int, int>>> map;
    int central_elevation = elevation[i][j];
    int min_elevation = INT_MAX;
    // north
    if (i - 1 >= 0) {
        checkInput(central_elevation, min_elevation, elevation[i - 1][j], map, i - 1, j);
    }
    // south
    if (i + 1 < dimension) {
        checkInput(central_elevation, min_elevation, elevation[i + 1][j], map, i + 1, j);
    }
    // west
    if (j - 1 >= 0) {
        checkInput(central_elevation, min_elevation, elevation[i][j - 1], map, i, j - 1);
    }
    // east
    if (j + 1 < dimension) {
        checkInput(central_elevation, min_elevation, elevation[i][j + 1], map, i, j + 1);
    }
    if (min_elevation != INT_MAX) {
        trickle_neighs = map[min_elevation];
        // update
    } else {
        // no update
    }
}

void rainfall(vector<vector<int>> &elevation, vector<vector<float>> &rain_absorb, float absorp_rate, 
                int time_steps, int dimension, struct timespec &start_time, struct timespec &end_time,
                int &steps) {
    int remaining = 1;
    vector<vector<float>> rain_drops(dimension, vector<float>(dimension, 0));
    vector<vector<float>> trickle(dimension, vector<float>(dimension, 0));
    
    // while (remaining) {
    //     steps++;
    //     for (int i = 0; i < dimension; i++) {
    //         for (int j = 0; j < dimension; j++) {
    //             if (time_steps > 0) {
    //                 rain_drops[i][j]++;
    //             }
    //             rain_drops[i][j] += trickle[i][j];
    //             if (rain_drops[i][j] >= absorp_rate) {
    //                 rain_drops[i][j] -= absorp_rate;
    //                 rain_absorb[i][j] += absorp_rate;
    //             } else {
    //                 rain_absorb[i][j] += rain_drops[i][j];
    //                 rain_drops[i][j] = 0;
    //             }
    //             // calculate trickle to neighbor
    //         }
    //     }
    //     for (int i = 0; i < dimension; i++) {
    //         for (int j = 0; j < dimension; j++) {
    //         }
    //     }
    // }
}

int main(int argc, char *argv[]) {
    // Error format
    if (argc != 6) {
        printMsg();
        return EXIT_FAILURE;
    }
    // P = # of parallel threads to use. ignored in sequential code
    // M = # of simulation time steps during which a rain drop will fall on each landscape point. 
    int M = stoi(argv[2]);
    // A = absorption rate
    int A = stoi(argv[3]);
    // N = dimension of the landscape (NxN)
    int N = stoi(argv[4]);
    // elevation_file = name of input file that specifies the elevation of each point.
    string elevation_file = argv[5];

    struct timespec start_time, end_time;

    vector<vector<int>> elevation(N, vector<int>(N, 0));
    if (!readFile(elevation_file, elevation)) {
        return EXIT_FAILURE;
    }

    vector<vector<int>> rainAbsorb(N, vector<int>(N, 0));

}
