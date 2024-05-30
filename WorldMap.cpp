#include "WorldMap.h"

//Public
WorldMap::WorldMap() : WorldMap(750) {} //multiple of chunkSize

WorldMap::WorldMap(int size) : map{}, maxSize{size}, lookX{0}, lookY{0}, screenSize{35}, initialized{false}, chunk_size{15} {
//  std::jthread genThread(&WorldMap::generateMap, this);
//    std::jthread genThread([this]() { this->generateTerrain(); }); //Call generateTerrain in thread
    spawnRandomzier(lookX, lookY);
}

//Based on how many threads, we split up the entire map. Then we use start and end row for batch generation.

WorldMap::~WorldMap() {}

void WorldMap::draw() const{

    std::stringstream ss;
    int xOffset = lookX - (screenSize - 1) / 2;
    int yOffset = lookY - (screenSize - 1) / 2;

    ss << "[ " << lookX << ", " << lookY << " ]\n";
    for (int y = 0; y < screenSize; y++) {
        for (int x = 0; x < screenSize; x++) {
            if (x == (screenSize - 1) / 2 && y == (screenSize - 1) / 2) {
                ss << "v ";
            } else {
                //Make this pairs with keys
                switch (map[y + yOffset][x + xOffset]) {
                    case PLAINS:
                        ss << "_ ";
                        break;
                    case FOREST:
                        ss << "F ";
                        break;
                    case FCENTER:
                        ss << "C ";
                        break;
                    default:
                        ss << "? ";
                        break;
                }
            }
        }
        ss << "\n";
    }
    ss << "\n";
    std::cout << ss.str() << std::endl;
}

void WorldMap::set(terrain type, int x, int y) {
    map[y][x] = type;
}

int WorldMap::size() const{
    return maxSize;
}


void WorldMap::clear() {

}

bool WorldMap::isEmpty() const{
    return initialized;
}

int WorldMap::x() const {
    return lookX;
}

int WorldMap::y() const {
    return lookY;
}

int WorldMap::chunkSize() const {
    return chunk_size;
}

//Private
void WorldMap::generateTerrain() {
    std::vector<terrain> terraVector(maxSize, PLAINS);
    for (int i = 0; i < maxSize; i++) {
        map.push_back(terraVector);
    } // Fill with plains
}

void WorldMap::threadGenerateTerrain() {

}

void WorldMap::spawnRandomzier(int &x, int &y) {
    // Randomness setup
    int center = maxSize / 2;
    int halfMap = 0.25 * maxSize;
    int halfToCenter = center - halfMap;
    int halfFromCenter = center + halfMap;
    std::default_random_engine randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> spawnDistribution(halfToCenter, halfFromCenter);

    //Spawn Coordinates
    x = spawnDistribution(randomGenerator);
    y = spawnDistribution(randomGenerator);
}




