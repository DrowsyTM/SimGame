#include "WorldMap.h"

//Public
WorldMap::WorldMap()
        : map{}, look_x{0}, look_y{0}, screen_size{35}, num_chunks{100} {
}

//Based on how many threads, we split up the entire map. Then we use start and end row for batch generation.

WorldMap::~WorldMap() {
}

void WorldMap::initializeObject(int map_x, int map_y) {
    x_dimension = map_x;
    y_dimension = x_dimension; //only care about squares for now

    map.resize(y_dimension, std::vector<terrain>(x_dimension, BLANK)); //Map default
    chunk_size = sqrt(x_dimension * y_dimension / num_chunks);
    spawnRandomzier(look_x, look_y);
}

// Non-threaded terrain generation
void WorldMap::generateTerrain() {
    std::vector<terrain> terraVector(x_dimension, PLAINS);
    map.resize(x_dimension, terraVector);// Fill with plains
}

void WorldMap::threadGenerateTerrain(int chunk) {
    int chunksDimension = x_dimension / chunk_size;

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

// Output player view to console
void WorldMap::draw() const {

    std::stringstream ss;
    int xOffset = look_x - (screen_size - 1) / 2;
    int yOffset = look_y - (screen_size - 1) / 2;

    ss << "[ " << look_x << ", " << look_y << " ]\n";
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

// Output full map to file
void WorldMap::printMap() const {

    std::stringstream ss;
    std::ofstream output_file("map_output.txt", std::fstream::trunc);

    for (int y = 0; y < x_dimension; y++) {
        for (int x = 0; x < x_dimension; x++) {

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

// Set a tile to specific terrain
void WorldMap::set(terrain type, int x, int y) {
    map[y][x] = type;
}

// Returns on of map dimensions
int WorldMap::size() const {
    return x_dimension;
}

// [Unimplemented] Sets world map to BLANK
void WorldMap::clear() {

}

// Returns true if initialized is false
bool WorldMap::isEmpty() const {
    return initialized;
}

// Returns look_x
int WorldMap::x() const {
    return look_x;
}

// Returns look_y
int WorldMap::y() const {
    return look_y;
}

// Returns chunk size (to be removed)
int WorldMap::chunkSize() const {
    return chunk_size;
}

// Returns # chunks (to be removed)
int WorldMap::getNumChunks() const {
    return num_chunks;
}

// Sets the map to initialized
void WorldMap::setInitialized() {
    initialized = true;
}

//----------Private

//Randomly sets look_x & look_y on the map. 25% buffer from edge
void WorldMap::spawnRandomzier(int &x, int &y) {
    // Randomness setup
    int center = x_dimension / 2;
    int halfMap = 0.25 * x_dimension;
    int halfToCenter = center - halfMap;
    int halfFromCenter = center + halfMap;
    std::default_random_engine randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> spawnDistribution(halfToCenter, halfFromCenter);

    //Spawn Coordinates
    x = spawnDistribution(randomGenerator);
    y = spawnDistribution(randomGenerator);
}














