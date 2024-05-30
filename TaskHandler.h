#ifndef SIMGAME_TASKHANDLER_H
#define SIMGAME_TASKHANDLER_H

#include <vector>
#include "WorldMap.h"
#include "Task.h"
#include <atomic>
#include <condition_variable>
#include <queue>
#include <memory>

class TaskHandler {
public:
    TaskHandler();
    ~TaskHandler();

    void taskMapGeneration(WorldMap map);
private:
    std::vector<std::jthread> workers;
    std::vector<std::unique_ptr<Task>> tasks; //use something else, Vector slow due to thread safety
    std::vector<std::mutex> locks;
    int numThreads;


    void workerThread();
    std::optional<std::unique_ptr<Task>> getNext();
    bool tryLock(int index);
    // Join all threads function?
};


#endif //SIMGAME_TASKHANDLER_H
