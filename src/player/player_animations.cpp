/* player_animations.cpp */
#include "player.h"
#include <math.h>
#include <stdlib.h>

#define TWO_PI 6.28318530718f

void Player_IdleAnimation(Player *player, float time) {
    Mesh *mesh = player->mesh;

    // Lazy-init base Z positions for all apex vertices
    static float *baseZs = NULL;
    if (baseZs == NULL) {
        baseZs = (float *)malloc(player->apexCount * sizeof(float));
        for (int i = 0; i < player->apexCount; i++) {
            int idx = player->apexIndices[i];
            baseZs[i] = mesh->vertices[idx*3 + 2];
        }
    }

    // Animation parameters
    const float freq = 1.0f;        // cycles per second
    const float ampY = 0.25f;       // vertical amplitude
    const float ampZ = 0.1f;        // back/forward amplitude
    float angle = time * freq * TWO_PI;

    // Compute offsets: back -> down -> forward -> up
    float offsetY = -sinf(angle) * ampY;
    float offsetZ = -cosf(angle) * ampZ;

    // Apply offsets to each apex vertex
    for (int i = 0; i < player->apexCount; i++) {
        int idx = player->apexIndices[i];
        mesh->vertices[idx*3 + 1] = player->baseY + offsetY;
        mesh->vertices[idx*3 + 2] = baseZs[i] + offsetZ;
    }

    // Update vertex buffer on GPU (position buffer index 0)
    UpdateMeshBuffer(*mesh,
                     0,
                     mesh->vertices,
                     player->mesh->vertexCount * 3 * sizeof(float),
                     0);
}

void Player_UpdateRotation(Player *player, float deltaAngle) {
    player->rotation += deltaAngle;
    if (player->rotation >= 360.0f) player->rotation -= 360.0f;
}
