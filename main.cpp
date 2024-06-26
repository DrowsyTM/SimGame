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


void ProfileFunctions(int workers, int loaders, int bucket_size, int map_size) {

    const int iterations = 100;
    double time = 0; //ms

    string input;
    cout << "Custom Message:\n> ";
    cin.sync();
    getline(cin, input);

    time_point currTime = system_clock::now();
    auto timeNow = system_clock::to_time_t(currTime); //C++11 tho, where is C++20, formatting?
    ofstream logFile("Logs/profileLog.txt", fstream::app);

    logFile << "[ " << input << " ]" << "\n" << ctime(&timeNow);
    logFile << "Iterations: " << iterations << endl;
    logFile << "Map Side: " << map_size << " | Total: " << map_size * map_size << endl;
    logFile << workers << " Workers, " << loaders << " Loaders, " << bucket_size << " Bucket Size\n" << endl;

    auto start = high_resolution_clock::now();
    auto stop = high_resolution_clock::now();


//    Profile TaskHandler Builder
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        TaskHandler test(workers, loaders, loaders);
    }
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    logFile << "Avg tHandler Build Time: \t" << time / iterations << " ms" << endl;

    //Profile destructor
    double total = 0;
    for (int i = 0; i < iterations; i++) {
        {
            TaskHandler tester(workers, loaders, bucket_size);
            start = high_resolution_clock::now();
        }
        stop = high_resolution_clock::now();
        total += static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    }
    double destructorTime = total / iterations;
    logFile << "Avg tDestructor Time: \t\t" << destructorTime << " ms" << endl;

    //Map init time
    total = 0;
    for (int i = 0; i < iterations; i++) {
        TaskHandler tester(workers, loaders, bucket_size);
        start = high_resolution_clock::now();
        tester.initializeMap(map_size, -1);
        stop = high_resolution_clock::now();
        total += static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    }
    logFile << "Avg WorldMap init Time: \t" << total / iterations << " ms" << endl;

    //Profile Threaded Map Generation
    total = 0;
    for (int i = 0; i < iterations; i++) {

        TaskHandler tester(workers, loaders, bucket_size);
        tester.initializeMap(map_size, -1);
        start = high_resolution_clock::now();
        tester.LoadingBay();
        tester.shutdownThreads();
        stop = high_resolution_clock::now();


        total += static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    }
    logFile << "Avg ThreadedGen Time: \t\t" << total / iterations - destructorTime << " ms" << endl;

    logFile << endl;
    logFile.close();
}

int main() { //Wait I've been calling map.draw() this whole time but this could cause read/write race conditions?

    ProfileFunctions(10, 10, 10, 10000);

//    TaskHandler tasker(10, 10, 10); //remove bucket-size option?
//
//    WorldMap *map = tasker.getMap();
//    tasker.initializeMap(10000, -1);
//    auto start = high_resolution_clock::now();
//    tasker.LoadingBay();
//    tasker.shutdownThreads();
//    auto stop = high_resolution_clock::now();
//    cout << "Generation time:\t" << static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000 <<
//         " ms" << endl;
//    map->draw(); //This isn't thread safe I don't think

//    map.printMap(); //can't call this while map is being generated. I gotta figure out a restriction just in case;

    // f me if I need to manage race conditions for map as well if I access it while generating it.
    // If I ever have to generate or add stuff, I can simply do it on a smaller temp map, then line it up and add it
    // between map draws. Or a temp overlay to the whole map which is then merged or something.
    // I know games have some sort of system where they execute certain updates in a certain order. If I have consistent
    // draws, it probably wouldn't be hard to do it with one function.
    // But that's as long as I keep my GameLoop one one thread

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
 separate the loading of loaders and the task generation(so we can time each step). Build it in timings?
 Want detailed measurements of each step, so I know what to optimize. Go in and set it up, then go about optimization.
 Idc about actual time, we can just time the execution of the sections even if it slows down the program.
 Hide it behind a flag. Ooh I could individual time each thread as well? They accumulate their results in a output buffer-
 which they later

 Alternatively to having each thread time itself, a separate thread could? Though there won't be much difference.
 It makes sense for a logger, but not a thread.

 STRUCTS

 Bypass need to initialize the whole map first?

 Paper down the process and try to make it more efficient

 After done figuring out worker/loader implementation, we might want to hard code terrain generation? Tbh impact is
 minimal, and we could give different tasks to different thread so its chill.

 Make struct that associates information rather than parallel vectors?
 Also make more functions (first test current system)
 Revert to more basic functionality if 1:1?

 Make better destructor

 Logging should be done in one place, based on information given by threads. Logging thread.

 Better profiling, historical comparison?
 Make logs in a specific folder

 Async might be a better way of doing some of my things.

 Currently map size is multiple of chunk_size. Fix that. Maybe make chunk_size dynamically sized.

 JUST do godamnn buckets. Currently, each task is so fast that the delay and overhead from multithreading just obliterates performance
 No need to even work-steal at the current stage. One thread is constantly caught up on the work.
 Ooh use multiple threads to distribute at once? Maybe even one loader thread per buckets(system should balance loads)
 Wonder if I can deposit multiple tasks at once into queue.
 10 Loaders, 10 Workers. Can probably integrate this into the chunk distribution as well.
 Some sort of "Loading Bay" method?
 Can be used for MapGeneration, but its main use is later for Timeline updates. Just if statements prob work.

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

**/
