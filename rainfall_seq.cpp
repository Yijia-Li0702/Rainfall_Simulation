#include "rainfall_seq.hpp"
#include <bits/stdc++.h>

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

public void printMsg() {
        cout << "Error!"
        cout << "Command : ./rainfall_seq <P> <M> <A> <N> <elevation_file>" << endl;
        cout << "P = # of parallel threads to use." << endl;
        cout << "M = # of simulation time steps during which a rain drop will fall on each landscape
        point. In other words, 1 rain drop falls on each point during the first M steps of the
        simulation." << endl;
        cout << "A = absorption rate (specified as a floating point number). The amount of raindrops
        that are absorbed into the ground at a point during a timestep." << endl;
        cout << "P = # of parallel threads to use." << endl;
        cout << "N = dimension of the landscape (NxN)" << endl;
        cout << "elevation_file = name of input file that specifies the elevation of each point." << endl;
}
public void readFile(string elevation_file, vector<vector<int>> &elevation) {
    fstream fin("./" + elevation_file);
    string line;
    int r = 0, item;
    if(fin) {
        while(getline(fin, line)) { 
            stringstream ss(line);
            while(ss >> item) {
                elevation[r].push_back(item);
            }
        }
        r++;
    } else {
        cout << "File doesn't exist, please check your input" << endl;
        return EXIT_FAILURE;
    }
}


int main(int argc, char *argv[]) {
    if(argc != 6) {
        printMsg();
        return EXIT_FAILURE;
    }
    int timeSteps = stoi(argv[2]);
    int absorpRate = stoi(argv[3]);
    int dimension = stoi(argv[4]);
    string elevation_file = stoi(argv[5]);
    struct timespec start_time, end_time;
    vector<vector<int>> elevation(dimension, vector<int>(dimension, 0));
    readFile(elevation_file, elevation);
    vector<vector<int>> rainAbsorb(dimension, vector<int>(dimension, 0));

    
}

public void rainfall(vector<vector<int>> & elevation, vector<vector<float>> & rainAbsorb, int absorpRate, int timeSteps, int dimension,
                    struct timespec & start_time, struct timespec & end_time) {
    int remaining = 1;
    vector<vector<float>> rainDrops(dimension, vector<float>(dimension, 0));
    vector<vector<float>> trickle(dimension, vector<float>(dimension, 0));
    int steps = 0;
    while(remaining) {
        step++;
        for(int i = 0; i < dimension; i++) {
            for(int j = 0; j < dimension; j++) {
                if(timeSteps > 0) {
                    rainDrops[i][j]++;
                }
                rainDrops[i][j] += trickle[i][j];
                if(rainDrops[i][j] >= absorpRate) {
                    rainDrops[i][j] -= absorpRate;
                    rainAbsorb[i][j] += absorpRate;
                } else {
                    rainAbsorb[i][j] += rainDrops[i][j];
                    rainDrops[i][j] = 0;
                }
                //calculate trickle to neighbor
                
            }
        }
        for(int i = 0; i < dimension; i++) {
            for(int j = 0; j < dimension; j++) {

            }
        }
    }

}

public void checkInput(int element,vector<int>& low_elev, vector<vector<int>>& elevation, unordered_map<int, vector<pair<int, int>>>& map, int i, int j) {
    if(element >= elevation[i][j]){
         if(map.contains(elevation[i][j])){
            auto it = map.find(elevation[i][j]);
            it.second.push(new pair<int, int>(i, j));
        } else {
            vector<pair<int, int>> vec(make_pair(i, j));
            map.insert(make_pair(elevation[i][j], vec));
            low_elev.push_back(elevation[i][j]);
        }
    }
}

public void trickle(int i, int j, int dimension, vector<vector<int>>& elevation, vector<vector<int>>& trickle,  vector<vector<int>>& rainDrops) {
    int neiborElev[4][2];
    //create a hashmap<int, list> store elevation and coordinate
    //create an list to store elevations
    //sort the list
    //record the number of lowest coordinate, go hashmap to find the coordinate
    //update trickle
    unordered_map<int, vector<pair<int, int>>> map;
    vector<int> low_elev;
    if(i > 0) {
        // neiborElev[0][0] = elevation[i - 1][j];
        // neiborElev[0][1] = 0; 
        checkInput(elevation, map, i - 1, j);
        
    }
    if(j > 0) {
         checkInput(elevation, map, i, j - 1);
    }
    if(i < dimension - 1) {
         checkInput(elevation, map, i + 1, j);
    }
    if(j < dimension - 1) {
        checkInput(elevation, map, i, j + 1);
    }
    sort(low_elev.begin(), low_elev.end());
    int lowest = low_elev[0];
    vector<pair<int, int>> points = map.find(lowest).second();

}

