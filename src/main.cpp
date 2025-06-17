/* main.cpp */
#include "raylib.h"
#include "raymath.h"

#include "player/player.h"
#include "level/platform.h"
#include "level/level_generator.h"
#include "level/light.h"
#include "physics/physics.h"

#if defined(PLATFORM_WEB)
#define ASSET(x) "assets/" x
#else
#define ASSET(x) "../assets/" x
#endif

static void CameraSmoothFollow(Camera* cam, Vector3 playerPos, float dt) {
    /* desired offset from player */
    const float CAM_HEIGHT   =  8.0f;
    const float CAM_DISTANCE = -22.0f;     // along –Z
    const float SMOOTH_SPEED =  0.5f;      // larger = snappier

    Vector3 desiredPos = {
        playerPos.x,
        playerPos.y + CAM_HEIGHT,
        playerPos.z + CAM_DISTANCE
    };

    /* exponential smoothing: pos += (desired - pos) * α */
    float alpha = 1.0f - powf(0.001f, dt * SMOOTH_SPEED);
    cam->position = Vector3Lerp(cam->position, desiredPos, alpha);
    cam->target   = Vector3Lerp(cam->target,   playerPos, alpha);
}

int main(void) {
    // ------------------------------------------------------------------
    // Window & camera
    // ------------------------------------------------------------------
    const int screenW = 1280, screenH = 720;
    const int fbW = 256,  fbH = 192;   // low-res off-screen buffer NDS : 256, 192

    InitWindow(screenW, screenH, "Ignite Jam");

    // ------------------------------------------------------------------
    // Platform prototype  &  level generator
    // ------------------------------------------------------------------
    Platform proto;
    Platform_Init(&proto,
                  ASSET("woodplatform.obj"),
                  ASSET("woodplatform_tex.png"));
    Platform ember;
    Platform_Init(&ember, ASSET("ember.obj"), ASSET("ember.png"));

    LevelGenerator level;
    LevelGenerator_Init(&level, &proto, &ember);

    Physics_SetLevelGenerator(&level);

    Vector3 spawn = LevelGenerator_GetSpawnPos(&level);
    spawn.y = spawn.y + 3.0f;

    Player player;
    Player_Init(&player,
                ASSET("fireguy.obj"),
                ASSET("fireguy_tex.png"),
                spawn);

    Body playerBody = {
        .pos        = spawn,
        .vel        = {0, 0, 0},
        .acc        = { 0, GRAVITY, 0 },
        .mass       = 1.0f,
        .localBBox  = player.localBBox 
    };

    Camera camera{};
    camera.position   = { player.position.x, 8.0f, -18.0f }; 
    camera.target     = {player.position.x, player.position.y, player.position.z};
    camera.up         = { 0.0f, 1.0f,  0.0f };
    camera.fovy       = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE; 

    // ------------------------------------------------------------------
    // Lights!
    // ------------------------------------------------------------------
    Light_Reset();
    int playerLight = Light_Add(
                                (Vector3){player.position.x, player.position.y + .5f, player.position.z}, // Position
                                lightRad, // Radius
                                (Color){255,160,100,180}); // Light Color

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
    DisableCursor();

    // ------------------------------------------------------------------
    while (!WindowShouldClose()) {

        float dt = 0.02f; // Frame rate independent of course! frametime = bleh.

        Player_Update(&player, &playerBody, dt);
        Body_Integrate(&playerBody, dt);
        Physics_Update(dt);

        // Conditional for game over restart
        if (playerBody.pos.y < -5.0f) {
            // re-init level + player + camera
            LevelGenerator_Init(&level, &proto, &ember);
            Physics_SetLevelGenerator(&level);

            spawn = LevelGenerator_GetSpawnPos(&level);
            spawn.y += 3.0f;

            player.position = spawn;
            playerBody.pos  = spawn;
            playerBody.vel  = (Vector3){0,0,0};

            camera.position = (Vector3){ player.position.x, 8.0f, -18.0f };
            camera.target   = spawn;

            Light_Reset();
            playerLight = Light_Add(
                (Vector3){spawn.x, spawn.y + 0.5f, spawn.z},
                lightRad,
                (Color){255,160,100,180}
            );

            continue; // skip rendering this frame
        }

        BoundingBox playerFeet = Player_GetFootBox(&player, {1.5f,1.5f,1.5f}, FOOT_SIZE);
        BoundingBox playerBB = Player_GetWorldBBox(&player, (Vector3){1.5f,1.5f,1.5f});

        int gx = (int)floor(playerBody.pos.x / CELL_WIDTH);
        int gy = (int)floor(playerBody.pos.y / CELL_HEIGHT);

        bool landedThisFrame = false;

        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
            {
                int wx = gx + dx;
                int wy = gy + dy;

                Platform *cell = LevelGenerator_Get(&level, wx, wy);

                if (!cell) continue;

                /* ---------- PLATFORM handling ----------------------------- */
                if (cell == &proto)          /* wooden platform */
                {
                    BoundingBox platBB = Platform_GetWorldBBox(
                        cell,
                        { wx*CELL_WIDTH, wy*CELL_HEIGHT, 0 }, 2.0f);

                    ResolvePlatformCollision(&playerBody,
                                             &platBB,
                                             &playerFeet,
                                             0.01f,
                                             &landedThisFrame);   // <─ set only here
                }
                /* ---------- EMBER handling -------------------------------- */
                else if (cell == &ember)
                {
                    BoundingBox emberBB = Platform_GetWorldBBox(
                        cell,
                        { wx*CELL_WIDTH, wy*CELL_HEIGHT, 0 }, 1.0f);

                    if (CheckCollisionBoxes(playerBB, emberBB))
                    { 
                        LevelGenerator_ClearCell(&level, wx, wy); // remove ember
                        Player_RefreshJump();                     // give stored jump
                    }
                }
            }

        onGround = landedThisFrame;          // ← ember never influences this flag
        player.position = playerBody.pos;   // hand back to render system

        CameraSmoothFollow(&camera, player.position, dt);

        // -- update ----------------------------------------------------
        Player_IdleAnimation(&player, GetTime());

        // Advance level if player climbs past halfway point
        LevelGenerator_Update(&level, player.position.y);
        // Update player emitter light position
        Light_UpdatePos(playerLight, (Vector3){player.position.x, player.position.y + .5f, player.position.z}, lightRad);

        // -- draw ------------------------------------------------------
        BeginTextureMode(rt);
        ClearBackground(BLACK);

        BeginMode3D(camera);
        LevelGenerator_Draw(&level);     // draw all platforms
        Player_Draw(&player, &camera);   // draw player
        Light_DrawAll(camera);
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
    EnableCursor();
    CloseWindow();
    return 0;
}
