#ifndef SIMGAME_TASKHANDLER_H
#define SIMGAME_TASKHANDLER_H

#include <vector>
#include <array>
#include "WorldMap.h"
#include "Task.h"
#include <atomic>
#include <condition_variable>
#include <queue>
#include <memory>
#include <fstream>

class TaskHandler {
public:
    TaskHandler();

    TaskHandler(int workerCount, int loaderCount, const int bucketSize);

    TaskHandler(int workerCount, int loaderCount, const int bucketSize, bool doLogging);

    ~TaskHandler();

    void LoadingBay(WorldMap &map);
//
//    void taskMapGeneration(WorldMap& map);
//    int numTasks();
//    void waitOnEmpty();

private:
    int num_workers;
    std::vector<std::jthread> workers;
    std::vector<std::vector<std::unique_ptr<Task>>> work_arrays;

    int num_loaders;
    std::vector<std::jthread> loaders;
    std::vector<std::vector<std::unique_ptr<Task>>> load_arrays;
    std::vector<std::vector<bool>> load_flags;

    int bucket_size;
    bool stop_flag;
    bool logger_flag;

    void loadMapTasks(WorldMap &map, int ID);

    void loadWorkers();

    void loadLoaderArrays();

//    std::queue<std::unique_ptr<Task>> tasks;

//    std::mutex mtx;
//    std::mutex worker_mtx;
//    std::stop_source stopSource;
//    std::stop_token stopToken; //Whether threads should stop permanently
//    std::condition_variable cv; //Signals whether tasks are available
//
//
//
//    void workerThread();
//
//    void push(std::unique_ptr<Task> task);

};


#endif //SIMGAME_TASKHANDLER_H
