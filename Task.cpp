#include "Task.h"

TaskTerrainGen::TaskTerrainGen(const int chunk, WorldMap* mapPointer) {
    taskData.three_byte1 = chunk; // <= 16777216
    taskData.pointer = reinterpret_cast<uint64_t>(mapPointer);
    //Need to also pass pointer
}

void TaskTerrainGen::execute() {
    //Terrain Gen logic using data
    //Ugh I need to use safe pointers don't I
    WorldMap* pointer = reinterpret_cast<WorldMap*>(taskData.pointer);
    int chunkSize = pointer->chunkSize();
    int chunksDimension = pointer->size() / chunkSize;
    int chunk = taskData.three_byte1;

    // Chunk 0-49 = 0-49 * 15
    int xOffset = (chunk % chunksDimension) * chunkSize;
    // Chunk 0-49 = 0, 50-99 = 1, etc * 15
    int yOffset = (chunk / 50) * chunkSize;

    for (int y = 0; y < chunkSize; y++) {
        for (int x = 0; x < chunkSize; x++) {

        }
    }

    //Or maybe just scrap this shit an just call this function via the map? Map already has access to the sizes
}

