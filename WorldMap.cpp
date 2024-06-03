#include "WorldMap.h"

//Public
WorldMap::WorldMap() : WorldMap(1000) {} //multiple of chunk_size <- change how this works

WorldMap::WorldMap(int size)
        : map{}, map_size{size}, lookX{0}, lookY{0}, screen_size{35}, initialized{false}, num_chunks{100} {

    map.resize(map_size, std::vector<terrain>(map_size, BLANK)); //Map default
    chunk_size = sqrt(size * size / num_chunks);
    spawnRandomzier(lookX, lookY);
}

//Based on how many threads, we split up the entire map. Then we use start and end row for batch generation.

WorldMap::~WorldMap() {}

// Non-threaded terrain generation
void WorldMap::generateTerrain() {
    std::vector<terrain> terraVector(map_size, PLAINS);
    map.resize(map_size, terraVector);// Fill with plains
}

void WorldMap::threadGenerateTerrain(int chunk) {
    int chunksDimension = map_size / chunk_size;

    // Chunk 0-49 = 0-49 * 15
    int xOffset = (chunk % chunksDimension) * chunk_size;
    // Chunk 0-49 = 0, 50-99 = 1, etc * 15
    int yOffset = (chunk / chunksDimension) * chunk_size;

    //maybe make the conditions use offset. Might be faster
    for (int y = 0; y < chunk_size; y++) {
        for (int x = 0; x < chunk_size; x++) {
            map[y + yOffset][x + xOffset] = PLAINS; //Uses chunk offset
        }
    }
}

void WorldMap::draw() const {

    std::stringstream ss;
    int xOffset = lookX - (screen_size - 1) / 2;
    int yOffset = lookY - (screen_size - 1) / 2;

    ss << "[ " << lookX << ", " << lookY << " ]\n";
    for (int y = 0; y < screen_size; y++) {
        for (int x = 0; x < screen_size; x++) {
            if (x == (screen_size - 1) / 2 && y == (screen_size - 1) / 2) {
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

void WorldMap::printMap() const {

    std::stringstream ss;
    std::ofstream output_file("map_output.txt", std::fstream::trunc);

    for (int y = 0; y < map_size; y++) {
        for (int x = 0; x < map_size; x++) {

            //Make this pairs with keys
            switch (map[y][x]) {
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
        ss << "\n";
    }
    ss << "\n";
    output_file << ss.str() << std::endl;
    output_file.close();

    std::cout << "Printed map to txt\n";
}

void WorldMap::set(terrain type, int x, int y) {
    map[y][x] = type;
}

int WorldMap::size() const {
    return map_size;
}

void WorldMap::clear() {

}

bool WorldMap::isEmpty() const {
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

int WorldMap::numChunks() const {
    return num_chunks;
}

void WorldMap::setInitialized() {
    initialized = true;
}

//Private
void WorldMap::spawnRandomzier(int &x, int &y) {
    // Randomness setup
    int center = map_size / 2;
    int halfMap = 0.25 * map_size;
    int halfToCenter = center - halfMap;
    int halfFromCenter = center + halfMap;
    std::default_random_engine randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> spawnDistribution(halfToCenter, halfFromCenter);

    //Spawn Coordinates
    x = spawnDistribution(randomGenerator);
    y = spawnDistribution(randomGenerator);
}

//wtf is this function
void WorldMap::updateLayer() {}









