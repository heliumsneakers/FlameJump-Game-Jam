#ifndef PLATFORM_H
#define PLATFORM_H

#include "raylib.h"

typedef struct Platform {
    Model       model;
    Texture2D   texture;
    BoundingBox localBBox;   // axis-aligned bounding box in model space
} Platform;

// init / unload
void Platform_Init(Platform *p, const char *objPath, const char *texPath);
void Platform_Unload(Platform *p);

// draw
void Platform_Draw(const Platform *p, Vector3 position, float scale);

// world-space bounding box helper
BoundingBox Platform_GetWorldBBox(const Platform *p, Vector3 position, float scale);

#endif // PLATFORM_H
