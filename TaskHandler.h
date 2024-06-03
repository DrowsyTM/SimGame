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


    struct Batch { //So do we just use a linked list sort of system. Or do we have a vector that can be iterated?
        TaskBatch tasks;
        Batch* next = nullptr;
    };

    int num_workers; // equivalent to # of buckets
    std::vector<Thread> workers; // collects worker threads
    std::vector<TaskBatch> work_arrays; // Buckets for each worker
    std::vector<Flag> is_working; //single flag for each bucket
    std::vector<Flag> is_loaded;

    int num_loaders;
    std::vector<Thread> loaders;
    std::vector<TaskBatch> load_arrays;

    //In fact, multiple loaders can load up muliple different load arrays and then just send
    // it to the first available worker. Implement this later, currently 1:1

    int bucket_size;
    Flag is_stopped;
    Flag is_logging;
    std::fstream logger;
    std::mutex logger_mtx;

    void workerThread(int ID);

    void loadMapTasks(WorldMap &map, int ID);

    void loadWorkers();

    void loadLoaderArrays();

    void loadLogger();

    //New funcs

    //   1:1    holds batches   batches     item in batch
    std::vector<std::vector<std::vector<std::unique_ptr<Task>>>> wArray;

    //yeah I've been thinking and it does seem like the main limitation right now is that Worker can't work while loader
    //swaps. If we use three vectors instead of two for this, it would actually be much better. The flag could flip to
    //show which vector it is active in.
    // Create a "step through" or visualization system that lets me work on this better? Just want to see which threads
    // are active and which are inactive(we want inactive to approach 0)

    //ehh figure this out on paper. I want rare/minimal interactions and NO DELAYS(so keep adding loaders till delay = 0)
    //I could just do this manually for the terrain gen, but I want it to work with all Tasks
    //We can easily estimate and split up work given map size. However, how do we optimize given an unkown /constant
    //stream of tasks?

};


#endif //SIMGAME_TASKHANDLER_H
