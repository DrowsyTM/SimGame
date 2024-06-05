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
    std::vector<std::vector<std::vector<TaskBatch>>> work_array_u;
    std::vector<std::atomic<int>> working_index; // Just do bounds checking. If = 9, set 0, otherwise iterate
    std::vector<std::atomic<int>> end_index;

    // The vector of TaskBatches is fixed in size. If worker hits "end" index, checks it again.
    // Then works through TaskBatches until it hits the end again(doesn't check until then).
    // Depending on the number of loaders, they are responsible for loading a certain # of workers.
    // 2 Loader 10 Worker means first loader does first 5, second does latter 5.
    // Loaders load batches with minimum size(lets say 10) into the batch vector. Keep doing this until all workers are
    // Caught up with work. If workers work faster than loaders load(we measure this somehow), we output this to log
    // In future, dynamically increase loader count until its just about filling up in time.
    // For loaders, we can start with 1/2 and keep iterating by one until balanced. For batch size, start at 10 and double?
    // Wait... we do doubling after filling up on loaders or what?
    // Wait... how do we track # of batches left in a bucket?
    // Also, what if after all buckets are filled, loader just fills batch until one opens up? no, this needs constant checks,
    // also all would go to first worker to finish
    // And if tasks are evenly distributed, we can just distribute without too many checks.
    // Dangerous if worker is so backed up loader just overwrites uncompleted tasks.
    // Ooh when worker checks for end index, it also saves its current index?


    // Create a "step through" or visualization system that lets me work on this better? Just want to see which threads
    // are active and which are inactive(we want inactive to approach 0)
    // Throughput

    //ehh figure this out on paper. I want rare/minimal interactions and NO DELAYS(so keep adding loaders till delay = 0)
    //I could just do this manually for the terrain gen, but I want it to work with all Tasks
    //We can easily estimate and split up work given map size. However, how do we optimize given an unkown /constant
    //stream of tasks?

};


#endif //SIMGAME_TASKHANDLER_H
