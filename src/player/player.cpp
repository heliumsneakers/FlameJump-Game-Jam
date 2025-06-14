#include "player.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

#define Y_EPSILON 1e-6f

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
