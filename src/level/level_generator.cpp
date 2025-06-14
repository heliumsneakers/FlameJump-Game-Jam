#include "level_generator.h"
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------
static inline Vector3 CellToWorld(int gx, int gy) {
    return (Vector3){ gx * CELL_WIDTH, gy * CELL_HEIGHT, 0.0f };
}

static void GenerateRow(LevelGenerator *lg, int worldRow) {
    // simple randomness: 1‑3 isolated platforms
    int count = 1 + rand() % 3;
    int placed = 0;
    while (placed < count) {
        int x = rand() % GRID_WIDTH;
        // avoid neighbours in this worldRow
        int gy = worldRow - lg->baseRow;
        bool blocked = false;
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            if (nx >= 0 && nx < GRID_WIDTH && lg->grid[gy][nx]) {
                blocked = true;
                break;
            }
        }
        if (!blocked) {
            lg->grid[gy][x] = lg->prototype;
            placed++;
        }
    }
}

static void GenerateChunk(LevelGenerator *lg) {
    // Generate GRID_HEIGHT new rows above current topRow
    int startRow = lg->topRow + 1;
    for (int r = 0; r < GRID_HEIGHT; r++) {
        int worldRow = startRow + r;
        int gy = worldRow - lg->baseRow;
        // ensure target row is within buffer (it always will be)
        memset(lg->grid[gy], 0, sizeof(Platform*) * GRID_WIDTH);
        GenerateRow(lg, worldRow);
    }
    lg->topRow += GRID_HEIGHT;
}

// -----------------------------------------------------------
void LevelGenerator_Init(LevelGenerator *lg, Platform *prototype) {
    lg->prototype = prototype;
    memset(lg->grid, 0, sizeof(lg->grid));

    lg->baseRow = 0;
    lg->topRow  = GRID_HEIGHT - 1;

    // starter floor (row 0)
    int startX = (GRID_WIDTH - START_FLOOR_PLATFORMS) / 2;
    for (int i = 0; i < START_FLOOR_PLATFORMS; i++) {
        lg->grid[0][startX + i] = prototype;
    }

    lg->playerSpawn = (Vector3) {
        (startX * CELL_WIDTH),     // x
        1.0f,                   // y
        0.0f,                   // z
    };

    // generate remaining rows of first chunk
    for (int row = 1; row < GRID_HEIGHT; row++) {
        GenerateRow(lg, row);
    }
}

void LevelGenerator_Update(LevelGenerator *lg, float playerY) {
    int playerRow = (int)(playerY / CELL_HEIGHT);
    int threshold = lg->topRow - GRID_HEIGHT / 2;

    if (playerRow >= threshold) {
        // shift entire grid down by GRID_HEIGHT rows (free space for new chunk)
        for (int gy = 0; gy < GRID_HEIGHT - GRID_HEIGHT; gy++) {}
        // easier: just advance indices, buffer is already big enough
        lg->baseRow += GRID_HEIGHT;
        GenerateChunk(lg);
    }
}

void LevelGenerator_Draw(const LevelGenerator *lg) {
    for (int gy = 0; gy < GRID_HEIGHT; gy++) {
        int worldRow = lg->baseRow + gy;
        for (int gx = 0; gx < GRID_WIDTH; gx++) {
            Platform *p = lg->grid[gy][gx];
            if (p) {
                Platform_Draw(p, CellToWorld(gx, worldRow), 2.0f);
            }
        }
    }
}

Vector3 LevelGenerator_GetSpawnPos(const LevelGenerator *lg)
{
    return lg->playerSpawn;
}
