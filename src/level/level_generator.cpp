// level/LevelGenerator.cpp
#include "level_generator.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------ helpers --------------------------------- */
static void SeedRNG()
{
    static bool seeded = false;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = true; }
}

static inline int RowToBuf(const LevelGenerator *lg, int worldRow)
{
    int rel = worldRow - lg->baseRow;                 // 0‥BUFFER_ROWS-1
    return (lg->headBuf + rel) % BUFFER_ROWS;         // circular slot
}

static inline Vector3 CellToWorld(int gx, int gy)
{
    return (Vector3){ gx * CELL_WIDTH, gy * CELL_HEIGHT, 0 };
}

/* ------------ tiny util: 3×3 isolation test ----------------------- */
static bool IsCellIsolated(const LevelGenerator *lg,
                           int worldRow, int gx)
{
    for (int ny = -1; ny <= 1; ++ny)
    {
        int wy = worldRow + ny;
        if (wy < lg->baseRow || wy > lg->topRow) continue;
        int by = RowToBuf(lg, wy);

        for (int nx = -1; nx <= 1; ++nx)
        {
            int wx = gx + nx;
            if (wx < 0 || wx >= GRID_WIDTH) continue;
            if (lg->grid[by][wx]) return false;   // neighbour occupied
        }
    }
    return true;
}

/* ------------------------ row generation -------------------------- */
static void GenerateRow(LevelGenerator *lg, int worldRow)
{
    int gy = RowToBuf(lg, worldRow);
    memset(lg->grid[gy], 0, sizeof(Platform*) * GRID_WIDTH);

    /* ---------- Rule A : 1-3 platforms ---------------------------- */
    int count = 1 + rand() % 3;

    if (count == 3)                                    /* contiguous 3 */
    {
        int start = rand() % (GRID_WIDTH - 2);
        for (int i = 0; i < 3; ++i) lg->grid[gy][start+i] = lg->prototype;
    }
    else                                               /* isolated 1–2 */
    {
        int placed = 0;
        while (placed < count)
        {
            int x = rand() % GRID_WIDTH;
            if (lg->grid[gy][x]) continue;

            bool gap = true;
            for (int dx=-1; dx<=1 && gap; ++dx)
                if (x+dx>=0 && x+dx<GRID_WIDTH && lg->grid[gy][x+dx])
                    gap = false;

            if (gap) { lg->grid[gy][x] = lg->prototype; ++placed; }
        }
    }

    /* ---------- Rule B : at most ONE ember every 4 rows ----------- */
    if ( (worldRow % 4) == 0 )                 /* first row of 4-row band */
    {
        int candidates[GRID_WIDTH];
        int candCnt = 0;

        for (int x = 0; x < GRID_WIDTH; ++x)
            if (!lg->grid[gy][x] && IsCellIsolated(lg, worldRow, x))
                candidates[candCnt++] = x;

        if (candCnt > 0)                       /* pick exactly one */
        {
            int pick = candidates[ rand() % candCnt ];
            lg->grid[gy][pick] = lg->emberProto;
        }
    }
}

/* ------------------------ chunk generation ------------------------ */
static void GenerateChunk(LevelGenerator *lg)
{
    int start = lg->topRow + 1;
    for (int r = 0; r < GRID_HEIGHT; ++r)
        GenerateRow(lg, start + r);

    lg->topRow += GRID_HEIGHT;
}

/* ------------------------ public API ------------------------------ */
void LevelGenerator_Init(LevelGenerator *lg,
                         Platform *platformProto,
                         Platform *emberProto)
{
    SeedRNG();

    lg->prototype  = platformProto;
    lg->emberProto = emberProto;
    memset(lg->grid, 0, sizeof(lg->grid));

    lg->baseRow = 0;
    lg->headBuf = 0;
    lg->topRow  = BUFFER_ROWS - 1;            // two chunks cached

    /* starter strip ------------------------------------------------ */
    int mid = (GRID_WIDTH - START_FLOOR_PLATFORMS) / 2;
    for (int i = 0; i < START_FLOOR_PLATFORMS; ++i)
        lg->grid[0][mid + i] = platformProto;

    lg->playerSpawn =
        (Vector3){ (mid + START_FLOOR_PLATFORMS/2.0f) * CELL_WIDTH,
                   1.0f, 0.0f };

    /* fill initial two chunks ------------------------------------- */
    for (int row = 1; row < BUFFER_ROWS; ++row)
        GenerateRow(lg, row);
}

void LevelGenerator_Update(LevelGenerator *lg, float playerY)
{
    int playerRow = (int)(playerY / CELL_HEIGHT);
    int trigger   = lg->baseRow + GRID_HEIGHT + GRID_HEIGHT/2;

    if (playerRow >= trigger)
    {
        GenerateChunk(lg);                       // append 24 new rows
        lg->baseRow += GRID_HEIGHT;             // slide window up
        lg->headBuf  = (lg->headBuf + GRID_HEIGHT) % BUFFER_ROWS;
    }
}

void LevelGenerator_Draw(const LevelGenerator *lg)
{
    for (int worldRow = lg->baseRow; worldRow <= lg->topRow; ++worldRow)
    {
        int gy = RowToBuf(lg, worldRow);
        for (int gx = 0; gx < GRID_WIDTH; ++gx)
        {
            Platform *p = lg->grid[gy][gx];
            if (!p) continue;

            float scale = (p == lg->prototype) ? 2.0f : 1.0f;
            Platform_Draw(p, CellToWorld(gx, worldRow), scale);
        }
    }
}

Platform* LevelGenerator_Get(const LevelGenerator *lg,
                             int worldGX, int worldGY)
{
    if (worldGX < 0 || worldGX >= GRID_WIDTH)           return NULL;
    if (worldGY < lg->baseRow || worldGY > lg->topRow)  return NULL;
    return lg->grid[ RowToBuf(lg, worldGY) ][ worldGX ];
}

Vector3 LevelGenerator_GetSpawnPos(const LevelGenerator *lg)
{
    return lg->playerSpawn;
}
