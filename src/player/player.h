#pragma once
#include "raylib.h"
#include "../physics/physics.h"


#define FOOT_SIZE   0.2f

extern bool onGround;
extern float lightRad;

// Player struct holds model, texture, and animation data
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

// Initialize the player: load model, texture, and prepare apex data
void Player_Init(Player *player, const char *objPath, const char *texPath, Vector3 spawnPos);

void Player_Update(Player *p, Body *playerBody, float dt);

// Unload player resources
void Player_Unload(Player *player);

// Draw the player model using the given camera
void Player_Draw(const Player *player, const Camera *camera);

// Animation helpers: bob apex vertices and rotate model
void Player_IdleAnimation(Player *player, float time);
void Player_UpdateRotation(Player *player, float deltaAngle);
void Player_RefreshJump(void);
BoundingBox Player_GetWorldBBox(const Player *p, Vector3 scale);
BoundingBox Player_GetFootBox(const Player *p, Vector3 scale, float side);
