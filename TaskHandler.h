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
    std::vector<std::jthread> workers;
    std::vector<std::jthread> loaders;


    //Wait why am I using a unique ptr for Task? Hm prob so that it can be passed to me.
    // Honestly Task is smaller than a ptr and there are so many more things to be optimized.
    // But I dunno.
    std::vector<std::vector<std::unique_ptr<Task>>> work_arrays;
    std::vector<std::vector<std::unique_ptr<Task>>> load_arrays;

    int num_workers;
    int num_loaders;
    int bucket_size;
    bool stop_flag;
    bool logger_flag;

    void loadMapTasks(WorldMap &map, int ID);

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
