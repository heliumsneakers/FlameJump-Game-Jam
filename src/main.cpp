/* main.cpp */
#include "raylib.h"
#include "raymath.h"
#include "player/player.h"

#if defined(PLATFORM_WEB)
    #define ASSET(x) "assets/" x
#else
    #define ASSET(x) "../assets/" x
#endif

int main(void)
{
    const int screenW = 1920, screenH = 1080;

    // PICO8 render res : 128x128
    // Nintendo DS res : 256x192

    const int fbW = 256, fbH = 192;

    InitWindow(screenW, screenH, "Ignite Jam");

    // Camera setup
    Camera camera = { 0 };
    camera.position   = (Vector3){ 4.0f, 2.0f, 4.0f };
    camera.target     = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize player
    Player player;
    Player_Init(&player, ASSET("fireguy.obj"), ASSET("fireguy_tex.png"), (Vector3){0,0,0});

    Model plat = LoadModel(ASSET("woodplatform.obj"));
    Texture2D platTex = LoadTexture(ASSET("woodplatform_tex.png"));
    plat.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = platTex;

    // Render texture for low-res
    RenderTexture2D rt = LoadRenderTexture(fbW, fbH);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_POINT);

    // Compute destination rectangle
    float scale = fminf((float)screenW / fbW, (float)screenH / fbH);
    Rectangle dest = {
        (screenW - fbW * scale) * 0.5f,
        (screenH - fbH * scale) * 0.5f,
        fbW * scale,
        fbH * scale
    };
    Rectangle src = { 0, 0, (float)fbW, -(float)fbH };

    SetTargetFPS(60);

    // Main loop
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Player_IdleAnimation(&player, GetTime());
        // Player_UpdateRotation(&player, 45.0f * dt);
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Draw to render texture
        BeginTextureMode(rt);
            ClearBackground(RAYWHITE);
            Player_Draw(&player, &camera);
            DrawModel(plat, (Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, BLACK);
            DrawModelWires(plat, (Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, RED);
        EndTextureMode();

        // Draw to screen
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(rt.texture, src, dest, (Vector2){0}, 0, WHITE);
        EndDrawing();
    }

    // Cleanup
    UnloadRenderTexture(rt);
    Player_Unload(&player);
    CloseWindow();
    return 0;
}
