#include "TaskHandler.h"

//Delegates to full constructor
TaskHandler::TaskHandler() : TaskHandler(5, 5, 10, false) {}

//Delegates to full constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize)
        : TaskHandler(workerCount, loaderCount, bucketSize, false) {}

//Full Constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize, bool doLogging)
        : num_workers{workerCount}, num_loaders{loaderCount}, bucket_max_capacity(bucketSize), is_logging{doLogging},
          is_stopped{false}, is_shutdown{false}, loaders{}, workers{}, work_indexes{}, bucket_sizes{},
          default_batch_size{10} {

    //Remove all args except for doLogging in the future.
    if (num_loaders > num_workers) {
        std::cout << "Loader count above worker count not supported yet\n";
        num_loaders = num_workers;
    }

    if (is_logging) {
        loadLogger(); //Loads logging system
    }
    loadWorkers(); //Loads workers and work_arrays
}

TaskHandler::~TaskHandler() {
    //Loaders first currently so that workers finish all their work
    //Loaders finish on their own eventually

    if (!is_shutdown) {
        shutdownThreads();
    }

    if (is_logging.load(std::memory_order_relaxed)) {
        logger << "Destructor fully executed" << std::endl;
        logger.close();
    }

}

void TaskHandler::shutdownThreads() {
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

    is_shutdown.store(true);
}

void TaskHandler::LoadingBay() {
    map.initializeObject(10000, -1);

    if (loaders.empty()) { //doesn't make it work but prevents too many problems I think
        for (int i = 0; i < num_loaders; i++) {
            loaders.emplace_back([this, i]() { loadMapTasks(i); });
        }
    }

    if (is_logging) {
        logger << "Loaded loaders" << std::endl;
    }
}

WorldMap *TaskHandler::getMap() {
    return &map;
}

//---------------------PRIVATE----------------------------


void TaskHandler::loadMapTasks(int loader_id) {
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
        chunk_iter = chunks_per_loader * loader_id; //starting chunk
        end_chunk = chunk_iter + chunks_per_loader;

    }
    if (loader_id == num_loaders - 1) { //If you're the last loader, you take rounding remainders
        end_chunk = map.getNumChunks();
    }

    //loader_id corresponds to bucket

    std::vector<int> corresponding_workers; //Which worker threads are "loaded" by the current loader thread
    for (int i = 0; i < num_workers; i += num_loaders) { //Round robin loading
        if (loader_id + i < num_workers) {
            corresponding_workers.emplace_back(loader_id + i);
        }
    }

    int batch_size = default_batch_size;
    for (auto worker_id: corresponding_workers) {
        for (int batch_index = 0; batch_index < bucket_max_capacity; batch_index++) {
            work_array[worker_id][batch_index].resize(default_batch_size);
        }
    }


    TaskBatch batch; //Vector of Task ptrs. Changes size by steps via some logic
    batch.resize(batch_size);
    std::deque<TaskBatch> batch_buffer; //If worker queue is all full.


    //Vars for while loop
    int bucket_size_copy = -1; //Fullness = # of uncompleted/unknown status tasks
    int smallest_bucket_size = -1; //Most empty size of bucket
    int smallest_bucket_id = -1; //loader_id of the smallest bucket
    int work_index_copy = -1; //Copies to minimize calls to atomic
    int batch_counter = 0; //How full our current batch is

    //Damn pretty overloaded system that breaks if we have more loaders than workers
    while (true) {
        // Maybe I should make a couple of batches and distrubute them
        if (batch_counter >= batch_size || chunk_iter >= end_chunk) {

            //Smallest bucket finder
            smallest_bucket_size = bucket_max_capacity + 1;
            for (int worker_id: corresponding_workers) { //For every worker this loader is in charge of
                bucket_size_copy = bucket_sizes[worker_id].load(std::memory_order_relaxed); //Copy
                if (bucket_size_copy < smallest_bucket_size) { //Find least full worker bucket
                    smallest_bucket_size = bucket_size_copy;
                    work_index_copy = work_indexes[worker_id].load(std::memory_order_relaxed);
                    smallest_bucket_id = worker_id;
                }
            }

            // Batch move operations
            if (smallest_bucket_size < batch_size) { //Not full bucket
                // This line swaps our batch with the batch of our worker
                if (batch_buffer.empty()) {
                    work_array[smallest_bucket_id][(work_index_copy + bucket_size_copy) % 10].swap(batch);
                    bucket_sizes[smallest_bucket_id].store(smallest_bucket_size + 1, std::memory_order_relaxed);
                    batch_counter = 0;
                } else {
                    work_array[smallest_bucket_id][(work_index_copy + bucket_size_copy) % 10].swap(
                            batch_buffer.front());
                    batch_buffer.pop_front();
                    // counter not reset so we try to input again
                    // Honestly unsure if internal buffer is necessary tbh. Loader should just wait. If wait too long
                    // cull loader and give all tasks to other loader?
                }
            } else {
                batch_buffer.push_back(std::move(batch));
                // Loader should log when it reaches 10 batches
                batch_counter = 0;
            }

            // rather than have an "end" index, we can track "length" from the working_index
            // We will base this on the behavior of worker to mark its current index then work to the latest available
            // index. This works well with looping the array
            // I need to do this in a way that minimizes simultaneous reads, but I want 24/7 uptime on workers
            if (chunk_iter >= end_chunk && batch_buffer.empty()) {
                // while loop break logic
                break;
            }
        } else {
            batch[batch_counter] = std::make_unique<TaskTerrainGen>(chunk_iter, &map);
            batch_counter++;
            chunk_iter++;
        }
        //End of loop
    }
    //End of func
}

void TaskHandler::loadWorkers() {
    work_array.resize(num_workers); //Resize outer vector to # of buckets
    for (int i = 0; i < num_workers; i++) {
        work_array[i].resize(bucket_max_capacity); //Resize 1 depth vector to # of batches per bucket
    } //Fill up the first vec with empty Task pointer vecs.
    //For the third layer of the vector, size of Batches can be changed runtime

    //Further functionality means # of workers can change as well.

    {
        std::vector<std::atomic<int>> temp(num_workers);
        for (int i = 0; i < num_workers; i++) {
            temp[i] = 0;
        }
        work_indexes.swap(temp);
    }

    {
        std::vector<std::atomic<int>> temp(num_workers);
        for (int i = 0; i < num_workers; i++) {
            temp[i] = 0;
        }
        bucket_sizes.swap(temp);
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
        logger << "Workers: " << num_workers << " | Loaders: " << num_loaders << " | Buckets: " << bucket_max_capacity
               << "\n";
    }

}

void TaskHandler::workerThread(int ID) {

    int batches_to_execute = 0;
    int no_batches_count = 0;
    int work_index = 0;
    while (!is_stopped.load(std::memory_order_relaxed)) { //Don't really like that it leaves work incomplete
        batches_to_execute = bucket_sizes[ID].load(std::memory_order_relaxed);
        if (batches_to_execute != 0) {
            for (int i = 0; i < batches_to_execute; i++) {
                //update halfway? Or every 5 batches?
                for (auto &task: work_array[ID][(work_index + i) % 10]) {
                    //Ensured WorldMap isn't destroyed between tasks
                    if (task != nullptr) { //At the end, we end up with having nullptr tasks. Strip them out?
                        task->execute();
                    }

                }
                bucket_sizes[ID]--;
                work_index++;
            }
            work_indexes[ID].store(work_index, std::memory_order_relaxed);
        } else {
            no_batches_count++;
        }
    }

    std::cout << "Thread " << ID << " no batch count: " << no_batches_count << std::endl;

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
 * Also changes to current: bucket_max_capacity needs to work regardless of size of map.
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
 * Can't have workers * bucket_max_capacity > x_dimension, no checks for end of chunks
 *
 * As seen from the log, bucket size up is good. The contention for atomic or the wait time is causing problems.
 * We probably want more loaders than workers, especially for super cheap tasks.
 *
 *
 */