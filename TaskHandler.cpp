#include "TaskHandler.h"

TaskHandler::TaskHandler() : queue{}, numThreads{12} {

}

TaskHandler::~TaskHandler() {

}

void TaskHandler::taskMapGeneration(WorldMap map) {
//    mapChunkFactory(map.size(), map.size());

}

// Private
void TaskHandler::workerThread() {
    // loops until work exists (conditional variable?)
}

//std::vector<TaskTerrainGen> TaskHandler::mapChunkFactory(int mapX, int mapY) {
//    return std::vector<TaskTerrainGen>();
//}


std::vector<Task> mapChunkFactory(int mapX, int mapY) {

}



