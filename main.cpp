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

const int MAP_DIMENSION_UPPER_LIMIT = 15000;

using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;

const int LMAP = 32; // Size of initial map
//Temporarily global, will change to reference when I get to it
terrain worldMap[LMAP][LMAP]; //2D enum array of size LMAP - should be much larger than LMAP
// Maybe create custom class for it to create custom behavior
// Also iteration would be nice and simplifies code
default_random_engine randomGenerator(system_clock::now().time_since_epoch().count());

struct ForestNode {
    int x, y;
    bool map[15][15];
};
ForestNode forestList[4000];
int forestAmount = 0;

int GameTick();

void LocalFeatureGen(); // Generates features (forest)
void GenerateForest(const int x, const int y);

void MergeFeatures();

void Draw();

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

int main() {
//    void (*limitedMapGenerator)(int, int) = MapGenerator; //function pointer to overload
//    jthread mapThread(limitedMapGenerator, 32, 500); // Start generating local map
//    jthread mapThread([]() { MapGenerator(32, 500); }); //Lambda func

//Hmm just implement TPS and use that to measure changes?

    WorldMap test;
    cout << test.numChunks();

    if (false) {
        jthread profileThread(ProfileFunctions);
    }

    // menu();

    cin.ignore(); // Wait for user input before ending
    return 0;
}

// Consecutive implementation
int GameTick() {


    this_thread::sleep_for(1ms); //1000 TPS Max

    return 0;
}

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

void LocalFeatureGen() {
    //For features, rather than iterate, we can just aim for a random spot since we don't fill everything

    static uniform_int_distribution<int> dist32(0, LMAP - 1);
    static auto d32 = bind(dist32, randomGenerator);


    //Forest chance 1/500
    int mapSize = LMAP * LMAP;//Max forests
    int rolls = mapSize / 500 + 1;
    //cout << "Num Forest Rolls: " << rolls << "\n";

    //Iterate through whole thing for now, annoying, implement some dice globally?

    for (; rolls > 0; rolls--) {
        GenerateForest(d32(), d32());
    }
}

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

void MergeFeatures() {
    //Just write local map -> world map translation function?
    //Maybe make it part of Forest function
    for (int i = 0; i < forestAmount; i++) {
        ForestNode node = forestList[i];
        for (int y = 0; y < 15; y++) {

            bool validY = y + node.y - 7 >= 0 && y + node.y - 7 < LMAP;
            for (int x = 0; x < 15; x++) {
                bool validX = node.map[y][x] == true && x + node.x - 7 >= 0 && x + node.x - 7 < LMAP;
                if (validX && validY) {
                    worldMap[y + node.y - 7][x + node.x - 7] = FOREST;
                }
            }
        }
    }
}



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
**/
