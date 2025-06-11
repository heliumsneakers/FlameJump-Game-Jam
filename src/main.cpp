// model_viewer_rt.cpp
#include "raylib.h"
#include "raymath.h"        // helpers

#if defined(PLATFORM_WEB)
    #define ASSET(x) "assets/" x     // used with --preload-file assets
#else
    #define ASSET(x) "../assets/" x
#endif

int main(void)
{
    const int screenW = 1920, screenH = 1080;
    const int fbW = 256, fbH = 192;       // low-res render target

    InitWindow(screenW, screenH, "Model viewer (240×160 RT → 800×600)");

    /* ---- Camera ---------------------------------------------------- */
    Camera camera = { {0} };
    camera.position   = (Vector3){ 4.0f, 2.0f, 4.0f };
    camera.target     = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    /* ---- Assets ---------------------------------------------------- */
    Model model = LoadModel(ASSET("box.obj"));
    Vector3 pos = { 0 };
    float   angle = 0.0f;

    /* ---- Render-texture ------------------------------------------- */
    RenderTexture2D rt = LoadRenderTexture(fbW, fbH);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_POINT);   // crisp pixels

    /* ---- Dest rectangle that preserves aspect ratio -------------- */
    float scale = fminf((float)screenW / fbW, (float)screenH / fbH);
    Rectangle dest = {
        (screenW - fbW * scale) * 0.5f,     // center X
        (screenH - fbH * scale) * 0.5f,     // center Y
        fbW * scale,
        fbH * scale
    };
    Rectangle src = { 0, 0, (float)fbW, -(float)fbH };   // flipped vertically

    SetTargetFPS(60);

    /* ---- Main loop ------------------------------------------------- */
    while (!WindowShouldClose())
    {
        angle += 45.0f * GetFrameTime();
        if (angle > 360.0f) angle -= 360.0f;
        UpdateCamera(&camera, CAMERA_ORBITAL);

        /* --- Draw into low-res framebuffer ------------------------- */
        BeginTextureMode(rt);
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                DrawModelEx(model, pos,
                            (Vector3){ 0, 1, 0 }, angle,
                            (Vector3){ 5, 5, 5 }, WHITE);
                DrawGrid(20, 1.0f);
            EndMode3D();

            DrawText("Drag mouse: orbit  |  Scroll: zoom", 4, 4, 10, GRAY);
        EndTextureMode();

        /* --- Now draw the RT scaled to the real window ------------- */
        BeginDrawing();
            ClearBackground(BLACK);                 // letterbox bars
            DrawTexturePro(rt.texture, src, dest, (Vector2){0}, 0, WHITE);
        EndDrawing();
    }

    /* ---- Cleanup --------------------------------------------------- */
    UnloadRenderTexture(rt);
    UnloadModel(model);
    CloseWindow();
    return 0;
}
