// level/platform.cpp
#include "platform.h"

void Platform_Init(Platform *p, const char *objPath, const char *texPath) {
    p->model   = LoadModel(objPath);
    p->texture = LoadTexture(texPath);
    // Assign diffuse texture
    p->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = p->texture;
}

void Platform_Draw(const Platform *p, Vector3 position, float scale) {
    DrawModel(p->model, position, scale, WHITE);
}

void Platform_Unload(Platform *p) {
    UnloadTexture(p->texture);
    UnloadModel(p->model);
}
