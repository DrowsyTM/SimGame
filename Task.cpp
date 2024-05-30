#include "Task.h"

//-------------------World Terrain Gen Task--------------------
TaskTerrainGen::TaskTerrainGen(const int chunk, WorldMap* mapPointer) {
    taskData.three_byte1 = chunk; // <= 16777216
    taskData.pointer = reinterpret_cast<uint64_t>(mapPointer);
}

void TaskTerrainGen::execute() {
    //Terrain Gen logic using data
    //Ugh I need to use safe pointers don't I
    WorldMap* pointer = reinterpret_cast<WorldMap*>(taskData.pointer);
    pointer->threadGenerateTerrain(taskData.three_byte1);
}

