#include "TaskHandler.h"

//Delegates to full constructor
TaskHandler::TaskHandler() : TaskHandler(10, 10, 10, false) {}

//Delegates to full constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize)
        : TaskHandler(workerCount, loaderCount, bucketSize, false) {}

//Full Constructor
TaskHandler::TaskHandler(int workerCount, int loaderCount, int bucketSize, bool doLogging)
        : num_workers{workerCount}, num_loaders{loaderCount}, bucket_size(bucketSize), logger_flag{doLogging},
          stop_flag{false} {


    work_arrays.resize(workerCount);
    for (int i = 0; i < workerCount; i++) {
        work_arrays[i].resize(10); //
    }

    for (int i = 0; i < num_workers; i++) {
        workers.emplace_back([this]() { /**work function with ID**/ });
    }
}


void TaskHandler::LoadingBay(WorldMap &map) {
    for (int i = 0; i < num_workers; i++) {
        loaders.emplace_back([this, &map, &i]() { loadMapTasks(map, i); });
    }
}

void TaskHandler::loadMapTasks(WorldMap &map, int ID) {
    int amount = map.numChunks() / num_loaders;
    int start = amount * ID;


}

//TaskHandler::~TaskHandler() {
//
//    stopSource.request_stop();
//    cv.notify_all();
//
//    for (auto &worker: workers) {
//        if (worker.joinable()) {
//            worker.join();
//        } else {
//            std::cout << "Worker didn't join\n";
//        }
//    }
//}
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