/* main.cpp */
#include "raylib.h"
#include "raymath.h"

#include "player/player.h"
#include "level/platform.h"
#include "level/level_generator.h"

#if defined(PLATFORM_WEB)
    #define ASSET(x) "assets/" x
#else
    #define ASSET(x) "../assets/" x
#endif

int main(void)
{
    // ------------------------------------------------------------------
    // Window & camera
    // ------------------------------------------------------------------
    const int screenW = 1280, screenH = 720;
    const int fbW = 256,  fbH = 192;   // low-res off-screen buffer

    InitWindow(screenW, screenH, "Ignite Jam");

    // ------------------------------------------------------------------
    // Player (always drawn at 0,1,0 — y rises as the game progresses)
    // ------------------------------------------------------------------
    Player player;
    Player_Init(&player,
                ASSET("fireguy.obj"),
                ASSET("fireguy_tex.png"),
                { 0.0f, 1.1f, 0.0f });

    Camera camera{};
    camera.position   = { 0.0f, 8.0f,  -15.0f };
    camera.target     = player.position;
    camera.up         = { 0.0f, 2.0f,  0.0f };
    camera.fovy       = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE; // CAMERA_ORBITAL FOR TESTING.
 
    // ------------------------------------------------------------------
    // Platform prototype  &  level generator
    // ------------------------------------------------------------------
    Platform proto;
    Platform_Init(&proto,
                  ASSET("woodplatform.obj"),
                  ASSET("woodplatform_tex.png"));

    LevelGenerator level;
    LevelGenerator_Init(&level, &proto);

    // ------------------------------------------------------------------
    // Off-screen render target
    // ------------------------------------------------------------------
    RenderTexture2D rt = LoadRenderTexture(fbW, fbH);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_POINT);

    float scale = fminf((float)screenW / fbW, (float)screenH / fbH);
    Rectangle dest = {
        (screenW - fbW * scale) * 0.5f,
        (screenH - fbH * scale) * 0.5f,
        fbW * scale,
        fbH * scale
    };
    Rectangle src = { 0, 0, (float)fbW, -(float)fbH };

    SetTargetFPS(60);

    // ------------------------------------------------------------------
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // -- update ----------------------------------------------------
        Player_IdleAnimation(&player, GetTime());
        UpdateCamera(&camera, CAMERA_PERSPECTIVE);

        // Advance level if player climbs past halfway point
        LevelGenerator_Update(&level, player.position.y);

        // -- draw ------------------------------------------------------
        BeginTextureMode(rt);
            ClearBackground(BLACK);

            BeginMode3D(camera);
                LevelGenerator_Draw(&level);     // draw all platforms
                Player_Draw(&player, &camera);   // draw player
            EndMode3D();

        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(rt.texture, src, dest, { 0, 0 }, 0, WHITE);
        EndDrawing();
    }

    // ------------------------------------------------------------------
    UnloadRenderTexture(rt);
    Platform_Unload(&proto);
    Player_Unload(&player);
    CloseWindow();
    return 0;
}
