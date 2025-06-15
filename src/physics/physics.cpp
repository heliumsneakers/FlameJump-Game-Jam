#include "physics.h"
#include "../player/player.h"   // for foot-cube helper

const float GRAVITY = -9.8f;

/* Resolve only if the tiny foot cube touches the platform’s top face
   – player : physics body (+ world position in player->pos)
   – footBox: pre-computed foot cube for this frame
*/
bool ResolvePlatformCollision(Body *player, const BoundingBox *platBox, const BoundingBox *footBox, float restitution, bool *landed) {
    /* 1) feet must intersect the platform’s AABB ------------- */
    if (!CheckCollisionBoxes(*footBox, *platBox)) return false;

    /* 2) must be descending                                   */
    if (player->vel.y >= 0.0f) return false;

    /* 3) vertical penetration depth                           */
    float penY = platBox->max.y - footBox->min.y;   // feet below top?
    if (penY < 0.0f) return false;                  // shouldn’t happen

    /* --- land ---------------------------------------------- */
   player->pos.y += penY + 0.0001f;   // snap exactly onto surface
   player->vel.y = -player->vel.y * restitution;   /*rest -> 0 = stick, 1 = bounce*/ 
    if (landed) *landed = true;

    return true;
}
