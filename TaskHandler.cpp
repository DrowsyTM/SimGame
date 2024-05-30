#include "TaskHandler.h"

TaskHandler::TaskHandler() : tasks{}, locks{}, numThreads{10} {
    for (int i = 0; i < numThreads; i++) {
        workers.push_back(std::jthread([this]() { this->workerThread(); }));
    }
}

TaskHandler::~TaskHandler() {}

//---------------Private-------------------
void TaskHandler::workerThread() {
    // loops until work exists (conditional variable?)
    while (true) {
        break;
        getNext()->get()->execute();
        //somehow delete the task from vector?
        locks[-1].unlock(); //how do I get the index of the task I just did
    }
}

std::optional<std::unique_ptr<Task>> TaskHandler::getNext() {
    for (int i = 0; i < tasks.size(); i++){
        if (tryLock(i)){
            return std::move(tasks[i]); //Hopefully this gives ownership of the task to the workerThread
            //Hmm how do we unlock and how do we clear out this task
        }
    }
    return std::nullopt;
}

bool TaskHandler::tryLock(int index) {
    if (locks[index].try_lock()) {
        return true;
    }
    return false;
}

//---------------Work with External Objects-------------------
void TaskHandler::taskMapGeneration(WorldMap map) {
    int num_chunks = map.numChunks();
    for (int i = 0; i < num_chunks; i++) {
        tasks.push_back(std::make_unique<TaskTerrainGen>(i, &map));
    }
}



