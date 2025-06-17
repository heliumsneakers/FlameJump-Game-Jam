#include "physics.h"
#include "../level/level_generator.h"
#include <math.h>
#include <string.h>

// how many can we “pending‐destroy” at once?
#define MAX_PENDINGS 256

typedef struct {
    bool   active;
    int    gx, gy;    // grid coords
    float  timer;     // seconds until clear
} Pending;

static Pending         sPendings[MAX_PENDINGS];
static LevelGenerator *sLevel = NULL;

const float GRAVITY = -9.8f;

void Physics_SetLevelGenerator(LevelGenerator *lg) {
    sLevel = lg;
    // clear any old entries
    memset(sPendings, 0, sizeof(sPendings));
}

void Physics_Update(float dt) {
    if (!sLevel) return;
    for (int i = 0; i < MAX_PENDINGS; i++) {
        if (!sPendings[i].active) continue;
        sPendings[i].timer -= dt;
        if (sPendings[i].timer <= 0.0f) {
            // time’s up → clear that cell
            LevelGenerator_ClearCell(sLevel,
                                     sPendings[i].gx,
                                     sPendings[i].gy);
            sPendings[i].active = false;
        }
    }
}

bool ResolvePlatformCollision(Body *player,
                              const BoundingBox *platBox,
                              const BoundingBox *footBox,
                              float restitution,
                              bool *landed)
{
    // 1) feet must hit top face
    if (!CheckCollisionBoxes(*footBox, *platBox)) return false;
    // 2) only if descending
    if (player->vel.y >= 0.0f) return false;
    // 3) penetration depth
    float penY = platBox->max.y - footBox->min.y;
    if (penY < 0.0f) return false;

    // — we have a landing —
    player->pos.y   += penY + 0.0001f;
    player->vel.y    = -player->vel.y * restitution;
    if (landed) *landed = true;

    // schedule that grid‐cell to vanish in 3 s
    if (sLevel) {
        // compute the world‐grid coords of this platform
        int worldGX = (int)floor(player->pos.x / CELL_WIDTH);
        int worldGY = (int)floor((platBox->max.y)   // platform’s row
                                / CELL_HEIGHT);

        // only add if not already pending
        bool already = false;
        for (int i = 0; i < MAX_PENDINGS; i++) {
            if (sPendings[i].active &&
                sPendings[i].gx == worldGX &&
                sPendings[i].gy == worldGY)
            {
                already = true;
                break;
            }
        }
        if (!already) {
            for (int i = 0; i < MAX_PENDINGS; i++) {
                if (!sPendings[i].active) {
                    sPendings[i].active = true;
                    sPendings[i].gx     = worldGX;
                    sPendings[i].gy     = worldGY;
                    sPendings[i].timer  = 0.5f;    // three‐second fuse
                    break;
                }
            }
        }
    }

    return true;
}
