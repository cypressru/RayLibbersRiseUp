//Info so I don't forget

//GAMEPAD_BUTTON_MIDDLE_RIGHT -> triangle / start
//GAMEPAD_BUTTON_RIGHT_FACE_* -> x y a b buttons (UP DOWN LEFT RIGHT orientation)
//GAMEPAD_BUTTON_LEFT_FACE_* -> dpad (UP LEFT RIGHT DOWN)
//GAMEPAD_BUTTON_LEFT_THUMB -> analog stick


// include extra libraries here
#include <raylib.h>
// -------

//Useful values definitions
#define ATTR_DRMCST_WIDTH 640
#define ATTR_DRMCST_HEIGHT 480
#define PLAYER_LIFES 5
#define BRICKS_LINES 5
#define BRICKS_PER_LINE 20
#define BRICKS_POSITION_Y 50
//-----


//Types and structures Definition
typedef enum GameScreen { LOGO, TITLE, GAMEPLAY, ENDING, RAYLOGO } GameScreen; // Screen management

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




// Program main entry point
int main(void)
{
    //Initialize Cool Variables and other such things
    const int screenWidth = ATTR_DRMCST_WIDTH;
    const int screenHeight = ATTR_DRMCST_HEIGHT;
    SetConfigFlags(FLAG_MSAA_4X_HINT); //SetConfigFlags lets you set special rules in initialization. This currently MSAA at 4x.

    // Window Initialization
    InitWindow(screenWidth, screenHeight, "Attempting to make a game :D" );

    // load textures
    Texture2D texLogo = LoadTexture("rd/rayliblogo.png");


    // Game variables
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
                            screen = LOGO;    // Change to TITLE screen after 3 seconds
                            framesCounter = 0;
                        }

                    } break;
                    case LOGO:
                    {
                        // Update LOGO screen data here!

                        framesCounter++;

                        if (framesCounter > 180)
                        {
                            screen = TITLE;    // Change to TITLE screen after 3 seconds
                            framesCounter = 0;
                        }

                    } break;


                    case TITLE:
                    {
                        // Update TITLE screen data here!

                        framesCounter++;

                        // LESSON 03: Inputs management (keyboard, mouse)
                        if (IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT)) screen = GAMEPLAY;

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

                                    if (ball.active)
                                    {
                                        // Ball movement logic
                                        ball.position.x += ball.speed.x;
                                        ball.position.y += ball.speed.y;

                                        //collision logic : ball vs screen limits
                                        if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0)) ball.speed.x *= -1;
                                        if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;

                                        // TODO: collision detection and resolution

                                        // Collision logic: ball vs player
                                        if (CheckCollisionCircleRec(ball.position, ball.radius, player.bounds))
                                        {
                                            ball.speed.y *= -1;
                                            ball.speed.x = (ball.position.x - (player.position.x + player.size.x/2))/player.size.x*5.0f;
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

                                            player.lifes--;
                                        }
                                        if (player.lifes <0)
                                        {
                                            screen = ENDING;
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


                        // if (!gamePaused)
                        // {
                            // TODO: Gameplay logic
                        // }


                    } break;
                    case ENDING:
                    {
                        // Update END screen data here!

                        framesCounter++;

                        // LESSON 03: Inputs management (keyboard, mouse)
                        if (IsGamepadButtonReleased(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT)) screen = TITLE;

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
                        DrawTexture(texLogo, screenWidth/2 - texLogo.width/2, screenHeight/2 - texLogo.height/2, WHITE);
                    } break;
                    case LOGO:

                    {
                        // Draw LOGO screen here!

                        DrawText("LOGO SCREEN", 20, 20, 40, LIGHTGRAY);
                    }
                    case TITLE:
                    {
                        // TODO: Draw TITLE screen here!
                        DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
                        DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
                        DrawText("PRESS START to JUMP to GAMEPLAY SCREEN", 120, 220, 20, DARKGREEN);

                    } break;
                    case GAMEPLAY:
                    {
                        // TODO: Draw GAMEPLAY screen here!
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
                        for (int i = 0; i < player.lifes; i++) DrawRectangle(20 + 40*i, screenHeight -42, 35, 20, LIGHTGRAY);

                        // Draw pause message when start is pressed
                        if (gamePaused) DrawText("Paused!", screenWidth/2 - MeasureText("Paused!", 40)/2, screenHeight/2 + 60, 40, GRAY);

                    } break;
                    case ENDING:
                    {
                        // TODO: Draw ENDING screen here!
                        DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
                        DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
                        DrawText("PRESS START to RETURN to TITLE SCREEN", 120, 220, 20, DARKBLUE);

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



