#ifndef SIMGAME_WORLDMAP_H
#define SIMGAME_WORLDMAP_H

#include <vector>
#include <memory> //unique_ptr, etc - safe pointers
#include <iostream>
#include <thread>
#include <random>
#include <chrono>

enum terrain {
    PLAINS, FOREST, FCENTER
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

    void set(terrain type, int x, int y);

    int x() const;

    int y() const;

    int size() const;

    void clear();

    bool isEmpty() const;

    int chunkSize() const;

private:
    std::vector<std::vector<terrain>> map;
    int maxSize;
    int lookX, lookY;
    int screenSize;
    bool initialized;
    int chunk_size;

    void generateTerrain();
    void threadGenerateTerrain(); //or just add overload to generateTerrain that allows specification of where to gen
    //hmm should WorldMap track chunks for them? Or is this tracked in TaskHandler?
    void updateLayer();

    void spawnRandomzier(int &x, int &y);
};

#endif //SIMGAME_WORLDMAP_H
