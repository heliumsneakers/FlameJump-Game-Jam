// level/platform.h
#ifndef PLATFORM_H
#define PLATFORM_H

#include "raylib.h"

// Simple platform: holds a model and its diffuse texture
typedef struct Platform {
    Model      model;
    Texture2D  texture;
} Platform;

// Load the platform model and texture
void Platform_Init(Platform *p, const char *objPath, const char *texPath);
// Unload resources
void Platform_Unload(Platform *p);
// Draw the platform at given position and scale
void Platform_Draw(const Platform *p, Vector3 position, float scale);

#endif // PLATFORM_H
