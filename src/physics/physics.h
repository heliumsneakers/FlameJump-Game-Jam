// physics.h -------------------------------------------------------------
#pragma once
#include "raylib.h"
#include "raymath.h"

typedef struct Body {
    Vector3 pos;       // centre
    Vector3 vel;
    Vector3 acc;       // (0,-g,0) for gravity
    float    mass;     // 1 for now
    BoundingBox localBBox;   // model-space box
} Body;

extern const float GRAVITY;

// integrate one step
static inline void Body_Integrate(Body *b, float dt) {
    b->vel = Vector3Add(b->vel, Vector3Scale(b->acc, dt));   // v += a·dt
    b->pos = Vector3Add(b->pos, Vector3Scale(b->vel, dt));   // x += v·dt
}

// get world AABB
static inline BoundingBox Body_GetWorldBox(const Body *b) {
    BoundingBox bb = b->localBBox;
    bb.min = Vector3Add(bb.min, b->pos);
    bb.max = Vector3Add(bb.max, b->pos);
    return bb;
}

bool ResolvePlatformCollision (Body *player, const BoundingBox *platBox, float restitution);
