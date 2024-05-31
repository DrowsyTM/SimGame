#include "TaskHandler.h"

TaskHandler::TaskHandler()
        : tasks{}, numThreads{10}, cv{}, mtx{}, stopSource{},
          stopToken{stopSource.get_token()} {
    for (int i = 0; i < numThreads; i++) {
        workers.emplace_back([this]() { this->workerThread(); });
    }
}

TaskHandler::~TaskHandler() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stopSource.request_stop();
        cv.notify_all();
    }
    while (!workers.empty()) {
        for (auto &worker: workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "Missed " << workers.size() << " thread(s).";
    }

}

//---------------Private-------------------

//Waits on queue, executes tasks from queue
void TaskHandler::workerThread() {
    while (!stopToken.stop_requested()) {//I think this should be true instead?
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !tasks.empty() || stopToken.stop_requested(); });
        if (stopToken.stop_requested()) {
            return;
        }
        auto task = pop();

        if (task) {
            try { //If execution fails for some reason, catches exception and prints it.
                task->execute();
            } catch (const std::exception &e) {
                std::cout << e.what();
            }
        }
    }
}

//Safely pops from queue and returns value
std::unique_ptr<Task> TaskHandler::pop() {
    std::lock_guard<std::mutex> lock(mtx);
    if (tasks.empty()) {
        return nullptr;
    }
    auto task = std::move(tasks.front()); //Hopefully works
    tasks.pop();
    return task; //compiler automatically moves
}

//Safely pushes onto queue
void TaskHandler::push(std::unique_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mtx);
    tasks.push(std::move(task));
    cv.notify_one(); //lets a thread through?
}


//---------------Work with External Objects-------------------
void TaskHandler::taskMapGeneration(WorldMap map) {
    std::lock_guard<std::mutex> lock(mtx);
    int num_chunks = map.numChunks();
    //Prevents multiple threads from doing this, but I'm not sure it matters
    for (int i = 0; i < num_chunks; i++) {
        push(std::make_unique<TaskTerrainGen>(i, &map));
    }
}





// Cleaner process should be its own task with .execute() ? Maybe
// No cleaner, just blacklist index and replace it?