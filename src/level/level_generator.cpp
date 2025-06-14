// level/LevelGenerator.cpp
#include "level_generator.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>      // for srand/time

/* ------------------------------------------------------------------ */
/* one-time RNG seed                                                  */
static void SeedRNG() {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(NULL));   // new seed each boot
        seeded = true;
    }
}

/* ------------------------------------------------------------------ */
static inline Vector3 CellToWorld(int gx, int gy) {
    return (Vector3){ gx * CELL_WIDTH, gy * CELL_HEIGHT, 0.0f };
}

/* ---------- per-row random generation ----------------------------- */
static void GenerateRow(LevelGenerator *lg, int worldRow) {
    int count   = 1 + rand() % 3;      // 1-3 isolated platforms
    int placed  = 0;

    while (placed < count) {
        int x = rand() % GRID_WIDTH;

        // avoid neighbours in this worldRow
        int gy = worldRow - lg->baseRow;
        bool blocked = false;
        for (int dx = -1; dx <= 1; ++dx) {
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

/* ---------- chunk generation (24 rows) ---------------------------- */
static void GenerateChunk(LevelGenerator *lg) {
    int startRow = lg->topRow + 1;
    for (int r = 0; r < GRID_HEIGHT; ++r) {
        int worldRow = startRow + r;
        int gy       = worldRow - lg->baseRow;

        memset(lg->grid[gy], 0, sizeof(Platform*) * GRID_WIDTH);
        GenerateRow(lg, worldRow);
    }
    lg->topRow += GRID_HEIGHT;
}

/* ------------------------------------------------------------------ */
void LevelGenerator_Init(LevelGenerator *lg, Platform *prototype) {
    SeedRNG();                         // NEW: ensures random layout per run

    lg->prototype = prototype;
    memset(lg->grid, 0, sizeof(lg->grid));

    lg->baseRow = 0;
    lg->topRow  = GRID_HEIGHT - 1;

    /* ---- fixed 5-platform starter floor (centred) ---------------- */
    int startX = (GRID_WIDTH - START_FLOOR_PLATFORMS) / 2;
    for (int i = 0; i < START_FLOOR_PLATFORMS; ++i) {
        lg->grid[0][startX + i] = prototype;
    }

    lg->playerSpawn = (Vector3){
        (startX + (float) START_FLOOR_PLATFORMS / 2) * CELL_WIDTH,
        1.0f,
        0.0f
    };

    /* ---- generate the rest of the first chunk (rows 1..23) ------- */
    for (int row = 1; row < GRID_HEIGHT; ++row) {
        GenerateRow(lg, row);
    }
}

void LevelGenerator_Update(LevelGenerator *lg, float playerY) {
    int playerRow = (int)(playerY / CELL_HEIGHT);
    int threshold = lg->topRow - GRID_HEIGHT / 2;

    if (playerRow >= threshold) {
        lg->baseRow += GRID_HEIGHT;    // virtual shift
        GenerateChunk(lg);             // new random chunk
    }
}

void LevelGenerator_Draw(const LevelGenerator *lg) {
    for (int gy = 0; gy < GRID_HEIGHT; ++gy) {
        int worldRow = lg->baseRow + gy;
        for (int gx = 0; gx < GRID_WIDTH; ++gx) {
            Platform *p = lg->grid[gy][gx];
            if (p) {
                Platform_Draw(p, CellToWorld(gx, worldRow), 2.0f);
            }
        }
    }
}

/* ---------------------------------------------------------------
   Return the Platform* at a given world-grid coordinate.
   worldGX : 0‥GRID_WIDTH-1  (column index)
   worldGY : absolute row index that keeps increasing upward
   --------------------------------------------------------------- */
Platform* LevelGenerator_Get(const LevelGenerator *lg, int worldGX, int worldGY) {
    if (worldGX < 0 || worldGX >= GRID_WIDTH) return NULL;
    if (worldGY < lg->baseRow)                return NULL;          // below buffer
    int gy = worldGY - lg->baseRow;                                // 0‥GRID_HEIGHT-1
    if (gy >= GRID_HEIGHT)                    return NULL;          // above buffer
    return lg->grid[gy][worldGX];
}

Vector3 LevelGenerator_GetSpawnPos(const LevelGenerator *lg) {
    return lg->playerSpawn;
}
