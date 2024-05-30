#include "WorldMap.h"

//Public
WorldMap::WorldMap() : WorldMap(500) {}

WorldMap::WorldMap(int size) : map{}, maxSize{size}, lookX{0}, lookY{0}, screenSize{35} {
//  std::jthread genThread(&WorldMap::generateMap, this);
    std::jthread genThread([this]() { this->generateTerrain(); }); //Call generateTerrain in thread
    spawnRandomzier(lookX, lookY);
}

void WorldMap::generateTerrain() {
    std::vector<terrain> terraVector(maxSize, PLAINS);
    for (int i = 0; i < maxSize; i++) {
        map.push_back(terraVector);
    } // Fill with plains
}

WorldMap::~WorldMap() {}

void WorldMap::draw() {
    int xOffset = lookX - (screenSize - 1) / 2;
    int yOffset = lookY - (screenSize - 1) / 2;

    std::cout << "[ " << lookX << ", " << lookY << " ]\n";
    for (int y = 0; y < screenSize; y++) {
        for (int x = 0; x < screenSize; x++) {
            if (x == (screenSize - 1) / 2 && y == (screenSize - 1) / 2) {
                std::cout << "v ";
            } else {
                //Make this pairs with keys
                switch (map[y + yOffset][x + xOffset]) {
                    case PLAINS:
                        std::cout << "_ ";
                        break;
                    case FOREST:
                        std::cout << "F ";
                        break;
                    case FCENTER:
                        std::cout << "C ";
                        break;
                    default:
                        std::cout << "? ";
                        break;
                }
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n" << std::endl;
}

void WorldMap::set(terrain type, int x, int y) {
    map[y][x] = type;
}

void WorldMap::clear() {

}

bool WorldMap::isEmpty() {
    return false;
}

int WorldMap::x() {
    return lookX;
}

int WorldMap::y() {
    return lookY;
}


//Private
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






