#include "physics.h"

const float GRAVITY = -9.8f;

bool ResolvePlatformCollision(Body *player,  const BoundingBox *platBox, float restitution, bool *landed) {
    BoundingBox pb = Body_GetWorldBox(player);

    if (!CheckCollisionBoxes(pb, *platBox)) return false;

    // Very cheap separation along Y only (since game is 2.5-D)
    float penY = platBox->max.y - pb.min.y;   // penetration depth
    player->pos.y += penY + 0.001f;           // push up slightly
    player->vel.y = -player->vel.y * restitution; /*rest -> 0 = stick, 1 = bounce*/
    if (landed) *landed = true;             // report landing
    
    return true;
}
