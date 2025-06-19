#ifndef LIGHT_H
#define LIGHT_H

#include "raylib.h"
#include <stdint.h>

#define MAX_LIGHTS 32         

typedef struct {
    bool     enabled;
    Vector3  pos;              // world-space position 
    float    radius;           // world units
    Color    color;            // tint (alpha controls strength)
} Light;

extern Light gLights[MAX_LIGHTS];
extern uint32_t gLightCount;

void Light_Reset(void);                    
int  Light_Add(Vector3 pos, float radius, Color tint);
void Light_UpdatePos(int id, Vector3 newPos, float newRadius);
void Light_DrawAll(Camera cam);            
#endif /* LIGHT_H */
