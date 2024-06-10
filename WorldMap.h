#ifndef SIMGAME_WORLDMAP_H
#define SIMGAME_WORLDMAP_H

#include <vector>
#include <memory> //unique_ptr, etc - safe pointers
#include <iostream>
#include <thread>
#include <random>
#include <chrono>
#include <fstream>

enum terrain {
    DEFAULT, PLAINS, FOREST, FCENTER
};

//class Tile {
//private:
//    terrain type;
//    int x, y;
//};

class WorldMap {
public:
    WorldMap(); //Uses default map size

    ~WorldMap();

    void initializeObject(int map_x, int map_y);

    void draw() const;

    void printMap() const;

    void set(terrain type, int x, int y);

    int size() const;

    int getNumChunks() const;

    void threadGenerateTerrain(int chunk);
    //or just add overload to generateTerrain that allows specification of where to gen
    //hmm should WorldMap track chunks for them? Or is this tracked in TaskHandler?

    void generateTerrain();


private:
    std::vector<std::vector<terrain>> map;
    int x_dimension, y_dimension;
    int screen_size;
    int chunk_size;
    int num_chunks;
    int look_x, look_y;

    void spawnRandomzier(int &x, int &y);
};

#endif //SIMGAME_WORLDMAP_H
