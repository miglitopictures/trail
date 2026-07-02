#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// external (raysan5)
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


enum State {
    SETUP,
    PLAYING,
    GAMEOVER
};

typedef struct {
    char name[16];

    bool sick;
    int health;

    float velocity;
} Person;



typedef struct {
    int hours;
    enum State gameState;
    Person party[4];
} GameData;

int main()
{
    enum State gameState = PLAYING;
    int hours = 0;
    float distance = 0;
    
    Person party[4];
    
    // setup party
    {

        strcpy(party[0].name, "Fabiano");
        party[0].velocity = 5;
        strcpy(party[1].name, "Vitória");
        party[1].velocity = 4;
        strcpy(party[2].name, "Mais Novo");
        party[2].velocity = 4;
        strcpy(party[3].name, "Mais Velho");
        party[3].velocity = 6;

        for (int i = 0; i < 4; i++)
        {
            party[i].health = 100;
            party[i].sick = false;
        }
        
    }
    
    
    
    const int windowWidth = 1200;
    const int windowHeight = 960;

    InitWindow(windowWidth, windowHeight, "Vidas Secas");

    while (!WindowShouldClose())
    {
        /* update */

        // Save Game
        if (IsKeyPressed(KEY_S))
        {
            FILE *file = fopen("./saves/save01.vd", "wb");

            if (file == NULL) break;

            GameData data;
            data.hours = hours;
            data.gameState = gameState;
            for (int i = 0; i < 4; i++) { data.party[i] = party[i]; }

            fwrite(&data, sizeof(GameData), 1, file);
            fclose(file);
        }

        // Load Game
        if (IsKeyPressed(KEY_L))
        {
            FILE *file = fopen("./saves/save01.vd", "rb");
            if (file == NULL) break;

            GameData data;

            if (fread(&data, sizeof(GameData), 1, file) == 1){
                printf("VS_STATUS: :) Data loaded sucessfully!");
                fclose(file);

                hours = data.hours;
                gameState = data.gameState;
                for (int i = 0; i < 4; i++) { party[i] = data.party[i]; }
            } else {
                printf("VS_ERROR: :() Data couldn`t load.");
                fclose(file);
            };
        }
        

        if (IsKeyPressed(KEY_ENTER) && gameState == PLAYING) {



            // simular

            float velocityParty = 0;

            for (int i = 0; i < 4; i++){

                // handle sickness
                if (!party[i].sick){
                    int roll = GetRandomValue(1,100);
                    if (roll < 10){
                        party[i].sick = true;
                    }
                } else {
                    int roll = GetRandomValue(0,100);
                    if (roll < 50){
                        party[i].sick = false;
                    }
                }


                // handle events (eventualmente)

                // apply damage
                int damage = GetRandomValue(5,10) * (party[i].sick ? 2 : 1);
                party[i].health -= damage;

                if (party[i].health < 0) {party[i].health = 0;} // clamp health to zero 0

                velocityParty += party[i].velocity * ((float) party[i].health / (float) 100);
                        
            }

            velocityParty = velocityParty / 4;
            distance += velocityParty;
            hours++;


            // check health sum for gameover
            int healthSum = 0;
            for (int i = 0; i < 4; i++){
                healthSum += party[i].health;
            }
            if (healthSum == 0)
            {
                gameState = GAMEOVER;
            }

        }

        

        /* draw */
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw Gameover
            if (gameState == GAMEOVER) {
                int size = 30;
                int width = MeasureText("EVERYONE DIED!", size);
                DrawText("EVERYONE DIED!", windowWidth / 2 - width / 2, windowHeight / 2, size, WHITE);
            }

            // Draw Hours and Distance
            {
                int size = 30;
                DrawText(TextFormat("Horas: %02d", hours), 30, 30, size, WHITE);
                DrawText(TextFormat("Distância: %.2fKm", distance), 30, 60, size, WHITE);
            }


            // Draw status bar (party health)
            for (int i = 3; i >= 0; i--)
            {
                int size = 30;
                int posY = windowHeight - 50 - ((3-i)*30);
                Color textColor = party[i].health == 0 ? RED : WHITE;
                DrawText(party[i].name, 30, posY, 30, textColor);
                DrawText(TextFormat("%03d", party[i].health), 230, posY, size, textColor);
                if (party[i].health == 0){
                    DrawText("(dead)", 310, posY, 30, textColor);

                } else if (party[i].sick){
                    DrawText("(sick)", 310, posY, 30, textColor);

                }
            }


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
