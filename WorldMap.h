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
private:
    std::vector<std::vector<terrain>> map;
    int maxSize;
    int lookX, lookY;
    int screenSize;

    void generateTerrain();

    void spawnRandomzier(int &x, int &y);

public:
    WorldMap(); //Uses default map size
    WorldMap(int size);

    ~WorldMap();

    void draw();

    void set(terrain type, int x, int y);

    int x();

    int y();

    void clear();

    bool isEmpty();


};

#endif //SIMGAME_WORLDMAP_H
