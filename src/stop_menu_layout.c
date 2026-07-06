#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
// external (raysan5)
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

enum State {
    STATE_PLAYING,
    STATE_STOP,
    STATE_STOP_CHECKPOINT,
    STATE_EVENT,
    STATE_GAMEOVER
};

enum State gameState = STATE_STOP_CHECKPOINT;


int main()
{
    const int windowWidth = 1200;
    const int windowHeight = 960;

    InitWindow(windowWidth, windowHeight, "Vidas Secas");
    GuiLoadStyle("resources/styles/dark/style_dark.rgs");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
    
    int activeOption = 0;
    int activeSubMenu = -1;

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(BLACK);

            // Stop Button
            if (gameState == STATE_PLAYING){
                int height = 60;
                int width = 200;
                int margin = 60;
                if (GuiButton((Rectangle) {windowWidth - width - margin, windowHeight - height - margin, width, height}, "Stop")) gameState = STATE_STOP;
            }

            // Stop Menu
            bool inCheckpoint = (gameState == STATE_STOP_CHECKPOINT);

            if (gameState == STATE_STOP || inCheckpoint){
                int height = 60;
                int width = 200;
                #define SUBMENU_NONE -1
                #define SUBMENU_BUY   0
                #define SUBMENU_SELL  1
                if (activeSubMenu == SUBMENU_NONE) {
                    const char* options = inCheckpoint? "Party;Car;Supply;Town" : "Party;Car;Supply";  
                    GuiToggleGroup((Rectangle){ 0,  windowHeight - height, width, height }, options, &activeOption);
                    
                    
                    const char* leaveText = inCheckpoint? "Leave" : "Back";  
                    if (GuiButton((Rectangle) {windowWidth - width, windowHeight - height, width, height}, leaveText)) {
                        gameState = STATE_PLAYING;
                    }

                    switch (activeOption)
                    {
                    // depending on the button clicked within the active option,  i want to enter a button specific "submenu", is there a way to do this without sabing a state? more imideate mode
                    case 0 /* Party */:  
                        GuiButton((Rectangle) {40, 40, width, height}, "Rest");
                        break;
                    case 1 /* Car */:  
                        GuiButton((Rectangle) {40, 40, width, height}, "Repair");
                        GuiButton((Rectangle) {40, 120, width, height}, "Upgrade");
                        GuiButton((Rectangle) {40, 200, width, height}, "Set Pace");
                        GuiButton((Rectangle) {40, 280, width, height}, "Road Map");
                        break;
                    case 2 /* Supply */:  
                        GuiButton((Rectangle) {40, 40, width, height}, "Scavange");
                        GuiButton((Rectangle) {40, 120, width, height}, "Trade");
                        GuiButton((Rectangle) {40, 200, width, height}, "Rationing");
                        break;
                    case 3 /* Town */:  
                        if (GuiButton((Rectangle) {40, 40, width, height}, "Buy"))   activeSubMenu = SUBMENU_BUY;
                        if (GuiButton((Rectangle) {40, 120, width, height}, "Sell")) activeSubMenu = SUBMENU_SELL;
                        break;
                    default:
                        break;
                    }
                } else if (activeSubMenu == SUBMENU_BUY)  {
                    if (GuiButton((Rectangle) {windowWidth - width, windowHeight - height, width, height}, "Done")) activeSubMenu = SUBMENU_NONE;
                }
                else if (activeSubMenu == SUBMENU_SELL) {
                    if (GuiButton((Rectangle) {windowWidth - width, windowHeight - height, width, height}, "Done")) activeSubMenu = SUBMENU_NONE;
                } 
                
                
            }   
        EndDrawing();
    }

    CloseWindow();
    return 0;
}