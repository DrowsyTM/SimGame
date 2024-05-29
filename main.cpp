#include <iostream>
#include <string>
#include <fstream> //Files
#include <chrono> //Time
#include <thread> //Create concurrent threads
#include <mutex> //Give thread exclusive access to a variable/object
#include <atomic> //Merge operations and make it "atomic"(indivisible), no intermediate steps
#include <random> //Random generator
#include <functional> //Used for bind function
#include <memory> //unique_ptr, etc - safe pointers

using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;

enum terrain {
    PLAINS, FOREST
};

const int LMAP = 32; // Size of initial map
//Temporarily global, will change to reference when I get to it
terrain worldMap[LMAP][LMAP]; //2D enum array of size LMAP - should be much larger than LMAP
// Maybe create custom class for it to create custom behavior
// Also iteration would be nice and simplifies code

default_random_engine ranGen(chrono::system_clock::now().time_since_epoch().count());

bool debug = true;

int GameTick();

void LocalMapGen(); //Generates local map
void LocalTerraGen();  // Generates terrain
void LocalFeatureGen(); // Generates features (forest)

void Draw();

void ProfileFunctions() {
    cout << "Profile of all Functions\n\n";
    const int iterations = 1000;
    double time = 0; //ms

    // Profile LocalMapGen
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        LocalMapGen();
    }
    auto stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    cout << "Average LocalMapGen Time: \t" << time / iterations << " ms\n";

    // Profile LocalTerraGen
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        LocalTerraGen();
    }
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    cout << "Average LocalTerraGen Time: \t" << time / iterations << " ms\n";

    // Profile LocalFeatureGen
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        LocalFeatureGen();
    }
    stop = high_resolution_clock::now();
    time = static_cast<double>(duration_cast<microseconds>(stop - start).count()) / 1000;
    cout << "Average LocalFeatureGen Time: \t" << time / iterations << " ms\n";
}

int main() {

    if (debug) {
        jthread profileThread(ProfileFunctions);
    }

    jthread mapThread(LocalMapGen); // Start generating local map
    cout << "Generating Local Map...\n";

    mapThread.join(); // Wait for map to finish - wil remove this and the print and go into menu

//    Draw();

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
void LocalMapGen() {

    //int result = distribution(ranGen);

    LocalTerraGen();
    LocalFeatureGen();


}

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

void LocalTerraGen() {
    // Iterate through Local Map Size
    int temp;

    for (int y = 0; y < LMAP; y++) {
        for (int x = 0; x < LMAP; x++) {
            if (true) {
                worldMap[y][x] = PLAINS; // Only Plains impelmeneted so far
            }
        }
    }

}

void LocalFeatureGen() {
    //For features, rather than iterate, we can just aim for a random spot since we don't fill everything

    uniform_int_distribution<int> dist32(0, LMAP-1);
    auto d32 = bind(dist32, ranGen);

    uniform_int_distribution<int> dist1000(1, 1000);
    auto d1000 = bind(dist1000,ranGen);


    //Forest chance 1/500
    int mapSize = LMAP * LMAP;
    int rolls = mapSize / 500 + 1; //Max forests
    //cout << "Num Forest Rolls: " << rolls << "\n";

    //Iterate through whole thing for now, annoying, implement some dice globally?
    for (int y = 0; y < LMAP; y++) {
        for (int x = 0; x < LMAP; x++) {
            if (rolls > 0) {
                if (d1000() <= 2) { // 1/500 | 0.2%
                    worldMap[y][x] = FOREST;
                    // do genForest() function instead
                    rolls--;
                }
            } else {
                break;
            }
        }
    }
}

void Draw() {
    // Printing map
    for (int y = 0; y < LMAP; y++) {
        for (int x = 0; x < LMAP; x++) {
            cout << ((worldMap[y][x] == 0) ? "_ " : "F ");
        }
        cout << "\n";
    }
}



/** TODO
 MEASURE EXECUTION TIME OF EVERYTHING.
 LOGS? DEBUG MODE? Custom log message at the start of every one like (Change Terrain to Noise)
 Work on map generator, forest gen
 Make TaskManager
 Timeline system
 Figure out layers (FOREST probably shouldn't be a terrain tile)
 Forest mechanics, balancing, wood, plants, animals, prey, predator, hunting, monsters
 OPTIMIZATION!!
 CLASSES? Class for map?
 Implement Menu
 genForest() function
 Global dice?
 MORE FUNCTIONS, LESS NESTING, SIMPLE FUNCTIONS
**/
