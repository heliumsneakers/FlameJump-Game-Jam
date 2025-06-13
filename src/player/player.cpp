/* player.cpp */
#include "player.h"
#include <stdlib.h>
#include <math.h>

#define Y_EPSILON 1e-6f

// Find the highest Y value among vertices
static float FindBaseY(Mesh *mesh) {
    float maxY = mesh->vertices[1];
    for (int i = 1; i < mesh->vertexCount; i++) {
        float y = mesh->vertices[i*3 + 1];
        if (y > maxY) maxY = y;
    }
    return maxY;
}

// Find all vertex indices close to baseY
static void FindApexIndices(Mesh *mesh, float baseY, int **outIndices, int *outCount) {
    int vc = mesh->vertexCount;
    int *indices = (int*)malloc(vc * sizeof(int)); 
    int count = 0;
    for (int i = 0; i < vc; i++) {
        float y = mesh->vertices[i*3 + 1];
        if (fabsf(y - baseY) < Y_EPSILON) {
            indices[count++] = i;
        }
    }
    *outIndices = indices;
    *outCount = count;
}

void Player_Init(Player *player, const char *objPath, const char *texPath, Vector3 spawnPos) {
    player->model   = LoadModel(objPath);
    player->texture = LoadTexture(texPath);
    player->position = spawnPos;
    player->rotation = 0.0f;
    player->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = player->texture;
    player->mesh = &player->model.meshes[0];
    player->baseY = FindBaseY(player->mesh);
    FindApexIndices(player->mesh, player->baseY, &player->apexIndices, &player->apexCount);
}

void Player_Unload(Player *player) {
    free(player->apexIndices);
    UnloadTexture(player->texture);
    UnloadModel(player->model);
}

void Player_Draw(const Player *player, const Camera *camera) {
    DrawModelEx(player->model,
                player->position,
                (Vector3){0,1,0},
                player->rotation,
                (Vector3){1.5f, 1.5f, 1.5f},
                WHITE);
}
