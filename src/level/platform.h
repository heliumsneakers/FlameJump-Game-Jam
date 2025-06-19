#ifndef PLATFORM_H
#define PLATFORM_H

#include "raylib.h"

extern Color platColor;

typedef struct Platform {
    Model       model;
    Texture2D   texture;
    BoundingBox localBBox;   // axis-aligned bounding box in model space
} Platform;

void Platform_Init(Platform *p, const char *objPath, const char *texPath);
void Platform_Unload(Platform *p);

void Platform_Draw(const Platform *p, Vector3 position, float scale);

BoundingBox Platform_GetWorldBBox(const Platform *p, Vector3 position, float scale);

#endif // PLATFORM_H
