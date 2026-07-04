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

int clampInt(int value, int min, int max){
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

typedef struct {
    float timeRemaining;
    float duration;
} Timer;

void setTimer(Timer *timer, float duration){
    timer->duration = duration;
    timer->timeRemaining = duration;
}

void resetTimer(Timer *timer){
    timer->timeRemaining = timer->duration;
}

int updateTimer(Timer *timer, double deltaTime){
    if (timer->timeRemaining <= 0.0) {
        return 1;
    };
    timer->timeRemaining -= deltaTime;
    return 0;
}

enum State {
    STATE_PLAYING,
    STATE_EVENT,
    STATE_GAMEOVER
};

typedef struct {
    char name[16];
    bool dead;
    bool sick;
    int health;
    int energy;
    float velocity;
} Person;


typedef struct {
    int food;
    int ammo;
    int weapon;
    int footwear;
} Inventory;

enum RationSize {
    SMALL  = 1,
    MEDIUM = 2,
    LARGE  = 3,
};

typedef struct {
    int count;
    Person member[4];
    Inventory inventory;
    enum RationSize ration;
} Party;

enum EventType {
    EVENT_MESSAGE,
    EVENT_DETOUR,
};

typedef struct Event Event;

typedef struct {
    char message[128];
    int nextId;
} Option;

struct Event {
    int id;
    char message[256];
    enum EventType type;
    int numOptions;
    Option options[4];
};

Event createEvent(int id, char *message, enum EventType type){
    Event event = {0};
    event.id = id;
    strcpy(event.message, message);
    event.type = type;
    event.numOptions = 0;
    return event;
};

Event events[32] = {0};

void addOption(Event *e, char *message, int nextIndex){
    Option option = {0};
    strcpy(option.message, message);
    option.nextId = nextIndex;
    e->options[e->numOptions] = option;
    e->numOptions++;
}

void triggerEvent(int eventId, int *currentEvent, enum State *gameState, int *hours){
    *currentEvent = eventId;  // set new id
    *gameState = STATE_EVENT; // set gamestate
    
    // se for um detour
    if (events[eventId].type == EVENT_DETOUR) {
        int hoursLost = GetRandomValue(3,8); // sorteia horas pedidas
        *hours += hoursLost; // aplica horas perdidas
        strcpy(events[*currentEvent].message, TextFormat("%s Perdeu %d horas.", events[*currentEvent].message, hoursLost)); //edita mensagem para falar quantas horas perdeu
    }
}

typedef struct {
    int hours;
    float distance;
    enum State gameState;
    int currentEvent;
    Party party;
} GameData;

int main()
{
    // creating event list
    events[0] = createEvent(0, "Uma família de Urubus rodeia no céu.", EVENT_MESSAGE);
    events[1] = createEvent(1, "Muita gente morreu nessa região.", EVENT_MESSAGE);
    events[2] = createEvent(2, "Um rio seco.", EVENT_MESSAGE);
    events[3] = createEvent(3, "Um cachorro mendigo olha estranho para vocês.", EVENT_MESSAGE);
    events[4] = createEvent(4, "Longo dia...", EVENT_MESSAGE);
    events[5] = createEvent(5, "A vontade é de largar tudo e sair correndo.", EVENT_MESSAGE);
    events[6] = createEvent(6, "Passam um esqueleto de boi.", EVENT_MESSAGE);
    events[7] = createEvent(7, "Uma fazenda... Longe demais para pedir água.", EVENT_MESSAGE);
    events[8] = createEvent(8, "Passarinhos piam na distância.", EVENT_MESSAGE);
    // type detour
    events[9] = createEvent(9, "Pegou a estrada errada!", EVENT_DETOUR);
    events[10] = createEvent(10, "Andaram em círculos...", EVENT_DETOUR);
  
    // eventEmptyHouse
    #define EMPTY_HOUSE       11
    #define EMPTY_HOUSE_ENTER 12

    events[EMPTY_HOUSE] = createEvent(EMPTY_HOUSE, "Found empty house", EVENT_MESSAGE);
    addOption(&events[EMPTY_HOUSE], "Enter", EMPTY_HOUSE_ENTER);
    addOption(&events[EMPTY_HOUSE], "Go Away", -1);

    events[EMPTY_HOUSE_ENTER] = createEvent(EMPTY_HOUSE_ENTER, "Nothing inside...", EVENT_MESSAGE);

    // eventFoundStranger
    #define FOUND_STRANGER       13
    #define STRANGER_TALK        14
    #define STRANGER_SHORTCUT    15
    #define STRANGER_WRONG_ROAD  16

    events[FOUND_STRANGER]      = createEvent(FOUND_STRANGER, "Um estranho chama sua atenção", EVENT_MESSAGE);
    addOption(&events[FOUND_STRANGER], "Talk",     STRANGER_TALK);
    addOption(&events[FOUND_STRANGER], "Go Away",  -1);
    
    events[STRANGER_TALK]       = createEvent(STRANGER_TALK, "Estão indo para o litoral?", EVENT_MESSAGE);
    addOption(&events[STRANGER_TALK], "Sim",           STRANGER_SHORTCUT);
    addOption(&events[STRANGER_TALK], "Não responder", -1);

    events[STRANGER_SHORTCUT]   = createEvent(STRANGER_SHORTCUT, "Conheço um atalho!", EVENT_MESSAGE);
    addOption(&events[STRANGER_SHORTCUT], "Seguir",        STRANGER_WRONG_ROAD);
    addOption(&events[STRANGER_SHORTCUT], "Deixar pra lá", -1);
    
    events[STRANGER_WRONG_ROAD] = createEvent(STRANGER_WRONG_ROAD, "Pegaram o caminho errado...", EVENT_DETOUR);

    // setup gameplay data
    bool moving = false;
    enum State gameState = STATE_PLAYING;
    int hours = 0;
    float distance = 0;

    int currentEventId; // id of current event

    // move timer
    Timer moveTimer;
    setTimer(&moveTimer, 0.3);

    
    // setup party
    Party party;
    {
        party.count = 4;
        strcpy(party.member[0].name, "Fabiano");
        party.member[0].velocity = 5;
        strcpy(party.member[1].name, "Vitória");
        party.member[1].velocity = 4;
        strcpy(party.member[2].name, "Mais Novo");
        party.member[2].velocity = 4;
        strcpy(party.member[3].name, "Mais Velho");
        party.member[3].velocity = 6;

        for (int i = 0; i < 4; i++)
        {
            party.member[i].health = 100;
            party.member[i].energy = 100;
            party.member[i].sick = false;
            party.member[i].dead = false;
        }

        // inventory init
        party.inventory.ammo = 10;
        party.inventory.weapon = 1;
        party.inventory.food = 50;
        party.inventory.footwear = 4;
        party.ration = MEDIUM;
        
    }
        
    const int windowWidth = 1200;
    const int windowHeight = 960;

    InitWindow(windowWidth, windowHeight, "Vidas Secas");

    while (!WindowShouldClose())
    {

        // Restart
        if (IsKeyPressed(KEY_R))
        {
            hours = 0;
            distance = 0;
            gameState = STATE_PLAYING;
            for (int i = 0; i < 4; i++)
            {
                party.member[i].health = 100;
                party.member[i].energy = 100;
                party.member[i].sick = false;
                party.member[i].dead = false;
            }
            party.count = 4;
            party.inventory.food = 50;
        }

        if (IsKeyPressed(KEY_F))
        {
            currentEventId = EMPTY_HOUSE;
        }
        if (IsKeyPressed(KEY_G))
        {
            currentEventId = FOUND_STRANGER;
        }

        // Save Game
        if (IsKeyPressed(KEY_S))
        {
            FILE *file = fopen("./saves/save01.vd", "wb");

            if (file == NULL) break;

            GameData data;
            data.hours = hours;
            data.distance = distance;
            data.gameState = gameState;
            data.currentEvent = currentEventId;
            data.party = party;

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
                distance = data.distance;
                gameState = data.gameState;
                currentEventId = data.currentEvent;
                party = data.party;
            } else {
                printf("VS_ERROR: :() Data couldn`t load.");
                fclose(file);
            };
        }

        // Rest
        if (IsKeyPressed(KEY_D)) {
            for (int i = 0; i < 4; i++){
                if (party.member[i].dead) continue;
                party.member[i].health += GetRandomValue(0,3);
                party.member[i].energy += GetRandomValue(2,10);
                party.member[i].health = clampInt(party.member[i].health, 0, 100);
                party.member[i].energy = clampInt(party.member[i].energy, 0, 100);
            }
            hours += 1;
        }
        
        if (moving){
            if (updateTimer(&moveTimer, GetFrameTime()) == 1) {
                // reset timer
                moving = false;
                resetTimer(&moveTimer);


                // simulate step
                float velocityParty = 0;
                int hoursSimulated = 3;

                party.count = 0;
                for (int i = 0; i < 4; i++){

                    if (party.member[i].dead) continue;

                    // eat
                    bool couldEat = party.inventory.food >= party.ration;
                    if (couldEat) {
                        party.inventory.food -= party.ration; // decrement total food
                    } else {
                        party.inventory.food = 0;
                    }; 
                
                    // simulate sickness
                    if (!party.member[i].sick && party.member[i].energy < 70){
                        int roll = GetRandomValue(1,100);
                        int chanceOfSickness = party.member[i].energy < 30 ? 10 : 3;
                        if (roll < chanceOfSickness){
                            party.member[i].sick = true;
                        }
                    } else {
                        int roll = GetRandomValue(0,100);
                        if (roll < 10){
                            party.member[i].sick = false;
                        }
                    }

                    // simulate energy loss
                    party.member[i].energy -= GetRandomValue(3,15);
                    party.member[i].energy = clampInt(party.member[i].energy, 0, 100);

                    // simulate health loss (damage)
                    bool isSick = party.member[i].sick;
                    int damage = GetRandomValue(isSick || !couldEat ? 1 : 0, 5);
                    damage = damage * (isSick ? 2 : 1); // if sick
                    damage = damage * (party.member[i].energy == 0 ? 2 : 1); // if exausted
                    damage = damage * (4 - party.ration); // ration size dependent health
                    damage = damage * (couldEat? 1 : 2);

                    party.member[i].health -= damage;
                    if (party.member[i].health <= 0) {
                        party.member[i].health = 0;
                        party.member[i].dead = true;
                        continue;
                    } // clamp health to zero 0

                    // its aliveee (contando os vivos)
                    party.count++;

                    // soma velocidades para gerar media
                    velocityParty += party.member[i].velocity * ((float) party.member[i].health / (float) 100) * ((float) party.member[i].energy / (float) 100);                        
                }

                velocityParty = party.count > 0 ? (velocityParty / party.count) : 0;
                
                distance += velocityParty * hoursSimulated;
                hours += hoursSimulated;


                // check gameover
                if (party.count == 0)
                {
                    gameState = STATE_GAMEOVER;
                }

                // eventos aleatorios
                bool eventShouldHappen = GetRandomValue(0,100) < 30 ? true : false;
                if (eventShouldHappen) {

                    triggerEvent(GetRandomValue(0,10), &currentEventId, &gameState, &hours);

                }
            }
        }
        
        // Enter
        if(!moving && (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))){

            switch (gameState) {
                
                case STATE_EVENT:
                    if (events[currentEventId].numOptions > 0) {
                        break;
                    }
                    gameState = STATE_PLAYING;
                    
                
                case STATE_PLAYING:
                    moving = true;
                    break;
                
                
                default:
                    break;
            }
        }

        

        /* draw */
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw Gameover
            if (gameState == STATE_GAMEOVER) {
                int size = 30;
                int width = MeasureText("EVERYONE DIED!", size);
                DrawText("EVERYONE DIED!", windowWidth / 2 - width / 2, windowHeight / 2, size, WHITE);
            }

            // Draw Event
            if (gameState == STATE_EVENT) {
                if (events[currentEventId].numOptions == 0) {
                    int size = 30;
                    int width = MeasureText(events[currentEventId].message, size);
                    int x = windowWidth / 2 - width / 2;
                    int y =  windowHeight / 2;
                    DrawRectangleLines(x-10, y-10, width+20, size+20, WHITE);
                    DrawText(events[currentEventId].message, x, y, size, WHITE);
                } else {
                    int size = 30;
                    int width = MeasureText(events[currentEventId].message, size);
                    int x = windowWidth / 2 - width / 2;
                    int y =  windowHeight / 2;
                    DrawRectangleLines(x-10, y-10, width+20, size+20, WHITE);
                    DrawText(events[currentEventId].message, x, y, size, WHITE);

                    for (int i = 0; i < events[currentEventId].numOptions; i++)
                    {
                        if (GuiButton((Rectangle){200 + i*120,400,100,40}, events[currentEventId].options[i].message)) {
                            if (events[currentEventId].options[i].nextId != -1) {
                                triggerEvent(events[currentEventId].options[i].nextId, &currentEventId, &gameState, &hours);
                            } else {
                                gameState = STATE_PLAYING;
                            }
                        }
                    }
            
                }
                
            }

            // Moving?
            {
                int size = 30;
                char *message = moving ? "Moving" : "Enter to continue";
                int width = MeasureText(message, size);
                int x = windowWidth - width  - 30;
                int y =  windowHeight - 50;
                DrawText(message, x, y , size, WHITE);
            }

            // Draw Hours and Distance
            {
                int size = 30;
                DrawText(TextFormat("Horas: %02d", hours), 30, 30, size, WHITE);
                DrawText(TextFormat("Distância: %.2fKm", distance), 30, 60, size, WHITE);
                DrawText(TextFormat("Food: %d", party.inventory.food), 30, 90, size, WHITE);
                DrawText(TextFormat("Alive: %d", party.count), 30, 120, size, WHITE);
            }

            // Draw Number of people alive
            {
                int size = 30;
            }


            // Draw status bar (party health)
            for (int i = 3; i >= 0; i--)
            {
                int size = 30;
                int posY = windowHeight - 50 - ((3-i)*30);
                Color textColor = party.member[i].health == 0 ? RED : WHITE;
                DrawText(party.member[i].name, 30, posY, 30, textColor);
                DrawText(TextFormat("%03d", party.member[i].health), 230, posY, size, textColor);
                if (party.member[i].health == 0){
                    DrawText("(dead)", 310, posY, 30, textColor);
                    
                } else if (party.member[i].sick){
                    DrawText("(sick)", 310, posY, 30, textColor);
                    
                }
                DrawText(TextFormat("energy: %03d", party.member[i].energy), 410, posY, size, textColor);
            }


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
