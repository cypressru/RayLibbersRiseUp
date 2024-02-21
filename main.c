//Info so I don't forget

//GAMEPAD_BUTTON_MIDDLE_RIGHT -> triangle / start
//GAMEPAD_BUTTON_RIGHT_FACE_* -> x y a b buttons (UP DOWN LEFT RIGHT orientation)
//GAMEPAD_BUTTON_LEFT_FACE_* -> dpad (UP LEFT RIGHT DOWN)
//GAMEPAD_BUTTON_LEFT_THUMB -> analog stick


// include extra libraries here
#include <raylib.h>
#include <kos.h>
#include <kos/init.h>
#include <dc/sound/sound.h>
#include <dc/sound/sfxmgr.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/maple/vmu.h>
#include <dc/fmath.h>
#include <arch/arch.h>
#include "dc_utils.h"
#include "dc_utils.c"
// -------

//Useful values definitions
#define ATTR_DRMCST_WIDTH 640
#define ATTR_DRMCST_HEIGHT 480
#define PLAYER_LIFES 5
#define BRICKS_LINES 5
#define BRICKS_PER_LINE 20
#define BRICKS_POSITION_Y 50
#define LEFT 0
#define CENTER 128
#define RIGHT 255
#define VMU_SCREEN_HEIGHT 32
#define VMU_SCREEN_WIDTH 48
//-----


//Types and structures Definition
typedef enum GameScreen { RAYLOGO, LOGO, TITLE, GAMEPLAY, ENDING } GameScreen; // Screen management

// Player structure
typedef struct Player{
    Vector2 position;
    Vector2 speed;
    Vector2 size;
    Rectangle bounds;
    int lifes;
} Player;

// Ball structure
typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
} Ball;

// Bricks structure
typedef struct Brick {
    Vector2 position;
    Vector2 size;
    Rectangle bounds;
    int resistance;
    bool active;
} Brick;

// code to reset the bricks on reset or loss
void resetBricks(Brick bricks[BRICKS_LINES][BRICKS_PER_LINE], int screenWidth)
{
    for (int j = 0; j < BRICKS_LINES; j++)
    {
        for (int i = 0; i < BRICKS_PER_LINE; i++)
        {
            bricks[j][i].size = (Vector2){ screenWidth/BRICKS_PER_LINE, 20 };
            bricks[j][i].position = (Vector2){ i*bricks[j][i].size.x, j*bricks[j][i].size.y + BRICKS_POSITION_Y };
            bricks[j][i].bounds = (Rectangle){ bricks[j][i].position.x, bricks[j][i].position.y, bricks[j][i].size.x, bricks[j][i].size.y };
            bricks[j][i].active = true;
        }
    }
}

// check if all bricks have been destroyed
bool allBricksDestroyed(Brick bricks[BRICKS_LINES][BRICKS_PER_LINE]) {
    for (int j = 0; j < BRICKS_LINES; j++) {
        for (int i = 0; i < BRICKS_PER_LINE; i++) {
            if (bricks[j][i].active) {
                return false;
            }
        }
    }
    return true;
}

//create level complete bool to fix a bug when we reset the bricks.
bool levelComplete = false;

// VMU STUFF
static uint8 vmu_lcd_bitmap[192];
static uint8 vmu_lcd_bitmap2[192];
int score = 0;
int highScore = 0;

// Program main entry point
int main(void)
{

    //Initialize Cool Variables and other such things
    const int screenWidth = ATTR_DRMCST_WIDTH;
    const int screenHeight = ATTR_DRMCST_HEIGHT;
    // Window Initialization
    InitWindow(screenWidth, screenHeight, "Attempting to make a game :D" );

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 1.0f, 2.0f, 1.0f }; // Camera position
    camera.up = (Vector3){ 0.0f, 0.5f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                   // Camera mode type
    camera.target = (Vector3){ 0.0f, 0.0};

    Model model = LoadModel("rd/stool.obj");                 // Load model
    Texture2D texture = LoadTexture("rd/stool_lores.png"); // Load model texture
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;            // Set map diffuse texture

    Vector3 position = { 0.0f, 0.0f, 0.0f };                    // Set model position

    BoundingBox bounds = GetMeshBoundingBox(model.meshes[0]);   // Set model bounds


    // Initialize KOS sound system
    snd_init();
    uint8_t volume = 128; // TODO this might fix sound, if not delete



    // load textures
    Texture2D texRayLogo = LoadTexture("rd/rayliblogo.png");
    Texture2D texLogo = LoadTexture("rd/madebycyp.png");

    // load wav files found in romdisk
    sfxhnd_t explosion = snd_sfx_load("/rd/explosion.wav");
    sfxhnd_t bounce = snd_sfx_load("/rd/bounce.wav");
    sfxhnd_t start = snd_sfx_load("/rd/start.wav");


    // Game variables
    int level = 1; // sets level to 1
    int framesCounter = 0;  // Counts frames
    GameScreen screen = RAYLOGO; // Setting the starting screen
    int gameResult = -1;    // 0 = lose, 1 = win, -1 = not defined
    bool gamePaused = false;    // Variable to determine if the game is paused
    int gamepad = 0;
    SetTargetFPS(60);

    // Check defined structs on top
    Player player = { 0 };
    Ball ball = { 0 };
    Brick bricks[BRICKS_LINES][BRICKS_PER_LINE] = { 0 };

    // Initialize player
    player.position = (Vector2){ screenWidth/2, screenHeight*7/8};
    player.speed = (Vector2){8.0f, 0.0f };
    player.size = (Vector2){100, 24 };
    player.lifes = PLAYER_LIFES;

    //Initialize Ball
    ball.radius = 10.0f;
    ball.active = false;
    ball.position = (Vector2){ player.position.x + player.size.x/2, player.position.y - ball.radius*2 };
    ball.speed = (Vector2){ 4.0f, 4.0f };

    //Initialize bricks
    for (int j = 0; j < BRICKS_LINES; j++)
    {
        for (int i = 0; i < BRICKS_PER_LINE; i++)
        {
            bricks[j][i].size = (Vector2){ screenWidth/BRICKS_PER_LINE, 20 };
            bricks[j][i].position = (Vector2){ i*bricks[j][i].size.x, j*bricks[j][i].size.y + BRICKS_POSITION_Y };
            bricks[j][i].bounds = (Rectangle){ bricks[j][i].position.x, bricks[j][i].position.y, bricks[j][i].size.x, bricks[j][i].size.y };
            bricks[j][i].active = true;
        }
    }
    //  VMU STUFF
    maple_device_t * dev;
    dev = maple_enum_type (0, MAPLE_FUNC_LCD);
    load_bmp_vmu(vmu_lcd_bitmap, "/rd/ballbuster1.bmp");
    load_bmp_vmu(vmu_lcd_bitmap2, "/rd/ballbuster2.bmp");
    vmu_draw_lcd (dev, vmu_lcd_bitmap);
    //Initialization over



    while (!WindowShouldClose()) // Detect window close button
    {
        //updates start

        if (IsKeyPressed(KEY_LEFT) && gamepad > 0) gamepad--;
        if (IsKeyPressed(KEY_RIGHT)) gamepad++;

        if (IsGamepadAvailable(gamepad)) {
            if (true)
            {

                switch(screen)
                {


                    case RAYLOGO:
                    {
                        // Update LOGO screen data here!

                        framesCounter++;

                        if (framesCounter > 180)
                        {
                            screen = LOGO;    // Change to LOGO screen after 3 seconds
                            framesCounter = 0;
                        }

                    } break;

                    case LOGO:
                    {
                        // Update LOGO screen data here!

                        framesCounter++;

                        if (framesCounter > 240)
                        {
                            screen = TITLE;    // Change to TITLE screen after 3 seconds
                            framesCounter = 0;
                        }

                    } break;

                    case TITLE:  // TODO - Make a real title screen
                    {
                        // Update TITLE screen data here!
                        UpdateCamera(&camera, CAMERA_PERSPECTIVE); //model stuff





                        if (IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT))
                        {
                            snd_sfx_play_chn(1, start, 170, CENTER);


                            screen = GAMEPLAY;
                        }

                    } break;
                    case GAMEPLAY:
                    {
                        // Update GAMEPLAY screen data here!
                        if (IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT)) gamePaused = !gamePaused;

                                if (!gamePaused)
                                {
                                    //Player movement logic
                                    if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) player.position.x -= player.speed.x;
                                    if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) player.position.x += player.speed.x;

                                    if ((player.position.x) <= 0) player.position.x = 0;
                                    if ((player.position.x + player.size.x) >= screenWidth) player.position.x = screenWidth - player.size.x;

                                    player.bounds = (Rectangle){player.position.x, player.position.y, player.size.x, player.size.y };

                                    //if all bricks are destroyed, go to next level
                                    if (allBricksDestroyed(bricks)) {

                                        ball.position.x = player.position.x + player.size.x/2;
                                        ball.position.y = player.position.y - ball.radius - 1.0f;
                                        ball.speed = (Vector2){ 0, 0 };
                                        ball.active = false;
                                        // TODO add a fancy animation, change color on level change
                                        resetBricks(bricks, screenWidth);
                                        bool levelComplete = true;
                                        level++;

                                    }


                                    if (ball.active)
                                    {
                                        // Ball movement logic
                                        ball.position.x += ball.speed.x;
                                        ball.position.y += ball.speed.y;

                                        //collision logic : ball vs screen limits
                                        if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0)) ball.speed.x *= -1;
                                        if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;

                                        // Collision logic: ball vs player
                                        if (CheckCollisionCircleRec(ball.position, ball.radius, player.bounds))
                                        {
                                            ball.speed.y *= -1;
                                            ball.speed.x = (ball.position.x - (player.position.x + player.size.x/2))/player.size.x*5.0f;
                                            snd_sfx_play_chn(1, bounce, 130, CENTER);
                                        }
                                        // Collision logic: ball vs bricks
                                        for (int j = 0; j <BRICKS_LINES; j++)
                                        {
                                            for (int i = 0; i < BRICKS_PER_LINE; i++)
                                            {
                                                if (bricks[j][i].active && (CheckCollisionCircleRec(ball.position, ball.radius, bricks[j][i].bounds)))
                                                {
                                                    bricks[j][i].active = false;
                                                    ball.speed.y *= -1;
                                                    snd_sfx_play_chn(1, bounce, 130, CENTER);
                                                    score++;

                                                    break;
                                                }
                                            }
                                        }





                                        // Game ending logic
                                        if ((ball.position.y + ball.radius) >= screenHeight)
                                        {
                                            ball.position.x = player.position.x + player.size.x/2;
                                            ball.position.y = player.position.y - ball.radius - 1.0f;
                                            ball.speed = (Vector2){ 0, 0 };
                                            ball.active = false;
                                            snd_sfx_play_chn(1, explosion, 130, CENTER);
                                            if (levelComplete == false)
                                            {
                                                player.lifes--;
                                            }
                                            else
                                            {
                                                levelComplete = false;
                                            }
                                        }
                                        if (player.lifes <0)
                                        {
                                            resetBricks(bricks, screenWidth);
                                            if (score > highScore)
                                            {
                                                highScore = score;
                                            }
                                            screen = ENDING;
                                            int level = 1;
                                            player.lifes = 5;
                                            framesCounter = 0;
                                        }
                                    }
                                    else
                                    {
                                        // reset the balls position
                                        ball.position.x = player.position.x + player.size.x/2;

                                        if (IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
                                        {
                                            //activate ball logic
                                            ball.active = true;
                                            ball.speed = (Vector2){ 0, -5.0f };
                                        }
                                    }
                                }


                         if (!gamePaused)
                         {
                            // TODO: make logic for a real pause menu
                         }


                    } break;
                    case ENDING: //TODO Make logic for ending screen
                    {
                        // Update END screen data here!

                        framesCounter++;

                        // LESSON 03: Inputs management (keyboard, mouse)
                        if (IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
                            score = 0;
                            screen = TITLE;
                            snd_sfx_play_chn(1, start, 170, CENTER);

                        }

                    } break;
                    default: break;
                }
                //updates stop

                //drawing starts

                BeginDrawing();

                ClearBackground(RAYWHITE);

                switch(screen)
                {
                    case RAYLOGO:
                    {
                        // TODO: Fix color!
                        DrawTexture(texRayLogo, screenWidth/2 - texLogo.width/2, screenHeight/2 - texLogo.height/2, WHITE);
                    } break;

                    case LOGO:

                    {
                        DrawTexture(texLogo, screenWidth/2 - texLogo.width/2, screenHeight/2 - texLogo.height/2, WHITE);
                    } break;


                    case TITLE:
                    {
                        ClearBackground(PURPLE);


                        BeginMode3D(camera);

                            DrawModel(model, position, 1.0f, WHITE);        // Draw 3d model with texture

                        EndMode3D();


                        // TODO: Draw TITLE screen here!
                        DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
                        DrawText("PRESS START to JUMP to GAMEPLAY SCREEN", 120, 220, 20, DARKGREEN);

                    } break;

                    case GAMEPLAY:
                    {

                        DrawRectangle(0, 0, screenWidth, screenHeight, PURPLE);
                        // Draw basic shapes
                        DrawRectangle(player.position.x, player.position.y, player.size.x, player.size.y, BLACK); //Draws players bar
                        DrawCircleV(ball.position, ball.radius, MAROON); //Draws ball
                        // Draw bricks
                        for (int j = 0; j < BRICKS_LINES; j++)
                        {
                            for (int i = 0; i < BRICKS_PER_LINE; i++)
                            {
                                if (bricks[j][i].active)
                                {
                                    if ((i + j)%2 ==0) DrawRectangle(bricks[j][i].position.x, bricks[j][i].position.y, bricks[j][i].size.x, bricks[j][i].size.y, GRAY);
                                    else DrawRectangle(bricks[j][i].position.x, bricks[j][i].position.y, bricks[j][i].size.x, bricks[j][i].size.y, GRAY);

                                }

                            }

                        }

                        // Draw GUI: Player lives
                        for (int i = 0; i < player.lifes; i++) DrawRectangle(20 + 40*i, screenHeight - 460, 35, 25, GREEN);
                        // Draw GUI: Level

                        DrawText(TextFormat("Level - %d", level), screenWidth - MeasureText(TextFormat("Level %d", level), 30) - 300, 20, 30, BLACK);
                        // Draw GUI: Score

                        DrawText(TextFormat("Score - %d", score), screenWidth - MeasureText(TextFormat("Score - %d", score), 30) - 80, 20, 30, BLACK);


                        // Draw pause message when start is pressed
                        if (gamePaused) DrawText("Paused!", screenWidth/2 - MeasureText("Paused!", 40)/2, screenHeight/2 + 60, 40, GRAY);

                    } break;

                    case ENDING:
                    {
                        // TODO: Draw ENDING screen here!
                        DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
                        DrawText(TextFormat("Score - %d", score), ((screenWidth / 2) - (MeasureText(TextFormat("Score - %d", score), 30) / 2)) + 25 - 28, (screenHeight / 2) - 40, 30, YELLOW);
                        DrawText(TextFormat("High Score - %d", highScore), (screenWidth / 2) - (MeasureText(TextFormat("High Score - %d", highScore), 30) / 2), (screenHeight / 2) + 10, 30, YELLOW);

                        DrawText("PRESS START to RETURN to TITLE SCREEN", 90, 360, 20, YELLOW);

                    } break;

                    default: break;
                }
                // End of Switch (screen)








            }
            else

            {
                DrawText(TextFormat("GP%d: NOT DETECTED", gamepad), 10, 10, 10, GRAY);

                // DrawTexture(texXboxPad, 0, 0, LIGHTGRAY);
            }
        }
        // Controller check ends
        EndDrawing();
        //drawing ends
    }

    // DE-INITIALIZE

    // UNLOAD RESOURCES HERE IF NEEDED

    CloseWindow();

    return 0;
}



