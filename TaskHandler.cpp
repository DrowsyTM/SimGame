#include "TaskHandler.h"

//Delegates to full constructor
TaskHandler::TaskHandler() : TaskHandler(5, 5, 10, false) {}

//Delegates to full constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize)
        : TaskHandler(workerCount, loaderCount, bucketSize, false) {}

//Full Constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize, bool doLogging)
        : num_workers{workerCount}, num_loaders{loaderCount}, bucket_size(bucketSize), logger_flag{doLogging},
          stop_flag{false}, loader_cvs{}, loaders{}, workers{} {

    if (logger_flag) {
        loadLogger(); //Loads logging system
    }

    loadWorkers(); //Loads workers and work_arrays
    loadLoaderArrays(); //Load load_arrays
}

TaskHandler::~TaskHandler() {
    stop_flag.store(true);
    if (logger_flag.load(std::memory_order_relaxed)) {
        logger << "Stop flag set" << std::endl;
    }
    int ID = 0;
    for (auto &worker: workers) {
        if (logger_flag.load(std::memory_order_relaxed)) {
            logger << "Joining thread " << ID << "..." << std::endl;
        }
        if (worker.joinable()) {
            worker.join();
        }
        ID++;
    }
    if (logger_flag.load(std::memory_order_relaxed)) {
        logger << "Workers joined" << std::endl;
    }

    if (!loaders.empty()) {
        for (int i = 0; i < num_loaders; i++) {
            if (loaders[i].joinable()) {
                loaders[i].join();
            }
        }
    } else if (logger_flag.load(std::memory_order_relaxed)) {
        std::lock_guard lock(logger_mtx);
        logger << "Loaders never initialized" << std::endl;
    }
    if (logger_flag.load(std::memory_order_relaxed)) {
        logger << "Destructor fully executed" << std::endl;
        logger.close();
    }

}

void TaskHandler::LoadingBay(WorldMap &map) {
    for (int i = 0; i < num_loaders; i++) {
        loaders.emplace_back([this, &map, i]() { loadMapTasks(map, i); });
    }
    if (logger_flag.load(std::memory_order_relaxed)) {
        std::lock_guard lock(logger_mtx);
        logger << "Loaded loaders" << std::endl;
    }
}

//---------------------PRIVATE----------------------------


void TaskHandler::loadMapTasks(WorldMap &map, int ID) {
    // chunkCount is how many chunks will be loaded by this laoder
    int chunkCount = map.numChunks() / num_loaders;
    int start = chunkCount * ID;
    int end = start + chunkCount;
    if (ID == num_loaders - 1) {
        end = map.numChunks();
    }
    //ID corresponds to bucket

    //For amount = 250, ID is 0-9
    //0-249, 250-499, 500-749 ... 2250-2499
    int row = 0;

    //Ok first of all we don't account for the for loop ending before we give the last Tasks
    // We have some ungenerated chunks, this might be why multithreading seems to break at the very end.
    for (int chunk = start; chunk < end; chunk++) {
        if (row != bucket_size) { //Not at end of bucket
            load_arrays[ID][row] = std::make_unique<TaskTerrainGen>(chunk, &map);
            row++; //Iterate bucket row
        }
        if (row == bucket_size) { //End of bucket
//            int iter = 0;

            while (working_flag[ID].load(std::memory_order_relaxed)) { //While worker is working - flag is true
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
            work_arrays[ID] = std::move(load_arrays[ID]);
            working_flag[ID].store(true, std::memory_order_relaxed);
            load_arrays[ID].resize(bucket_size);
            //Worker should now start working
            row = 0;
        }
    }
}

void TaskHandler::loadWorkers() {
    work_arrays.resize(num_workers);
    for (int i = 0; i < num_workers; i++) {
        work_arrays[i].resize(bucket_size); //
    } //Fill up the first vec with empty Task pointer vecs.


    std::vector<std::atomic<bool>> temp(num_workers);
    working_flag.swap(temp); //atomics can't be moved...
    for (auto &flag: working_flag) {
        flag.store(false, std::memory_order_relaxed);
    } //all working_flags are false

    for (int i = 0; i < num_workers; i++) {
        workers.emplace_back([this, i]() { this->workerThread(i); });
    }

    {
        std::lock_guard lock(logger_mtx);
        logger << "Loaded workers, work arrays, and work_flags\n";
    }
}

void TaskHandler::loadLoaderArrays() {
    load_arrays.resize(num_loaders);
    for (int i = 0; i < num_loaders; i++) {
        load_arrays[i].resize(bucket_size); //
    }
    if (logger_flag) {
        std::lock_guard lock(logger_mtx);
        logger << "Loaded loader arrays" << std::endl;
    }
}

void TaskHandler::loadLogger() {
    std::string logName = "HandlerLog.txt";
    logger.open(logName, std::fstream::app);
    if (!logger.is_open()) {
        logger_flag = false;
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

    while (!stop_flag.load(std::memory_order_relaxed)) {
        int iter = 0;
        //While corresponding flag is false (default) & < 0.1 ms of wait
        while (!working_flag[ID].load(std::memory_order_relaxed) && iter < 100000) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            iter++;
        }
        if (iter >= 100000) { //Waiting function to not waste resources.
            if (logger_flag.load(std::memory_order_relaxed)) {
                std::lock_guard lock(logger_mtx);
                logger << "Thread " << ID << " went to long wait\n";
            }
            while (!working_flag[ID].load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                if (stop_flag.load(std::memory_order_relaxed)){
                    return;
                }
            }
        }
        iter = 0;


        for (auto &task: work_arrays[ID]) {
            task->execute(); //check if null? If array not full
        }
        working_flag[ID].store(false, std::memory_order_relaxed);
    }

}

/** TODO
 * Work with alternative ratios of worker/loaders,
 * Work with map not a multiple of chunks
 * Batches rather than static arrays
 *
 *
 */