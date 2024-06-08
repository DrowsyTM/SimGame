#include "TaskHandler.h"

//Delegates to full constructor
TaskHandler::TaskHandler() : TaskHandler(5, 5, 10, false) {}

//Delegates to full constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize)
        : TaskHandler(workerCount, loaderCount, bucketSize, false) {}

//Full Constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize, bool doLogging)
        : num_workers{workerCount}, num_loaders{loaderCount}, bucket_size(bucketSize), is_logging{doLogging},
          is_stopped{false}, loaders{}, workers{}, end_indexes{}, bucket_fullness{} {


    //Remove all args except for doLogging in the future.

    if (is_logging) {
        loadLogger(); //Loads logging system
    }
    loadWorkers(); //Loads workers and work_arrays
}

TaskHandler::~TaskHandler() {
    //Loaders first currently so that workers finish all their work
    //Loaders finish on their own eventually
    if (!loaders.empty()) {
        for (int i = 0; i < num_loaders; i++) {
            if (loaders[i].joinable()) {
                loaders[i].join();
            }
        }
    } else if (is_logging.load(std::memory_order_relaxed)) {
        std::lock_guard lock(logger_mtx);
        logger << "Loaders never initialized" << std::endl;
    }
    //Flag to stop workers
    is_stopped.store(true);

    if (is_logging.load(std::memory_order_relaxed)) {
        logger << "Stop flag set" << std::endl;
    }
    int ID = 0;

    for (auto &worker: workers) {
        if (is_logging.load(std::memory_order_relaxed)) {
            logger << "Joining thread " << ID << "..." << std::endl;
        }
        if (worker.joinable()) {
            worker.join();
        }
        ID++;
    }
    if (is_logging.load(std::memory_order_relaxed)) {
        logger << "Workers joined" << std::endl;
    }

    if (is_logging.load(std::memory_order_relaxed)) {
        logger << "Destructor fully executed" << std::endl;
        logger.close();
    }

}

void TaskHandler::LoadingBay(WorldMap &map) {
    if (loaders.empty()) { //doesn't make it work but prevents too many problems I think
        for (int i = 0; i < num_loaders; i++) {
            loaders.emplace_back([this, &map, i]() { loadMapTasks(map, i); });
        }
    }

    if (is_logging) {
        logger << "Loaded loaders" << std::endl;
    }
}

//---------------------PRIVATE----------------------------


void TaskHandler::loadMapTasks(WorldMap &map, int ID) {
    // I think the structure of this makes sense more this way:
    // This func calls a func of world map that lets it distribute chunks.
    // this func can determine # of chunks, or just how many workers to split to
    // Then it works like a "GetNext func" returning a start_chunk chunk or something.
    // This would look cleaner and more modular.

    // ehh for now lets not break what works well, but later worldmap shouldn't care about chunks
    // Or we create a full on chunk system for updates as well.

    int chunk_iter; //Used for iterating in while loop. Starts at first chunk handled by current thread
    int end_chunk; //Last chunk handled by thread


    { // To free up chunks_per_loader
        // chunks_per_loader is how many chunks will be loaded by this loader
        int chunks_per_loader = map.getNumChunks() / num_loaders;
        chunk_iter = chunks_per_loader * ID; //starting chunk
        end_chunk = chunk_iter + chunks_per_loader;

    }
    if (ID == num_loaders - 1) { //If you're the last loader, you take rounding remainders
        end_chunk = map.getNumChunks();
    }

    //ID corresponds to bucket

    std::vector<int> corresponding_workers; //Which worker threads are "loaded" by the current loader thread
    for (int i = 0; i < num_workers; i + num_loaders) {
        if (ID + i < num_workers) {
            corresponding_workers.emplace_back(ID + i);
        }
    }

    //For amount = 250, ID is 0-9
    //0-249, 250-499, 500-749 ... 2250-2499

    int batch_size = 10;
    int batch_counter = 0; //How full our current batch is
    TaskBatch batch(batch_size);

    std::vector<TaskBatch> batch_buffer(10); //If all worker queues are full of batches

    int bucket_fullness_copy;
    int end_index_copy; //Copies to minimize calls to atomic
    int lowest_bucket_fullness;

    while (true) {
        if (batch_counter < batch_size) {
            batch.emplace_back(std::make_unique<TaskTerrainGen>(start_chunk, &map));
            batch_counter++;
        } else {
            batch_counter = 0;
            lowest_bucket_fullness = num_workers; // 1 above max ID / bucket index

            for (int ID: corresponding_workers) {
                bucket_fullness_copy = bucket_fullness[ID].load(); //To prevent calling atomic twice
                if (bucket_fullness_copy < lowest_bucket_fullness) { //Find least full worker bucket
                    lowest_bucket_fullness = bucket_fullness_copy;
                }
            }
            if (lowest_bucket_fullness != 10) { //Fullness = # of uncompleted/unknown status tasks

            }
            // rather than have an "end" index, we can track "length" from the working_index
            // We will base this on the behavior of worker to mark its current index then work to the latest available
            // index. This works well with looping the array
        }

        chunk_iter++;
    }

    for (int chunk = start_chunk; chunk < end_chunk; chunk++) {


    }

}

void TaskHandler::loadWorkers() {
    work_array.resize(num_workers); //Resize outer vector to # of buckets
    for (int i = 0; i < num_workers; i++) {
        work_array[i].resize(bucket_size); //Resize 1 depth vector to # of batches per bucket
    } //Fill up the first vec with empty Task pointer vecs.
    //For the third layer of the vector, size of Batches can be changed runtime

    //Further functionality means # of workers can change as well.

    {
        std::vector<std::atomic<int>> temp(num_workers);
        for (int i = 0; i < num_workers; i++) {
            temp[i] = 0;
        }
        end_indexes.swap(temp);
    }

    {
        std::vector<std::atomic<int>> temp(num_workers);
        for (int i = 0; i < num_workers; i++) {
            temp[i] = 0;
        }
        bucket_fullness.swap(temp);
    }


    for (int i = 0; i < num_workers; i++) {
        workers.emplace_back([this, i]() { this->workerThread(i); });
    }

    if (is_logging) {
        logger << "Loaded workers, work arrays, and work_flags\n";
    }

}

void TaskHandler::loadLogger() {
    std::string logName = "HandlerLog.txt";
    logger.open(logName, std::fstream::app);
    if (!logger.is_open()) {
        is_logging = false;
        std::cout << "Unable to open " << logName << std::endl;
    } else {
        std::chrono::time_point currTime = std::chrono::system_clock::now();
        auto timeNow = std::chrono::system_clock::to_time_t(currTime);
        logger << "\n\n" << ctime(&timeNow);
        logger << "Workers: " << num_workers << " | Loaders: " << num_loaders << " | Buckets: " << bucket_size
               << "\n";
    }

}

void TaskHandler::workerThread(int ID) {

    while (!is_stopped.load(std::memory_order_relaxed)) {
        int iter = 0;
        //While corresponding flag is false (default) & < 0.1 ms of wait
        while (!is_working[ID].load(std::memory_order_relaxed) && iter < 100000) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            iter++;
        }
        if (iter >= 100000) { //Waiting function to not waste resources.
            while (!is_working[ID].load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                if (is_stopped.load(std::memory_order_relaxed)) {
                    return;
                }
            }
        }
        iter = 0;


        for (auto &task: work_arrays[ID]) {
            task->execute(); //check if null? If array not full
        }
        is_working[ID].store(false, std::memory_order_relaxed);
    }

}

/** TODO
 *
    //Currently just set up the batch system with no dynamic changes.

    //Each loading thread has its own internal buffer of batches. (And of indices they are in charge of?
    //Ohh we can have a metric of how back up it is? Like, goal is 10 at most backed up, and we increase batch size to match.
    //We increase batch size when we have a consistent queue. Otherwise, we increase/decrease loader count?
    //We might need to detach threads and use stop tokens?

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
    //We can easily estimate and split up work given map size. However, how do we optimize given an unknown /constant
    //stream of tasks?
 *
 * Destructor shouldn't need to wait for threads. Threads should use memory-safe stopping procedure.
 *
 *
 * First of all, lets not change anything and optimize the system for 100k on 100k. Look at single threading.
 * Then use current system as benchmark for further changes.
 * Also changes to current: bucket_size needs to work regardless of size of map.
 *
 * I suspect that the main delay is while the worker is waiting on the loader.
 * That's why bigger bucket leads to massive improvement
 * It's the worker flipping its flag and then waiting for its array to be swapped.
 * I gotta measure the execution time of all the steps. It's odd, feels like a 1ns wait is reasonable.
 * We can have a set of arrays where the worker can always jump to the next one, loader fills up another one.
 * Essentially worker should be able to instantly jump to next work with NO DELAY.
 *
 * If we load faster than worker works, we just load batch after batch. We can have an antomic flag just in case
 * worker catches up.
 *
 * Destructor timing is super inconsistent.
 *
 * Work with alternative ratios of worker/loaders,
 * Work with map not a multiple of chunks
 * Batches rather than static arrays
 * Ability to clear the LoadingBay or Reuse loaders
 * Loop that just executes any function given to it? As a "hold thread" object or omsething
 * !!!Something to track when map is completed. The loaders or even the tasks could mark sections as "completed"
 * Can't have workers * bucket_size > map_size, no checks for end of chunks
 *
 * As seen from the log, bucket size up is good. The contention for atomic or the wait time is causing problems.
 * We probably want more loaders than workers, especially for super cheap tasks.
 *
 *
 */