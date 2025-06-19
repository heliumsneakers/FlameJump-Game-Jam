#pragma once
#include "raylib.h"
#include "../physics/physics.h"


#define FOOT_SIZE   0.2f

extern bool onGround;
extern float lightRad;

typedef struct Player {
    Model           model;
    Texture2D       texture;
    BoundingBox     localBBox;
    Vector3         position;
    float           rotation;
    Mesh            *mesh;
    int             *apexIndices;  // array of apex vertex indices
    int             apexCount;    // number of apex vertices
    float           baseY;        // original Y of apex vertices
} Player;

void Player_Init(Player *player, const char *objPath, const char *texPath, Vector3 spawnPos);

void Player_Update(Player *p, Body *playerBody, float dt);

void Player_Unload(Player *player);

void Player_Draw(const Player *player, const Camera *camera);

void Player_IdleAnimation(Player *player, float time);
void Player_UpdateRotation(Player *player, float deltaAngle);
void Player_RefreshJump(void);
BoundingBox Player_GetWorldBBox(const Player *p, Vector3 scale);
BoundingBox Player_GetFootBox(const Player *p, Vector3 scale, float side);
