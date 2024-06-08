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
    using Thread = std::jthread;
    using TaskPtr = std::unique_ptr<Task>;
    using TaskBatch = std::vector<TaskPtr>;
    using Flag = std::atomic<bool>;

    //Okay so cache locality? Flattening



    //In fact, multiple loaders can load up muliple different load arrays and then just send
    // it to the first available worker. Implement this later, currently 1:1

    Flag is_stopped;
    Flag is_logging;
    std::fstream logger;
    std::mutex logger_mtx;

    void workerThread(int ID);

    void loadMapTasks(WorldMap &map, int ID);

    void loadWorkers();

    void loadLogger();

    //New funcs

    //   1:1    holds batches   batches     item in batch
    std::vector<std::vector<TaskBatch>> work_array;
    std::vector<std::atomic<int>> bucket_fullness; // Just do bounds checking. If = 9, set 0, otherwise iterate
    std::vector<std::atomic<int>> end_indexes; //both _index vectors have all values at 0
    int batch_size; //Current batch size. For now, constant.
    int bucket_size; //Bucket holds batches. Size 10.

    std::vector<Thread> workers; // collects worker threads
    int num_workers; // equivalent to # of buckets
    std::vector<Thread> loaders;
    int num_loaders;


};


#endif //SIMGAME_TASKHANDLER_H
