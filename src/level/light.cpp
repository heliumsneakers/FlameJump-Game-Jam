#include "light.h"
#include "raylib.h"
#include "raymath.h"

Light      gLights[MAX_LIGHTS];
uint32_t   gLightCount = 0;

/* 256×256 white radial sprite generated once */
static RenderTexture2D glowRT;
static bool glowReady = false;

/* ---------- helpers ------------------------------------------- */
static void EnsureGlowTexture(void)
{
    if (glowReady) return;

    glowRT = LoadRenderTexture(256, 256);
    BeginTextureMode(glowRT);
        ClearBackground(BLANK);
        for (int r = 128; r > 0; --r) {
            float a = 255.0f * ((128-r) / 128.0f);
            DrawCircle(128, 128, (float)r, (Color){255,255,255,(unsigned char)a});
        }
    EndTextureMode();
    glowReady = true;
}

/* ---------- API ----------------------------------------------- */
void Light_Reset(void)
{
    gLightCount = 0;
}

int Light_Add(Vector3 pos, float radius, Color tint)
{
    if (gLightCount >= MAX_LIGHTS) return -1;
    EnsureGlowTexture();

    int id = (int)gLightCount++;
    gLights[id] = (Light){ true, pos, radius, tint };
    return id;
}

void Light_UpdatePos(int id, Vector3 newPos, float newRadius)
{
    if (id < 0 || id >= (int)gLightCount) return;
    gLights[id].pos = newPos;
    gLights[id].radius = newRadius;
}

void Light_DrawAll(Camera cam)
{
    EnsureGlowTexture();
    BeginMode3D(cam);

    Rectangle src = { 0, 0,
        (float)glowRT.texture.width,
        (float)glowRT.texture.height };     /* normal, not flipped */

    for (uint32_t i = 0; i < gLightCount; ++i)
    {
        if (!gLights[i].enabled) continue;

        Vector3  pos  = gLights[i].pos;
        Vector2  size = { gLights[i].radius, gLights[i].radius };

        /* simple upright billboard */
        DrawBillboardRec(cam,
                         glowRT.texture,
                         src,
                         pos,
                         size,
                         gLights[i].color);
    }

    EndMode3D();
}
