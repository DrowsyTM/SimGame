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
#include <chrono>

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
    int num_workers; // equivalent to # of buckets
    std::vector<std::jthread> workers; // collects worker threads
    std::vector<std::vector<std::unique_ptr<Task>>> work_arrays; // Buckets for each worker
    std::vector<std::atomic<bool>> working_flag; //single flag for each bucket

    int num_loaders;
    std::vector<std::jthread> loaders;
    std::vector<std::vector<std::unique_ptr<Task>>> load_arrays;
    std::vector<std::condition_variable> loader_cvs; //Corresponds to each loader

    //In fact, multiple loaders can load up muliple different load arrays and then just send
    // it to the first available worker. Implement this later, currently 1:1

    std::mutex logger_mtx;
    int bucket_size;
    std::atomic<bool> stop_flag;
    bool logger_flag;
    std::fstream logger;

    void workerThread(int ID);

    void loadMapTasks(WorldMap &map, int ID);

    void loadWorkers();

    void loadLoaderArrays();

    void loadLogger();


};


#endif //SIMGAME_TASKHANDLER_H
