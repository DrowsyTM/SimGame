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
    PLAINS, FOREST, FCENTER, BLANK
};

class Tile {
private:
    terrain type;
    int x, y;
};

class WorldMap {
public:
    WorldMap(); //Uses default map size
    WorldMap(int size);

    ~WorldMap();

    void draw() const;

    void printMap() const;

    void set(terrain type, int x, int y);

    int x() const;

    int y() const;

    int size() const;

    void clear();

    bool isEmpty() const;

    int chunkSize() const;

    int getNumChunks() const;

    void setInitialized();

    void threadGenerateTerrain(int chunk);
    //or just add overload to generateTerrain that allows specification of where to gen
    //hmm should WorldMap track chunks for them? Or is this tracked in TaskHandler?

    void generateTerrain();

private:
    std::vector<std::vector<terrain>> map;
    int map_size;
    int screen_size;
    bool initialized;
    int chunk_size;
    int num_chunks;
    int look_x, look_y;

    void updateLayer();

    void spawnRandomzier(int &x, int &y);
};

#endif //SIMGAME_WORLDMAP_H
