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

    loadLogger(); //Loads logging system

    loadWorkers(); //Loads workers and work_arrays
    loadLoaderArrays(); //Load load_arrays

    std::cout << load_arrays.size();
}

TaskHandler::~TaskHandler() {
    for (int i = 0; i < num_workers; i++) {
        if (workers[i].joinable()) {
            workers[i].join();
        } else if (logger_flag) {
            logger << "Worker " << i << "didn't join\n";
        }
    }
    if (!loaders.empty()) {
        for (int i = 0; i < num_loaders; i++) {

            if (loaders[i].joinable()) {
                loaders[i].join();
            } else if (logger_flag) {
                logger << "Loader " << i << "didn't join\n";
            }

        }
    } else if (logger_flag) {
        logger << "No loaders present at destruction\n";
    }

    if (logger_flag) {
        logger.close();
    }
}


void TaskHandler::LoadingBay(WorldMap &map) {
    loadLoaderArrays();
    for (int i = 0; i < num_loaders; i++) {
        loaders.emplace_back([this, &map, &i]() { loadMapTasks(map, i); });
    }
}

//---------------------PRIVATE----------------------------

void TaskHandler::loadMapTasks(WorldMap &map, int ID) {
    // chunkCount is how many chunks will be loaded by this laoder
    int chunkCount = map.numChunks() / num_loaders;
    int start = chunkCount * ID;

    //ID corresponds to bucket

    //For amount = 250, ID is 0-9
    //0-249, 250-499, 500-749 ... 2250-2499
    int row = 0;
    for (int chunk = start; chunk < start + chunkCount; chunk++) {
        if (row != bucket_size) { //Not at end of bucket
            load_arrays[ID][row] = std::make_unique<TaskTerrainGen>(row, &map);
            row++; //Iterate bucket row

        } else { //End of bucket

//            int iter = 0;
            while (working_flag[ID].load(std::memory_order_relaxed)) { //While worker is working - flag is true
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
            work_arrays[ID] = std::move(load_arrays[ID]);
            working_flag[ID].store(true, std::memory_order_relaxed);
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

    for (int i = 0; i < num_workers; i++) {
        workers.emplace_back([this]() { /**work function with ID**/ });
    }


    logger << "Loaded workers, work arrays, and work_flags\n";

}

void TaskHandler::loadLoaderArrays() {
    load_arrays.resize(num_loaders);
    for (int i = 0; i < num_loaders; i++) {
        load_arrays[i].resize(bucket_size); //
    }
}

void TaskHandler::loadLogger() {
    if (logger_flag) {
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
}


//
//int TaskHandler::numTasks() {
//    return tasks.size();
//}
//
//void TaskHandler::waitOnEmpty() {
//    while (!tasks.empty()) {
//    }
//    return;
//}
//
////---------------Private-------------------
//
////Waits on queue, executes tasks from queue
//void TaskHandler::workerThread() {
//    while (true) {//I think this should be true instead
//        std::unique_ptr<Task> task;
//        {
//            std::unique_lock<std::mutex> lock(mtx);
//            cv.wait(lock, [this] { return !tasks.empty() || stopToken.stop_requested(); });
//            if (stopToken.stop_requested() && tasks.empty()) {
//                return;
//            }
//            if (!tasks.empty()) {
//                task = std::move(tasks.front());
//                tasks.pop();
//            }
//        }
//        if (task) {
//            try { //If execution fails for some reason, catches exception and prints it.
//                task->execute();
//            } catch (const std::exception &e) {
//                std::cout << e.what();
//            }
//        }
//    }
//}
//
//
////Safely pushes onto queue
//void TaskHandler::push(std::unique_ptr<Task> task) {
//    std::lock_guard<std::mutex> lock(mtx);
//    {
//        tasks.push(std::move(task));
//    }
//    cv.notify_one(); //lets a thread through?
//}
//
//
////---------------Work with External Objects-------------------
//void TaskHandler::taskMapGeneration(WorldMap &map) {
//    int num_chunks = map.numChunks();
//    //Prevents multiple threads from doing this, but I'm not sure it matters
//    for (int i = 0; i < num_chunks; i++) {
//        push(std::make_unique<TaskTerrainGen>(i, &map));
//    }
//    std::cout << "Generated " << num_chunks << " chunks.\n";
//}
//
//
//
//
//
//
//// Cleaner process should be its own task with .execute() ? Maybe
//// No cleaner, just blacklist index and replace it?