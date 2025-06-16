#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "../physics/physics.h"
#include <stdlib.h>
#include <math.h>

#define Y_EPSILON 1e-6f
#define MOVE_SPEED   6.0f      // units / s
#define JUMP_FORCE  18.0f

bool onGround = false;
bool canJump  = false;
static const float COYOTE_MAX = 0.15f;   // 150 ms grace
static float coyoteTimer = 0.0f;         // persists across frames

// ------------------------------------------------ utility (unchanged)
static float FindBaseY(Mesh *mesh) {
    float maxY = mesh->vertices[1];
    for (int i = 1; i < mesh->vertexCount; ++i) {
        float y = mesh->vertices[i * 3 + 1];
        if (y > maxY) maxY = y;
    }
    return maxY;
}

static void FindApexIndices(Mesh *mesh,
                            float baseY,
                            int **outIdx,
                            int  *outCnt)
{
    int vc = mesh->vertexCount;
    int *idx = (int*)malloc(vc * sizeof(int));
    int cnt  = 0;

    for (int i = 0; i < vc; ++i) {
        float y = mesh->vertices[i * 3 + 1];
        if (fabsf(y - baseY) < Y_EPSILON) idx[cnt++] = i;
    }
    *outIdx = idx;
    *outCnt = cnt;
}

// ------------------------------------------------ init / unload
void Player_Init(Player *p, const char *objPath, const char *texPath, Vector3 spawnPos) {
    p->model     = LoadModel(objPath);
    p->texture   = LoadTexture(texPath);
    p->position  = spawnPos;
    p->rotation  = 0.0f;

    p->model.materials[0]
        .maps[MATERIAL_MAP_DIFFUSE].texture = p->texture;

    p->mesh   = &p->model.meshes[0];
    p->baseY  = FindBaseY(p->mesh);
    FindApexIndices(p->mesh, p->baseY,
                    &p->apexIndices, &p->apexCount);

    p->localBBox = GetModelBoundingBox(p->model);   // NEW
    onGround = false;
}

void Player_RefreshJump(void) {
    canJump = true;
    coyoteTimer = COYOTE_MAX;
}

void Player_Update(Player *p, Body *playerBody, float dt) {
    /* --- horizontal input ----------------------------------------- */
    float h = 0.0f;
    if (IsKeyDown(KEY_A)) h += 1.0f;
    if (IsKeyDown(KEY_D)) h -= 1.0f;
    playerBody->vel.x = h * MOVE_SPEED;
    
    if(IsKeyDown(KEY_W)) playerBody->vel.y = 5.0f * MOVE_SPEED;

    /* ------------ coyote timer update ----------------------------- */
    if (onGround) {
        coyoteTimer = COYOTE_MAX;
        canJump = true;                  // refresh while grounded
    } else {
        coyoteTimer -= dt;               // tick down in the air
    }

    if (IsKeyPressed(KEY_SPACE) && (canJump || coyoteTimer > 0.0f))
    {
        playerBody->vel.y  = JUMP_FORCE;
        onGround     = false;
        canJump      = false;        // consume stored jump
        coyoteTimer  = 0.0f;         // consume grace window
    }

    /* --- gravity --------------------------------------------------- */
    playerBody->vel.y += GRAVITY * dt;

    /* --- integrate ------------------------------------------------- */
    p->position.x += playerBody->vel.x * dt;
    p->position.y += playerBody->vel.y * dt;

}

void Player_Unload(Player *p) {
    free(p->apexIndices);
    UnloadTexture(p->texture);
    UnloadModel(p->model);
}

// ------------------------------------------------ draw
void Player_Draw(const Player *p, const Camera *cam) {
    const Vector3 scale = { 1.5f, 1.5f, 1.5f };
    DrawModelEx(p->model,
                p->position,
                (Vector3){0,1,0},
                p->rotation,
                scale,
                WHITE);

    // Debug visualisation:
    DrawBoundingBox(Player_GetWorldBBox(p, scale), RED);
    DrawBoundingBox(Player_GetFootBox(p, scale, FOOT_SIZE), BLUE);
}

BoundingBox Player_GetWorldBBox(const Player *p, Vector3 scale) {
    BoundingBox bb = p->localBBox;

    // 1) uniform scale
    bb.min = Vector3Scale(bb.min, scale.x);
    bb.max = Vector3Scale(bb.max, scale.x);

    // 2) translate to world space
    bb.min = Vector3Add(bb.min, p->position);
    bb.max = Vector3Add(bb.max, p->position);

    // 3) shrink a bit:  20 % narrower (X & Z)  and 10 % shorter (Y)
    float shrinkX = (bb.max.x - bb.min.x) * 0.20f;   // each side
    float shrinkZ = (bb.max.z - bb.min.z) * 0.20f;
    float shrinkY = (bb.max.y - bb.min.y) * 0.20f;

    bb.min.x += shrinkX;
    bb.max.x -= shrinkX;
    bb.min.z += shrinkZ;
    bb.max.z -= shrinkZ;

    bb.min.y += shrinkY;          // raise the floor a bit
    // (optionally) bb.max.y -= shrinkY;  // lower the head if desired

    return bb;
}

BoundingBox Player_GetFootBox(const Player *p,
                               Vector3 scale,
                               float   side)
{
    /* full‐body world bbox */
    BoundingBox wb = Player_GetWorldBBox(p, scale);

    /* foot centre = middle of XZ, lowest Y */
    float cx = (wb.min.x + wb.max.x) * 0.5f;
    float cz = (wb.min.z + wb.max.z) * 0.5f;
    float cy =  wb.min.y;                      // feet plane

    float h = side * 0.5f;                     // half side

    BoundingBox cube;
    cube.min = (Vector3){ cx - h, cy - h, cz - h };
    cube.max = (Vector3){ cx + h, cy + h, cz + h };
    return cube;
}
