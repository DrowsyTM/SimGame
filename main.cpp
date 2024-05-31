#include <iostream>
#include <string>
#include <fstream> //Files
#include <chrono> //Time
#include <thread> //Create concurrent threads
#include <mutex> //Give thread exclusive access to a variable/object
#include <atomic> //Merge operations and make it "atomic"(indivisible), no intermediate steps
#include <random> //Random generator
#include <functional> //Used for bind function
#include <format>

#include "WorldMap.h"
#include "TaskHandler.h"

const int MAP_DIMENSION_UPPER_LIMIT = 15000;

using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;

struct ForestNode {
    int x, y;
    bool map[15][15];
};
ForestNode forestList[4000];
int forestAmount = 0;

int GameTick();

/**
void ProfileFunctions() {

    const int iterations = 1000;
    double time = 0; //ms

    string input;
    cout << "Custom Message:\n> ";
    cin.sync();
    getline(cin, input);

    time_point currTime = system_clock::now();
    auto timeNow = system_clock::to_time_t(currTime); //C++11 tho, where is C++20, formatting?
    ofstream logFile("profileLog.txt", fstream::app);

    logFile << "[ " << input << " ]" << "\n" << ctime(&timeNow) << "\n";

    forestAmount = 0;
    // Profile LocalFeatureGen
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        LocalFeatureGen();
    }
    auto stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    logFile << "Avg LocalFeatureGen Time: \t" << time / iterations << " ms\n";

    forestAmount = 0;
    //Profile GenerateForest
    uniform_int_distribution<int> dist32(0, 31);
    start = high_resolution_clock::now();
    auto d32 = bind(dist32, randomGenerator);
    for (int i = 0; i < iterations; ++i) {
        GenerateForest(d32(), d32());
    }
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    logFile << "Avg GenerateForest Time: \t" << time / iterations << " ms\n";

    // Profile MergeFeatures
    cout << "Nodes: " << forestAmount << "\n";
    start = high_resolution_clock::now();
    MergeFeatures();
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    logFile << "Avg MergeFeature Time: \t\t" << time / iterations << " ms\n";

    //Profile Map Constructor + Destructor
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        WorldMap test;
    }
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    logFile << "Avg WorldMap Build Time: \t" << time / iterations << " ms\n";

    //Profile Map Draw
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        static WorldMap test;
        test.draw();
    }
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    logFile << "Avg WorldMap Draw Time: \t" << time / iterations << " ms\n";

    logFile << "\n";
    logFile.close();
}
**/

int main() {
    //    jthread mapThread([]() { MapGenerator(32, 500); }); //Lambda func
    //Hmm just implement TPS and use that to measure changes?

    WorldMap map;
    TaskHandler tasker;
    tasker.taskMapGeneration(map);
    map.draw();

    cin.ignore(); // Wait for user input before ending
    return 0;
}

// Consecutive implementation
int GameTick() {


    this_thread::sleep_for(1ms); //1000 TPS Max

    return 0;
}

/**
void GenerateForest(const int x, const int y) {
    static uniform_int_distribution<int> dist02(-1, 1);
    static auto d02 = bind(dist02, randomGenerator);

    ForestNode newForest;
    newForest.x = x;
    newForest.y = y;

    static int iy, ix;
    for (int y = 0; y < 15; y++) {
        iy = y - 7 + d02();
        for (int x = 0; x < 15; x++) {
            ix = x - 7 + d02();

            if ((abs(iy) + abs(ix)) * 0.1 < 0.5) {
                newForest.map[y][x] = 1;
            } else {
                newForest.map[y][x] = 0;
            }
        }
    }
    forestList[forestAmount] = newForest;
    forestAmount++;

    // Also forestNode?
    // Node gets its own map. Function that takes this map and puts it onto world?

}
**/

//For forests, either remove them from terrain gen or spawn them super rare with a custom terrain symbol
// Then go around and place forest clumps at those terrain synmbols
// Or change how worldGen works. First generate ground, then "ecosystems", then "stuctures", in a systematic way
// This would also allow layers. I can have a ground layer, then layers for other stuff.
// How to display tho?? Feels like 3D is just has a lot more potential, or at least 3D on 2D plane?

// Pre-generates local play area (reduce loading time & load rest of map later)
// Need to start near middle of map or something. Or random spot

//Figure out how to make all this work for GlobalMap. Maybe a "chunk" system? Rolls per chunk for feature?
//Rolls per chunk means we can just roll for the chunk then randomly place it
//Or maybe just no chunks and some sort of probability system that keeps in track how close you are
//To another forest. Low chance to be really close, becomes more likely farther away, more consistent
//Or chunks + probability distance. Dunno, not sure why I use chunks(usually for 3d optimization?)
//Though chunks would be about unloading chunks. We want to update whole world.
//Or maybe chunks work with the retroactive updates to AI and world updates?

//Maybe one WorldMapGen that simply stops

//For map rather than iterating every single coordinate, I probably want to
//Transform noise/map into what I need. With matrix stuff? Or is this more calculations. Large noise map?

//Ahh, world gen should probably run on multiple threads to accelerate it



/** TODO
 Allow partial chunks or force expand map to minimum chunk limit(so that map dimensions are not multiple of chunk_size)
 Save each seperate line of description, then paste it between map draws?
 Maybe make abstract class for all features?
 Measure time execution of GenerateForest
 Fix GenerateForest somehow bloating LocalMapGen (figure out threading for world gen as well)
 Use heap for generate forest?
 Fill in GenerateForest, then maybe switch to noisemap or something
 ForestNodes + resource tracking, stats
 Ooh maybe spawn ForestNode, it generates itself, merges if need be, etc?
 Work on map generator, forest gen
 Make TaskHandler
 Timeline system
 Figure out layers (FOREST probably shouldn't be a terrain tile)
 Forest mechanics, balancing, wood, plants, animals, prey, predator, hunting, monsters
 OPTIMIZATION!!
 CLASSES? Class for map?
 Implement Menu
 MORE FUNCTIONS, LESS NESTING, SIMPLE FUNCTIONS
 Figure out formatting library, mutex, atomic
 Make Forest "Node" that keeps track of stats and updates? Also has map of forest. Simple 0 / 1 binary map

 IMPELMENT TRY/CATCH IN TASKHANDLER
**/
