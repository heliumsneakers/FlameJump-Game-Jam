#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "../physics/physics.h"
#include <stdlib.h>
#include <math.h>

#define Y_EPSILON 1e-6f
#define MOVE_SPEED   6.0f      // units / s
#define JUMP_FORCE  12.0f

bool onGround = false;

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

void Player_Update(Player *p, Body *playerBody, float dt)
{
    /* --- horizontal input ----------------------------------------- */
    float h = 0.0f;
    if (IsKeyDown(KEY_A)) h += 1.0f;
    if (IsKeyDown(KEY_D)) h -= 1.0f;
    playerBody->vel.x = h * MOVE_SPEED;
    
    if(IsKeyDown(KEY_W)) playerBody->vel.y = 2.0f * MOVE_SPEED;

    /* --- jump ------------------------------------------------------ */
    if (onGround && IsKeyPressed(KEY_SPACE)) {
        playerBody->vel.y = JUMP_FORCE;
        onGround   = false;
    }

    /* --- gravity --------------------------------------------------- */
    playerBody->vel.y += GRAVITY * dt;

    /* --- integrate ------------------------------------------------- */
    p->position.x += playerBody->vel.x * dt;
    p->position.y += playerBody->vel.y * dt;

    /* --- simple ground clamp (y=1.0) ------------------------------ */
    if (p->position.y < 1.0f) {
        p->position.y = 1.0f;
        playerBody->vel.y    = 0.0f;
        onGround      = true;
    }
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
    // DrawBoundingBox(Player_GetWorldBBox(p, scale), RED);
}

// ------------------------------------------------ bbox helper
BoundingBox Player_GetWorldBBox(const Player *p, Vector3 scale) {
    BoundingBox bb = p->localBBox;

    // scale
    bb.min = Vector3Scale(bb.min, scale.x);   // uniform scale
    bb.max = Vector3Scale(bb.max, scale.x);

    // translate
    bb.min = Vector3Add(bb.min, p->position);
    bb.max = Vector3Add(bb.max, p->position);
    return bb;
}
