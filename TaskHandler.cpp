#include "TaskHandler.h"

TaskHandler::TaskHandler()
        : tasks{}, numThreads{1}, cv{}, mtx{}, stopSource{},
          stopToken{stopSource.get_token()} {
    for (int i = 0; i < numThreads; i++) {
        workers.emplace_back([this]() { this->workerThread(); });
    }
}

TaskHandler::~TaskHandler() {

    stopSource.request_stop();
    cv.notify_all();

    for (auto &worker: workers) {
        if (worker.joinable()) {
            worker.join();
        } else {
            std::cout << "Worker didn't join\n";
        }
    }
}

int TaskHandler::numTasks() {
    return tasks.size();
}

void TaskHandler::waitOnEmpty() {
    while (!tasks.empty()) {
    }
    return;
}

//---------------Private-------------------

//Waits on queue, executes tasks from queue
void TaskHandler::workerThread() {
    while (true) {//I think this should be true instead
        std::unique_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !tasks.empty() || stopToken.stop_requested(); });
            if (stopToken.stop_requested() && tasks.empty()) {
                return;
            }
            if (!tasks.empty()) {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }
        if (task) {
            try { //If execution fails for some reason, catches exception and prints it.
                task->execute();
            } catch (const std::exception &e) {
                std::cout << e.what();
            }
        }
    }
}


//Safely pushes onto queue
void TaskHandler::push(std::unique_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mtx);
    {
        tasks.push(std::move(task));
    }
    cv.notify_one(); //lets a thread through?
}


//---------------Work with External Objects-------------------
void TaskHandler::taskMapGeneration(WorldMap &map) {
    int num_chunks = map.numChunks();
    //Prevents multiple threads from doing this, but I'm not sure it matters
    for (int i = 0; i < num_chunks; i++) {
        push(std::make_unique<TaskTerrainGen>(i, &map));
    }
    std::cout << "Generated " << num_chunks << " chunks.\n";
}






// Cleaner process should be its own task with .execute() ? Maybe
// No cleaner, just blacklist index and replace it?