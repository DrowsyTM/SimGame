#ifndef SIMGAME_TASK_H
#define SIMGAME_TASK_H

#include <string>
#include "WorldMap.h"

class Task { // turn into abstract?
public:
    virtual ~Task() = default;

    virtual void execute() = 0;

    struct Data {
        unsigned int three_byte1: 24; //Or maybe combine multiple ints to do this
        uint64_t pointer;
//        unsigned int flag1: 1;
//        unsigned int flag2: 1;
//        unsigned int three_bit1: 3;
//        unsigned int three_bit2: 3;
//        unsigned int two_byte1: 16;
//        unsigned int two_byte2: 16;
    };
};

class TaskTerrainGen : public Task {
public:
    TaskTerrainGen(int chunk, WorldMap* mapPointer);
    void execute() override;

private:
    Data taskData;
    //three_byte1 stores chunk #
};


#endif //SIMGAME_TASK_H
