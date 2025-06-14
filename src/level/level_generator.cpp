// level/LevelGenerator.cpp
#include "level_generator.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------ */
static void SeedRNG()
{
    static bool seeded = false;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = true; }
}

static inline int RowToBuf(const LevelGenerator *lg, int worldRow)
{
    int rel = worldRow - lg->baseRow;                 // 0…47
    return (lg->headBuf + rel) % BUFFER_ROWS;         // physical slot
}

static inline Vector3 CellToWorld(int gx, int gy)
{
    return (Vector3){ gx * CELL_WIDTH, gy * CELL_HEIGHT, 0.0f };
}

/* -------- generate one row --------------------------------------- */
static void GenerateRow(LevelGenerator *lg, int worldRow)
{
    int gy = RowToBuf(lg, worldRow);
    memset(lg->grid[gy], 0, sizeof(Platform*) * GRID_WIDTH);

    int count = 1 + rand() % 3;
    for (int placed = 0; placed < count; )
    {
        int x  = rand() % GRID_WIDTH;
        bool ok = true;
        for (int dx=-1; dx<=1 && ok; ++dx)
            if (x+dx>=0 && x+dx<GRID_WIDTH &&
                lg->grid[gy][x+dx]) ok=false;

        if (ok) { lg->grid[gy][x] = lg->prototype; placed++; }
    }
}

/* -------- generate 24-row chunk ---------------------------------- */
static void GenerateChunk(LevelGenerator *lg)
{
    int start = lg->topRow + 1;
    for (int r = 0; r < GRID_HEIGHT; ++r)
        GenerateRow(lg, start + r);

    lg->topRow += GRID_HEIGHT;
}

/* ------------------------------------------------------------------ */
void LevelGenerator_Init(LevelGenerator *lg, Platform *proto)
{
    SeedRNG();
    lg->prototype = proto;
    memset(lg->grid, 0, sizeof(lg->grid));

    lg->baseRow = 0;
    lg->topRow  = BUFFER_ROWS - 1;   // first two chunks will fit
    lg->headBuf = 0;

    /* starter strip (row 0) */
    int mid = (GRID_WIDTH - START_FLOOR_PLATFORMS)/2;
    for (int i=0;i<START_FLOOR_PLATFORMS;++i)
        lg->grid[0][mid+i] = proto;

    lg->playerSpawn = (Vector3){
        (mid + START_FLOOR_PLATFORMS/2)*CELL_WIDTH, 1.0f, 0.0f };

    /* fill first chunk rows 1..23 */
    for (int row=1; row<GRID_HEIGHT; ++row) GenerateRow(lg,row);

    /* second chunk rows 24..47 */
    for (int row=GRID_HEIGHT; row<BUFFER_ROWS; ++row) GenerateRow(lg,row);
}

/* -------- runtime update ----------------------------------------- */
void LevelGenerator_Update(LevelGenerator *lg, float playerY)
{
    int playerRow = (int)(playerY / CELL_HEIGHT);

    /* generate new chunk when player enters upper half of second chunk */
    int trigger = lg->baseRow + GRID_HEIGHT + GRID_HEIGHT/2;
    if (playerRow >= trigger)
    {
        /* add third chunk on top */
        GenerateChunk(lg);

        /* drop bottom chunk by advancing baseRow */
        lg->baseRow += GRID_HEIGHT;
        lg->headBuf  = (lg->headBuf + GRID_HEIGHT) % BUFFER_ROWS;
    }
}

/* -------- draw ---------------------------------------------------- */
void LevelGenerator_Draw(const LevelGenerator *lg)
{
    for (int worldRow = lg->baseRow;
         worldRow <= lg->topRow;
         ++worldRow)
    {
        int gy = RowToBuf(lg, worldRow);
        for (int gx = 0; gx < GRID_WIDTH; ++gx)
            if (Platform *p = lg->grid[gy][gx])
                Platform_Draw(p, CellToWorld(gx, worldRow), 2.0f);
    }
}

/* -------- lookup -------------------------------------------------- */
Platform* LevelGenerator_Get(const LevelGenerator *lg,
                             int worldGX, int worldGY)
{
    if (worldGX < 0 || worldGX >= GRID_WIDTH)   return NULL;
    if (worldGY < lg->baseRow || worldGY > lg->topRow) return NULL;

    int gy = RowToBuf(lg, worldGY);
    return lg->grid[gy][worldGX];
}

Vector3 LevelGenerator_GetSpawnPos(const LevelGenerator *lg)
{ return lg->playerSpawn; }
