#ifndef SIMGAME_TASKHANDLER_H
#define SIMGAME_TASKHANDLER_H

#include <vector>
#include "WorldMap.h"
#include "Task.h"

class TaskHandler {
public:
    TaskHandler();
    ~TaskHandler();

    void taskMapGeneration(WorldMap map);
private:
    std::vector<std::thread> workers;
    std::vector<Task> queue; //use something else, Vector slow due to thread safety
    int numThreads;

    void workerThread();
//    std::vector<TaskTerrainGen> mapChunkFactory(int mapX, int mapY);
};


#endif //SIMGAME_TASKHANDLER_H
