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

    void taskMapGeneration(WorldMap& map);
    int numTasks();

private:
    std::vector<std::jthread> workers;
    std::queue<std::unique_ptr<Task>> tasks;
    int numThreads;
    std::mutex mtx;
    std::stop_source stopSource;
    std::stop_token stopToken; //Whether threads should stop permanently
    std::condition_variable cv; //Signals whether tasks are available

    void workerThread();

    std::unique_ptr<Task> pop();

    void push(std::unique_ptr<Task> task);

};


#endif //SIMGAME_TASKHANDLER_H
