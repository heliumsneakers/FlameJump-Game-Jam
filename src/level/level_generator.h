#ifndef LEVEL_GENERATOR_H
#define LEVEL_GENERATOR_H

#include "platform.h"
#include "raylib.h"

#define GRID_WIDTH    16   // cells horizontally
#define GRID_HEIGHT   24   // cells vertically (visible window)
#define CELL_WIDTH    2.0f // world units per cell (x)
#define CELL_HEIGHT   4.0f // world units per cell (y)

#define START_FLOOR_PLATFORMS 5  // contiguous starting floor

typedef struct LevelGenerator {
    Platform *prototype;                       // reusable platform model
    Platform *grid[GRID_HEIGHT][GRID_WIDTH];   // current window cells

    int baseRow;            // world row index corresponding to grid[0]
    int topRow;             // highest generated world row so far
    Vector3 playerSpawn;    // player spawn point.
} LevelGenerator;

void LevelGenerator_Init(LevelGenerator *lg, Platform *prototype);
void LevelGenerator_Update(LevelGenerator *lg, float playerY);
void LevelGenerator_Draw(const LevelGenerator *lg);
Platform* LevelGenerator_Get(const LevelGenerator *lg, int worldGX, int worldGY);
Vector3 LevelGenerator_GetSpawnPos(const LevelGenerator *lg);

#endif // LEVEL_GENERATOR_H
