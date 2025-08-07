/* main.cpp */
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

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
    const float CAM_HEIGHT   =  8.0f;
    const float CAM_DISTANCE = -22.0f;     // along –Z
    const float SMOOTH_SPEED =  0.5f;      // larger = snappier | smaller = smoother

    Vector3 desiredPos = {
        playerPos.x,
        playerPos.y + CAM_HEIGHT,
        playerPos.z + CAM_DISTANCE
    };

    // exponential smoothing: pos += (desired - pos) * alpha
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

    InitAudioDevice();    
    Music bgm = LoadMusicStream(ASSET("music/turning-page.wav"));
    bgm.looping = true;
    SetMusicPan(bgm, 0.5f);
    SetMusicVolume(bgm, 0.5f);
    PlayMusicStream(bgm);               

    Sound deathSound = LoadSound(ASSET("sounds/death.wav"));
    Sound emberSound = LoadSound(ASSET("sounds/ember.wav"));

    SetSoundVolume(emberSound, 0.7f);

    // ------------------------------------------------------------------
    // Platform prototype  &  level generator
    // ------------------------------------------------------------------
    Platform proto;
    Platform_Init(&proto,
                  ASSET("woodplatform.obj"),
                  ASSET("woodplatform_tex.png"));
    Platform ember;
    Platform_Init(&ember, ASSET("ember.obj"), ASSET("ember.png")); 

    Image whiteImg = GenImageColor(1,1, WHITE);
    Texture2D whiteTex = LoadTextureFromImage(whiteImg);
    SetTextureFilter(whiteTex, TEXTURE_FILTER_BILINEAR);

    Shader fireShader = LoadShader(NULL, "assets/shaders/fire.fs");
    int timeLoc = GetShaderLocation(fireShader, "iTime");
    int resLoc  = GetShaderLocation(fireShader, "iResolution");
    SetShaderValue(fireShader, resLoc,
                   (float[2]){ (float)fbW, (float)fbH },
                   SHADER_UNIFORM_VEC2);

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


    // SCORING
    int score = 0;
    int maxRow = (int)floor(spawn.y / CELL_HEIGHT);  // highest row reached so far
    const int EMBER_SCORE = 50; // flat bonus per ember 

    float lavaY = playerBody.pos.y - 25.0f;

    SetTargetFPS(60);
    DisableCursor();

    // ------------------------------------------------------------------
    while (!WindowShouldClose()) {

        float dt = 0.02f; // Frame rate independent of course! frametime = bleh.
        float shaderTime = (float)GetTime();

        UpdateMusicStream(bgm);

        Player_Update(&player, &playerBody, dt);
        Body_Integrate(&playerBody, dt);
        Physics_Update(dt);

        SetShaderValue(fireShader, timeLoc, &shaderTime,
                       SHADER_UNIFORM_FLOAT);

        // each frame, see if we've climbed into a new grid‐row
        int curRow = (int)floor(playerBody.pos.y / CELL_HEIGHT);
        if (curRow > maxRow) {
            score += (curRow - maxRow) + 4;  // +5 point per row
            maxRow = curRow;
        }
        
        float targetLava = playerBody.pos.y - 25.0f;
        if (targetLava > lavaY) lavaY = targetLava;

        for (int row = level.baseRow; row <= level.topRow; row++) {
            float rowY = row * CELL_HEIGHT;
            if (rowY < lavaY) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    if (LevelGenerator_Get(&level,x,row))
                        LevelGenerator_ClearCell(&level,x,row);
                }
            }
        }

        if (playerBody.pos.y < lavaY) {
            PlaySound(deathSound);
            
            lavaY = playerBody.pos.y - 25.0f;
            // reset score / rows
            score = 0;
            maxRow = (int)floor(spawn.y / CELL_HEIGHT);

            // rebuild level & physics
            LevelGenerator_Init(&level, &proto, &ember);
            Physics_SetLevelGenerator(&level);

            // reset player
            spawn = LevelGenerator_GetSpawnPos(&level);
            spawn.y += 3.0f;
            player.position = spawn;
            playerBody.pos  = spawn;
            playerBody.vel  = (Vector3){0,0,0};

            // reset camera
            camera.position = (Vector3){ player.position.x, 8.0f, -18.0f };
            camera.target   = spawn;

            // reset light
            Light_Reset();
            playerLight = Light_Add(
                (Vector3){spawn.x, spawn.y + .5f, spawn.z},
                lightRad,
                (Color){255,160,100,180}
            );

            // skip rendering to avoid a frame of “dead” player
            continue;
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
                        PlaySound(emberSound);
                        score += EMBER_SCORE;
                    }
                }
            }

        onGround = landedThisFrame;          // ember never influences this flag
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

        BeginMode3D(camera);

        rlDisableDepthTest();            // so they always show
        const int FLAMES_PER_PLATFORM = 12;
        for (int i = 0; i < Physics_GetPendingCount(); i++)
        {
            int gx, gy;
            float tnorm; // ignore normalized timer here, but could fade with it
            Physics_GetPending(i, &gx, &gy, &tnorm);

            // center of the platform in world space:
            Vector3 basePos = {
                gx*CELL_WIDTH + CELL_WIDTH*0.5f,
                gy*CELL_HEIGHT + CELL_HEIGHT - 3.5f,  // a hair above the top
                0
            };

            // for each flame particle
            for (int f = 0; f < FLAMES_PER_PLATFORM; f++)
            {
                // random jitter around center
                float rx = (((GetRandomValue(0,100)/100.0f)-0.5f) * CELL_WIDTH*0.6f);
                float ry =  GetRandomValue(0,100)/100.0f;            // lift
                float rz = (((GetRandomValue(0,100)/100.0f)-0.5f) * CELL_WIDTH*0.6f);

                Vector3 pos = {
                    basePos.x + rx,
                    basePos.y + ry,
                    basePos.z + rz
                };

                // flickering size
                float s = (GetRandomValue(3,8)/10.0f);
                Vector2 size = { s, s };

                // flickering color between red→yellow
                float t = GetRandomValue(0,100)/100.0f;
                Color c = ColorLerp(RED, YELLOW, t);
                c.a = 44;    // semi‐transparent

                DrawBillboardPro(
                    camera,
                    whiteTex,
                    (Rectangle){ 0,0, (float)whiteTex.width, (float)whiteTex.height },
                    pos,
                    (Vector3){0,1,0},              // up-vector
                    size,
                    (Vector2){ size.x*0.5f, size.y*0.5f }, // origin
                    0.0f,                          // rotation
                    c                               // tint
                );
            }
        }
        rlEnableDepthTest();

        EndMode3D();

        DrawText(TextFormat("Score: %d", score), 10, 10, 30, WHITE);

        EndDrawing();
    }
    // ------------------------------------------------------------------

    UnloadMusicStream(bgm);
    CloseAudioDevice();
    UnloadRenderTexture(rt);
    UnloadImage(whiteImg);
    Platform_Unload(&proto);
    Player_Unload(&player);
    EnableCursor();
    CloseWindow();
    return 0;
}
