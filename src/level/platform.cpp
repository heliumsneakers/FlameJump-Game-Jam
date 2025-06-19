#include "platform.h"
#include "raymath.h"

// -------------------------------------------------- init / unload
void Platform_Init(Platform *p, const char *objPath, const char *texPath) {
    p->model   = LoadModel(objPath);
    p->texture = LoadTexture(texPath);

    p->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = p->texture;

    p->localBBox = GetModelBoundingBox(p->model);
}

void Platform_Unload(Platform *p) {
    UnloadTexture(p->texture);
    UnloadModel(p->model);
}

void Platform_Draw(const Platform *p, Vector3 position, float scale) {
    DrawModel(p->model, position, scale, WHITE);
    // DrawBoundingBox(Platform_GetWorldBBox(p, position, scale), GREEN);
}

BoundingBox Platform_GetWorldBBox(const Platform *p, Vector3 position, float scale) {
    BoundingBox bb = p->localBBox;

    bb.min = Vector3Scale(bb.min, scale);
    bb.max = Vector3Scale(bb.max, scale);

    bb.min = Vector3Add(bb.min, position);
    bb.max = Vector3Add(bb.max, position);

    return bb;
}
