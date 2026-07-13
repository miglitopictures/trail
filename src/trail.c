//* Vidas Sequias
// The Assassination of Whale the Dog

// INCLUDES //
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

#define YEAR 1934
#define MON 6
#define DAY 10
#define HOUR 6

const char* startingDateString() {
    return TextFormat("%02d/%02d/%04d | %02d:00", DAY, MON, YEAR, HOUR);
}

struct tm startingDate() {
    struct tm startDate = {0};
    startDate.tm_year = YEAR - 1900;
    startDate.tm_mon = MON;
    startDate.tm_mday = DAY;
    startDate.tm_hour = HOUR;
    return startDate;
}

const char* currentDateString(int totalHours) {
    struct tm startDate = startingDate();
    time_t startSeconds = mktime(&startDate);

    time_t currentSeconds = startSeconds + (totalHours * 3600);
    struct tm *currentDate = localtime(&currentSeconds);

    return TextFormat("%02d/%02d/%04d | %02d:00", 
        currentDate->tm_mday, 
        currentDate->tm_mon + 1, 
        currentDate->tm_year + 1900, 
        currentDate->tm_hour
    );
}

struct tm* currentDate(int totalHours) {
    struct tm startDate = startingDate();
    time_t startSeconds = mktime(&startDate);

    time_t currentSeconds = startSeconds + (totalHours * 3600);

    return localtime(&currentSeconds);
}

int currentMonthInt(int totalHours) {
    struct tm startDate = startingDate();
    time_t startSeconds = mktime(&startDate);
    time_t currentSeconds = startSeconds + (totalHours * 3600);
    return localtime(&currentSeconds)->tm_mon;
}

// SPRITE ANIMATION (spritesheets)
typedef struct {
    Texture2D texture;
    Vector2 frameSize;
    int frameCount;
    int numCols;
    int currentFrame;
} Sprite;

Sprite createSprite(char *texturePath, Vector2 frameSize, int frameCount, int numCols){
    Sprite sprite = { 0 }; 
    sprite.texture = LoadTexture(texturePath);
    sprite.frameSize = frameSize;
    sprite.frameCount = frameCount;
    sprite.numCols = numCols;
    return sprite;
}

void drawSprite(Sprite *sprite, float x, float y, float scale, float rotation, Color tint){
    float offsetX, offsetY;
    offsetX = (sprite->currentFrame % sprite->numCols) * sprite->frameSize.x;
    offsetY = (int)(sprite->currentFrame / sprite->numCols) * sprite->frameSize.y;
    DrawTexturePro(sprite->texture, (Rectangle){offsetX, offsetY, sprite->frameSize.x, sprite->frameSize.y}, 
                                    (Rectangle){x, y, sprite->frameSize.x * scale, sprite->frameSize.y * scale}, 
                                    (Vector2){0, 0}, rotation, tint);

} // reference by https://bedroomcoders.co.uk/posts/185

// TIMER

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
    STATE_START, // TODO: pending implentation!
    STATE_MENU,  // TODO: pending implentation!

    STATE_PLAYING,
    STATE_STOP,
    STATE_STOP_CHECKPOINT, //TODO: talvez mudar o nome para STATE_CHECKPOINT
    STATE_EVENT,

    STATE_GAMEOVER,
    STATE_WIN   // TODO: pending implentation!
};

// cachorro tem menos vida, mas talvez pode ter energia que diminui e quando zera comeca  aperder vida
typedef struct {
    char name[16];
    //// bool foundFood;
    bool dead;
    bool sick;
    int health;
    float velocity;  //TODO: velocidade é necessário?
} Pet;

typedef struct {
    char name[16];
    bool dead;
    bool sick;
    int health;
    int energy;
    float velocity;  //TODO: velocidade é necessário?
} Person;


// TODO: sera que os itens do inventorio tem um peso que afeta o quanto o personagem cansa durante a caminhada? ()
typedef struct {
    int food;
    int ammo;
    int weapon;
    int footwear; // (struct que tem um int desgaste)
} Inventory;

int buy(int *sellerIten, int *buyerIten, int price, int *wallet){
    if (*wallet < price) return 0; // not enough money
    // make trasaction
    *wallet -= price;
    *sellerIten  -= 1;
    *buyerIten += 1;
    return 1;
}

enum RationSize {
    SMALL  = 1,
    MEDIUM = 2,
    LARGE  = 3,
};

typedef struct {
    int count;
    int money;
    
    int faith; // TODO: pending implementation (itens, devoto)

    Person member[4];
    Pet dog;
    Inventory inventory;
    enum RationSize ration;
} Party;

#define PERSON_MAX_HEALTH 300
#define PERSON_MAX_ENERGY 300
#define DOG_MAX_HEALTH 100

void setupParty(Party *party){

    party->count = 4;

    // stats party
    for (int i = 0; i < 4; i++) {
        party->member[i].health = PERSON_MAX_HEALTH;
        party->member[i].energy = PERSON_MAX_ENERGY;
        party->member[i].sick = false;
        party->member[i].dead = false;
    }

    party->dog.health = DOG_MAX_HEALTH;
    party->dog.sick = false;
    party->dog.dead = false;

    // inventory init
    party->inventory.ammo = 10;
    party->inventory.weapon = 1;
    party->inventory.food = 1200;
    party->inventory.footwear = 4;
    party->money = 70;
    party->ration = MEDIUM;
}

// Descansar
// TODO: simular quando comida acabou? 
void restParty(Party *party, int *hours){
    
    // people
    for (int i = 0; i < 4; i++){
        if (party->member[i].dead) continue;

        if (party->inventory.food < party->ration) continue; //LOOK: nao descansa sem comida?
        party->inventory.food -= party->ration;

        party->member[i].health += GetRandomValue(0,3);
        party->member[i].energy += GetRandomValue(2,10);
        party->member[i].health = clampInt(party->member[i].health, 0, PERSON_MAX_HEALTH);
        party->member[i].energy = clampInt(party->member[i].energy, 0, PERSON_MAX_ENERGY);
    }

    // dog
    if (!party->dog.dead){
        party->dog.health += GetRandomValue(1,4);
        party->dog.health = clampInt(party->dog.health, 0, DOG_MAX_HEALTH);
    }

    *hours += 1;
}

void simulateParty(Party *party, float *distance, int *hours){

    // simulate step
    float velocityParty = 0;
    int hoursSimulated = 3;

    if (party->count > 0){ // people simulate

        party->count = 0;
        for (int i = 0; i < 4; i++) {

            if (party->member[i].dead) continue;

            // eat
            bool couldEat = party->inventory.food >= party->ration;
            if (couldEat) {
                party->inventory.food -= party->ration; // decrement total food
            } else {
                party->inventory.food = 0;
            }; 
        
            // simulate sickness
            if (!party->member[i].sick && party->member[i].energy < 70){
                int roll = GetRandomValue(1,100);
                int chanceOfSickness = party->member[i].energy < 30 ? 10 : 3;
                if (roll < chanceOfSickness){
                    party->member[i].sick = true;
                }
            } else {
                int roll = GetRandomValue(0,100);
                if (roll < 10){
                    party->member[i].sick = false;
                }
            }

            // simulate energy loss
            party->member[i].energy -= GetRandomValue(3,15);
            party->member[i].energy = clampInt(party->member[i].energy, 0, 100);

            // simulate health loss (damage)
            bool isSick = party->member[i].sick;

            int damage = GetRandomValue(isSick || !couldEat ? 1 : 0, 5);

            damage = damage * (isSick ? 2 : 1); // if sick
            damage = damage * (party->member[i].energy == 0 ? 2 : 1); // if exausted
            damage = damage * (4 - party->ration); // ration size dependent health
            damage = damage * (couldEat? 1 : 2);

            party->member[i].health -= damage;
            
            if (party->member[i].health <= 0) {
                party->member[i].health = 0;
                party->member[i].dead = true;
                continue;
            } // clamp health to zero 0

            // its aliveee (contando os vivos)
            party->count++;

            // soma velocidades para gerar media
            velocityParty += party->member[i].velocity * ((float) party->member[i].health / (float) PERSON_MAX_HEALTH) * ((float) party->member[i].energy / (float) PERSON_MAX_ENERGY);                        
        }
    }

    

    if (!party->dog.dead){ // simulate dog
        // simulate dog sickness
        if (!party->dog.sick){
            int roll = GetRandomValue(1,100);
            int chanceOfSickness = 50;
            if (roll < chanceOfSickness){
                party->dog.sick = true;
            }
        } else {
            int roll = GetRandomValue(0,100);
            if (roll < 1){
                party->dog.sick = false;
            }
        }

        // simulate health loss (damage)
        bool isSick = party->dog.sick;
        int damage = GetRandomValue(isSick ?  5 : 0, isSick ?  8 : 1);

        party->dog.health -= damage;
        if (party->dog.health <= 0) {
            party->dog.health = 0;
            party->dog.dead = true;
        } // clamp health to zero 0 and kill
    }

    velocityParty = party->count > 0 ? (velocityParty / party->count) : party->dog.velocity;
    
    *distance += velocityParty * hoursSimulated;
    *hours += hoursSimulated;
}

enum Weather {
    VERY_HOT,     // 0
    HOT,          // 1
    COOL,         // 2
    RAINY,        // 3
    VERY_RAINY,   // 4
};

#define WEATHER_SIM_STEP 4

void simulateWeather(enum Weather *weather, int *weatherSimCounter, int currentMonth){

    *weatherSimCounter = (*weatherSimCounter + 1) % WEATHER_SIM_STEP;

    if (*weatherSimCounter != 0) return;

    if (currentMonth > 4 && *weather != VERY_RAINY) // JAN - MAIO
    {
        if (*weather == RAINY)
        {
            if (GetRandomValue(0,100) < 70) { // 70%
                *weather = COOL;
            } else if (GetRandomValue(0,100) < 10){
                *weather = VERY_RAINY;
            }
        }

        if (GetRandomValue(0,100) < 2) { // 1%
            *weather = RAINY;
        } else {
            *weather = GetRandomValue(0, 1); // HOT or VERY_HOT
        }
    }

    else  // MAR - DEZ
    {
        if (*weather == VERY_RAINY)
        {
            if (GetRandomValue(0,100) < 10) { // 10%
                *weather = RAINY;
            }
        }
        else if (*weather == RAINY)
        {
            if (GetRandomValue(0,100) < 70) { // 70%
                *weather = COOL;
            } else if (GetRandomValue(0,100) < 10){
                *weather = VERY_RAINY;
            }
        }
        else
        {
            if (GetRandomValue(0,100) < 10) { // 10%
                *weather = RAINY;
            } else {
                *weather = GetRandomValue(0, 2);
            }
        }
        
    }
}

typedef struct {
    enum Weather weather;
    int weatherSimCounter;
    int hours;
    float distance;

    enum State state;

    unsigned int activeFlags; // EVENT BITMASK

    int currentEvent;
    int checkpointsVisited;
    Party party;
} GameData;

// Event condition flags
#define COND_NONE         (0)       // 00000000 00000000 00000000 00000000
#define COND_LOW_FOOD     (1 << 0)  // 00000000 00000000 00000000 00000010
#define COND_LOW_MONEY    (1 << 1)  // 00000000 00000000 00000000 00000010
#define COND_ALONE        (1 << 2)  // 00000000 00000000 00000000 00000100 ...
#define COND_DOGLESS      (1 << 3)
#define COND_SICK_DOG     (1 << 4)  // evento sacrificio de baleia
#define COND_NIGHT        (1 << 5)
#define COND_VERY_HOT     (1 << 6)
#define COND_RAINY        (1 << 7)
#define COND_VERY_RAINY   (1 << 8)
#define COND_COWLESS      (1 << 9)
#define COND_SLEEPLESS    (1 << 10)
#define COND_GODLESS      (1 << 11)
#define COND_FAITHFUL     (1 << 12)
#define COND_TRAUMA       (1 << 13) // se apanhar da policia ou escolher matar baleia ou se a familia inteira morrer


enum EventType {
    EVENT_MESSAGE,
    EVENT_DETOUR,
};

typedef struct Event Event;

typedef struct {
    char message[128];
    int nextId;
} Option;

typedef struct {
    int hours;
    int money;
    Inventory inventory;
} Effects;


Effects* createEffect(int hours, int money, Inventory inventory){
    Effects *effect = malloc(sizeof(Effects));
    if (!effect) return NULL;
    effect->hours = hours;
    effect->money = money;
    effect->inventory = inventory;
    return effect;
};

struct Event {
    char message[256];
    enum EventType type;

    // distance range for event
    float minDistance;
    float maxDistance;
    // conditional flags
    unsigned int requiredFlags;
    unsigned int forbiddenFlags;

    int weight;

    int numOptions;
    Option options[4];
    Effects *effects;
};

Event createEvent(char *message, enum EventType type, Effects *effects, float minDistance, float maxDistance, unsigned int requiredFlags, unsigned int forbiddenFlags){
    Event event = {0};
    strcpy(event.message, message);
    event.type = type;
    event.effects = effects;
    event.minDistance = minDistance;
    event.maxDistance = maxDistance;
    event.requiredFlags = requiredFlags;
    event.forbiddenFlags = forbiddenFlags;
    event.numOptions = 0;
    return event;
};

#define MAX_EVENTS 128
#define MAX_RANDOM_EVENTS 32 // Indices 0 to 31 are allowed to be rolled randomly because they will be "starting" events

Event events[MAX_EVENTS] = {0};

void addOption(Event *e, char *message, int nextIndex){
    Option option = {0};
    strcpy(option.message, message);
    option.nextId = nextIndex;
    e->options[e->numOptions] = option;
    e->numOptions++;
}

void triggerEvent(int eventId, int *currentEvent, enum State *gameState, int *hours, int *partyMoney, Inventory *partyInventory){
    *currentEvent = eventId;  // set new id
    *gameState = STATE_EVENT; // set gamestate

    if (events[eventId].effects){
        *hours += events[eventId].effects->hours;
        *partyMoney += events[eventId].effects->money;
        partyInventory->ammo += events[eventId].effects->inventory.ammo;
        partyInventory->food += events[eventId].effects->inventory.food;
        partyInventory->footwear += events[eventId].effects->inventory.footwear;
        partyInventory->weapon += events[eventId].effects->inventory.weapon;
        // continue here
    }
}

unsigned int updateActiveFlags(GameData *game){

    unsigned int flags = COND_NONE;
    
    if (game->party.inventory.food < game->party.ration * game->party.count) flags |= COND_LOW_FOOD;

    if (game->party.money < 5)  flags |= COND_LOW_MONEY;

    if (game->party.count == 1) flags |= COND_ALONE;

    if (game->party.dog.dead) {
        flags = flags | COND_DOGLESS;
    } else if (game->party.dog.sick) {
        flags = flags | COND_SICK_DOG;
    }

    int timeOfDay = game->hours % 24;
    if (timeOfDay >= 12) flags |= COND_NIGHT;

    if (game->weather == VERY_HOT) {
        flags |= COND_VERY_HOT;
    } else if (game->weather == RAINY){
        flags |= COND_RAINY;
    } else if (game->weather == VERY_RAINY){
        flags |= COND_VERY_RAINY;
    }

    
    return flags;
}


#define MAX_ACTIVE_POOL 128
int activeEventPool[MAX_ACTIVE_POOL];
int activePoolCount = 0;

void refreshEventPool(float currentDistance, unsigned int activeFlags) {
    activePoolCount = 0;

    for (int i = 0; i < MAX_RANDOM_EVENTS; i++) {
        Event *e = &events[i];

        if (e->message[0] == '\0') continue; // skip empty events

        // conditional checks for flags and distance range
        if (currentDistance < e->minDistance || currentDistance > e->maxDistance) continue;
        if ((activeFlags & e->requiredFlags) != e->requiredFlags) continue;
        if ((activeFlags & e->forbiddenFlags) != 0) continue;

        // Add to active pool
        activeEventPool[activePoolCount] = i;
        activePoolCount++;

        if (activePoolCount >= MAX_ACTIVE_POOL) break;
    }
}

#define MAX_CHECKPOINTS 32

enum CheckpointType {
    TOWN,
    SITE,
};

typedef struct {
    char name[MAX_CHECKPOINTS][32];
    int numTotal;
    enum CheckpointType type[MAX_CHECKPOINTS];
    float distance[MAX_CHECKPOINTS];
    Inventory inventory[MAX_CHECKPOINTS];
} Checkpoints;

void addCheckpoint(Checkpoints *checkpoints, char* name, int distance, enum CheckpointType type, Inventory inventory){
    checkpoints->distance[checkpoints->numTotal] = distance;
    checkpoints->type[checkpoints->numTotal] = type;
    checkpoints->inventory[checkpoints->numTotal] = inventory;
    strcpy(checkpoints->name[checkpoints->numTotal], name);
    checkpoints->numTotal++;
}



int main() {
    //*___SETUP___*//

    bool debug = false;

    SetRandomSeed((unsigned int)time(NULL));
    
    Checkpoints checkpoints = {0}; 
    {  // add checkpoints data
        addCheckpoint(&checkpoints, "Abandoned House",      20, SITE,(Inventory){50, 200,  10, 40});
        addCheckpoint(&checkpoints, "The Cave",             35, SITE,(Inventory){ 0,   0,   0,  2});
        addCheckpoint(&checkpoints, "Petrolina",            40, TOWN,(Inventory){50, 200,  10, 40});
        addCheckpoint(&checkpoints, "Serra Talhada",        70, TOWN,(Inventory){50, 200,  10, 10});
        addCheckpoint(&checkpoints, "Carnabeira da Penha",  90, TOWN,(Inventory){15, 249, 100, 23});
        // (Inventory) {food, ammo, weapon, footwear}
    }


    { // populating the event list
        events[0] = createEvent("Uma família de Urubus rodeia no céu.", EVENT_MESSAGE,NULL,0, 9999, COND_NONE, COND_NONE);

        events[1] = createEvent("Muita gente morreu nessa região.", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[2] = createEvent("Um rio seco.", EVENT_MESSAGE, NULL, 0, 9999, COND_NONE, COND_NONE);
        events[3] = createEvent("Um cachorro selvagem olha estranho para vocês.", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[4] = createEvent("Longo dia...", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[5] = createEvent("A vontade é de largar tudo e sair correndo.", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[6] = createEvent("Passam um esqueleto de boi.", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[7] = createEvent("Uma fazenda... Longe demais para pedir água.", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[8] = createEvent("Passarinhos piam na distância.", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        events[9] = createEvent("Pegou a estrada errada! Perdeu 6 horas.", EVENT_MESSAGE, createEffect(6,0,(Inventory){0}) , 0, 9999, COND_NONE, COND_NONE);
        events[10] = createEvent( "Andaram em círculos... Perdeu 3 horas.", EVENT_MESSAGE, createEffect(3,0,(Inventory){0}) , 0, 9999, COND_NONE, COND_NONE);
        // eventEmptyHouse
        #define EMPTY_HOUSE        11

        events[EMPTY_HOUSE] = createEvent("Found empty house", EVENT_MESSAGE, NULL , 20, 9999, COND_NONE, COND_NONE); // testing ranged distance events
        addOption(&events[EMPTY_HOUSE], "Enter", 32);
        addOption(&events[EMPTY_HOUSE], "Go Away", -1);

        events[32] = createEvent("Nothing inside...", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);

        // eventFoundStranger
        #define FOUND_STRANGER  12

        events[FOUND_STRANGER] = createEvent("Um estranho chama sua atenção", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        addOption(&events[FOUND_STRANGER], "Talk",     33);
        addOption(&events[FOUND_STRANGER], "Go Away",  -1);
        
        events[33] = createEvent("Estão indo para o litoral?", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        addOption(&events[33], "Sim",           34);
        addOption(&events[33], "Não responder", -1);

        events[34]   = createEvent("Conheço um atalho!", EVENT_MESSAGE, NULL , 0, 9999, COND_NONE, COND_NONE);
        addOption(&events[34], "Seguir",         35);
        addOption(&events[34], "Deixar pra lá", -1);
        
        events[35] = createEvent("Pegaram o caminho errado... Perdeu 4 horas.", EVENT_MESSAGE, createEffect(4,0,(Inventory){0}) , 0, 9999, COND_NONE, COND_NONE);

        // eventEmptyHouse
        #define DOG_FOUND_FOOD        13
        events[DOG_FOUND_FOOD] = createEvent("Baleia achou um preá", EVENT_MESSAGE, createEffect(0,0,(Inventory){.ammo=0, .food=4, .footwear=0, .weapon=0}) , 0, 9999, COND_NONE, COND_DOGLESS | COND_SICK_DOG);

    }

    // setup gameplay data
    GameData game = {0};
    game.state = STATE_PLAYING;

    game.weatherSimCounter = 0;
    game.weather = VERY_HOT;

    game.hours = 0;
    game.distance = 0;

    bool moving = false;
    // move timer
    Timer moveTimer;
    setTimer(&moveTimer, 0.7);


    // stop menu globals
    int activeStopMenu  =  0;
    int activeStopSubmenu = -1;
    

    { // init party
        setupParty(&game.party);

        // names party
        strcpy(game.party.member[0].name, "Fabiano");
        game.party.member[0].velocity = 5;
        strcpy(game.party.member[1].name, "Vitória");
        game.party.member[1].velocity = 4;
        strcpy(game.party.member[2].name, "Mais Novo");
        game.party.member[2].velocity = 4;
        strcpy(game.party.member[3].name, "Mais Velho");
        game.party.member[3].velocity = 6;

        strcpy(game.party.dog.name, "Baleia");
        game.party.dog.velocity = 8;
    }
        
    const int windowWidth = 1200;
    const int windowHeight = 960;

    InitWindow(windowWidth, windowHeight, "Vidas Secas");
    GuiLoadStyle("resources/styles/dark/style_dark.rgs");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

    Sprite spriteFamilia = createSprite("resources/sprites/sprite_familia.png",(Vector2){96, 48}, 2, 1);

    while (!WindowShouldClose()) {
        //*___UPDATE___*//

        // Restart
        if (IsKeyPressed(KEY_R)) {
            game.state = STATE_PLAYING;
            game.activeFlags = COND_NONE;
            
            game.hours = 0;
            game.distance = 0;

            game.weatherSimCounter = 0;
            game.weather = VERY_HOT;
            
            game.checkpointsVisited = 0;
            setupParty(&game.party);
        }

        // Toogle Debug
        if (IsKeyPressed(KEY_D)) {
            debug = !debug;
        }

        // next animation frame (delete)
        if (IsKeyPressed(KEY_UP)) {
            spriteFamilia.currentFrame = (spriteFamilia.currentFrame + 1) % spriteFamilia.frameCount;
        }


        // Save Game
        if (IsKeyPressed(KEY_S)) {
            FILE *file = fopen("./saves/save01.vd", "wb");

            if (file == NULL) break;
            fwrite(&game, sizeof(GameData), 1, file);
            fclose(file);
        }

        // Load Game
        if (IsKeyPressed(KEY_L)) {
            FILE *file = fopen("./saves/save01.vd", "rb");
            if (file == NULL) break;

            GameData data;

            if (fread(&data, sizeof(GameData), 1, file) == 1){
                printf("VS_STATUS: :) Data loaded sucessfully!");
                fclose(file);

                game = data;

            } else {
                printf("VS_ERROR: :() Data couldn`t load.");
                fclose(file);
            };
        }
        
        if (moving){
            
            { // animate sprite
                int fps = 6;
                int frame = (int) (GetTime() * fps);
                spriteFamilia.currentFrame = frame % spriteFamilia.frameCount;
            }

            if (updateTimer(&moveTimer, GetFrameTime()) == 1) {
                // reset timer
                moving = false;
                resetTimer(&moveTimer);

                simulateParty(&game.party, &game.distance, &game.hours);

                simulateWeather(&game.weather, &game.weatherSimCounter, currentMonthInt(game.hours));

                game.activeFlags = updateActiveFlags(&game);

                // check gameover
                if (game.party.count == 0 && game.party.dog.dead == true) {
                    game.state = STATE_GAMEOVER;
                }
                // check if reached checkpoint
                else if (game.distance >= checkpoints.distance[game.checkpointsVisited] && game.checkpointsVisited < checkpoints.numTotal) {
                    game.distance = checkpoints.distance[game.checkpointsVisited];
                    game.state = STATE_STOP_CHECKPOINT;
                    activeStopMenu = 3; // town menu index
                } else {
                    // roll eventos aleatorios
                    // game.activeFlags = updateActiveFlags(&game);
                    refreshEventPool(game.distance, game.activeFlags);
                    bool eventShouldHappen = GetRandomValue(0,100) < 30 ? true : false;
                    if (eventShouldHappen) {
                        int poolIndex = GetRandomValue(0, activePoolCount - 1);
                        int chosenEventId = activeEventPool[poolIndex];
                        triggerEvent(chosenEventId, &game.currentEvent, &game.state, &game.hours, &game.party.money, &game.party.inventory);

                    }
                }

                
            }
        }
        
        // Enter
        if (!moving && (IsKeyPressed(KEY_ENTER))) {

            switch (game.state) {

                case STATE_STOP_CHECKPOINT:
                    // checkpoints.numVisited++;
                    // gameState = STATE_PLAYING;
                    // moving = true;
                    break;
                
                case STATE_EVENT:
                    if (events[game.currentEvent].numOptions > 0) {
                        break;
                    }
                    game.state = STATE_PLAYING;
                    moving = true;
                    break;
                    
                
                case STATE_PLAYING:
                    moving = true;
                    break;
                
                
                default:
                    break;
            }
        }       

        //*___DRAW___*//
        BeginDrawing();

            ClearBackground(BLACK);

            // Debug
            if (debug){
                unsigned int allFlags[] =     {COND_LOW_FOOD, COND_LOW_MONEY, COND_ALONE, COND_SICK_DOG, COND_DOGLESS, COND_NIGHT, COND_VERY_HOT, COND_RAINY, COND_VERY_RAINY};
                const char* allFlagsTexts[] = {"LOWFOOD"    , "LOWMONEY"    , "ALONE"   , "SICKDOG"    , "DOGLESS"   , "NIGHT"   , "VERY_HOT"   , "RAINY"   , "VERY RAINY"   };
                int implementedFlagCount = sizeof(allFlags) / sizeof(unsigned int);

                Vector2 pos = {30,300};
                int margin = 20;
                int size = 20;
                int padding = 20;
                DrawRectangle(pos.x, pos.y, 200, (size*implementedFlagCount)+ padding * 2,WHITE);
                for (int i = 0; i < implementedFlagCount; i++){
                    
                    DrawText(allFlagsTexts[i], pos.x + padding, pos.y + padding + (i*margin),size,(game.activeFlags & allFlags[i]) == allFlags[i] ? RED : BLACK);
                }

            }

            // Draw Gameover
            if (game.state == STATE_GAMEOVER) {
                int size = 30;
                char *text = "EVERYONE DIED!";
                int width = MeasureText(text, size);
                DrawText(text, windowWidth / 2 - width / 2, windowHeight / 2, size, WHITE);
            }

            // Draw Event
            if (game.state == STATE_EVENT) {

                int size = 30;
                int width = MeasureText(events[game.currentEvent].message, size);
                int x = windowWidth / 2 - width / 2;
                int y =  (windowHeight / 2) - 100;

                if (events[game.currentEvent].numOptions == 0) {
                    DrawRectangleLines(x-10, y-10, width+20, size+20, WHITE);
                    DrawText(events[game.currentEvent].message, x, y, size, WHITE);
                } else {
                    DrawRectangleLines(x-10, y-10, width+20, size+20, WHITE);
                    DrawText(events[game.currentEvent].message, x, y, size, WHITE);

                    int opWidth = 200;
                    int opGap = 20;
                    int opTotalWidth = (events[game.currentEvent].numOptions - 1) * (opWidth + opGap) + opWidth;

                    for (int i = 0; i < events[game.currentEvent].numOptions; i++)
                    {
                        
                        if (GuiButton((Rectangle){((windowWidth / 2) - (opTotalWidth / 2)) + i*(opWidth+opGap),y+60,opWidth,40}, events[game.currentEvent].options[i].message)) {
                            if (events[game.currentEvent].options[i].nextId != -1) {
                                triggerEvent(events[game.currentEvent].options[i].nextId, &game.currentEvent, &game.state, &game.hours, &game.party.money, &game.party.inventory);
                            } else {
                                game.state = STATE_PLAYING;
                            }
                        }
                    }
            
                }
                
            }

            if (game.state == STATE_PLAYING){
                int height = 60;
                int width = 200;
                int margin = 60;
                if (GuiButton((Rectangle) {windowWidth - width - margin, windowHeight - height - margin, width, height}, "Stop")) {
                    game.state = STATE_STOP;
                    activeStopMenu = 0;
                }
            }
            
            if (game.state == STATE_PLAYING || game.state == STATE_EVENT) {

                {
                    float scale = 2;
                    DrawRectangle(0,500 + (48 * scale) + 2, windowWidth, 150, WHITE);
                    drawSprite(&spriteFamilia, windowWidth - (spriteFamilia.frameSize.x * scale) - 90 , 500, scale, 0, WHITE);
                }
                
                { // Draw action text -- "Moving" : "Enter to continue"
                    int size = 30;
                    char *text = moving ? "Moving" : "Enter to continue";
                    int width = MeasureText(text, size);
                    int x = windowWidth - width  - 30;
                    int y =  windowHeight - 50;
                    DrawText(text, x, y , size, WHITE);
                }

                
                { // Draw Hours, Distance, Food, Party Count
                    char *weatherText[5] = {"VERY HOT", "HOT", "COOL", "RAINY", "VERY RAINY"};
                    int size = 30;
                    {
                        int width = MeasureText(currentDateString(game.hours), size);
                        DrawText(currentDateString(game.hours), windowWidth - width - 30, 30, size, WHITE);
                    }
                    {
                        int width = MeasureText(weatherText[game.weather], size);
                        DrawText(weatherText[game.weather], windowWidth - width - 30, 60, size, WHITE);
                    }
                    DrawText(TextFormat("Distância: %.2fKm", game.distance),    30,  30, size, WHITE);
                    DrawText(TextFormat("Food: %d", game.party.inventory.food), 30,  60, size, WHITE);
                    DrawText(TextFormat("Alive: %d", game.party.count),         30,  90, size, WHITE);
                    DrawText(TextFormat("Money: %d", game.party.money),         30, 120, size, WHITE);
                    DrawText(TextFormat("Events: %d", activePoolCount),         30, 150, size, WHITE);
                    // DrawText(TextFormat("wSimCounter: %d", game.weatherSimCounter),         30, 180, size, WHITE);
                }


                // Draw status bar (party health)
                for (int i = 3; i >= 0; i--) {
                    int size = 30;
                    int posY = windowHeight - 50 - ((3-i)*30);
                    Color textColor = game.party.member[i].health == 0 ? RED : WHITE;
                    DrawText(game.party.member[i].name, 30, posY, 30, textColor);
                    DrawText(TextFormat("%03d", game.party.member[i].health), 230, posY, size, textColor);
                    if (game.party.member[i].health == 0) {
                        DrawText("(dead)", 310, posY, 30, textColor);
                        
                    } else if (game.party.member[i].sick) {
                        DrawText("(sick)", 310, posY, 30, textColor);
                        
                    }
                    DrawText(TextFormat("energy: %03d", game.party.member[i].energy), 410, posY, size, textColor);
                }

                { // Draw dog status bar (dog health)
                    int size = 30;
                    int posY = windowHeight - 200;
                    Color textColor = game.party.dog.health == 0 ? RED : WHITE;
                    DrawText(game.party.dog.name, 30, posY, 30, textColor);
                    DrawText(TextFormat("%03d", game.party.dog.health), 230, posY, size, textColor);
                    if (game.party.dog.health == 0) {
                        DrawText("(dead)", 310, posY, 30, textColor);
                        
                    } else if (game.party.dog.sick) {
                        DrawText("(sick)", 310, posY, 30, textColor);
                        
                    }
                }
            }

            // Stop Menu
            bool inCheckpoint = (game.state == STATE_STOP_CHECKPOINT);

            if (game.state == STATE_STOP || inCheckpoint) {
                int height = 60;
                int width = 200;
                #define SUBMENU_NONE -1
                #define SUBMENU_BUY   0
                #define SUBMENU_SELL  1
                // #define SUBMENU_REST  2
                if (activeStopSubmenu == SUBMENU_NONE) {

                    // const char* base = "Party;Faith;Supply";
                    char options[256];
                    char leaveText[32];
                    if (inCheckpoint){
                        switch (checkpoints.type[game.checkpointsVisited])
                        {
                        case TOWN:
                            strcpy(options, "Party;Faith;Supply;Town");
                            break;
                        case SITE:
                            strcpy(options, TextFormat("Party;Faith;Supply;%s", checkpoints.name[game.checkpointsVisited]));
                            break;
                        default:
                            break;
                        }
                        strcpy(leaveText, "Leave");
                    } else {
                        strcpy(options, "Party;Faith;Supply");
                        strcpy(leaveText, "Back");
                    }
                      
                    GuiToggleGroup((Rectangle){ 0,  windowHeight - height, width, height }, options, &activeStopMenu);
                    
                    if (GuiButton((Rectangle) {windowWidth - width, windowHeight - height, width, height}, leaveText)) {
                        if (inCheckpoint) game.checkpointsVisited++; // leave checkpoint
                        game.state = STATE_PLAYING;
                        moving = false;
                    }

                    switch (activeStopMenu) {
                    case 0 /* Party */:  
                        if (GuiButton((Rectangle) {40, 40, width, height}, "Rest")) restParty(&game.party, &game.hours);
                        // Draw status bar (party health)
                        for (int i = 3; i >= 0; i--) {
                            int size = 30;
                            int posY = windowHeight - 120 - ((3-i)*30);
                            Color textColor = game.party.member[i].health == 0 ? RED : WHITE;
                            DrawText(game.party.member[i].name, 30, posY, 30, textColor);
                            DrawText(TextFormat("%03d", game.party.member[i].health), 230, posY, size, textColor);

                            if (game.party.member[i].health == 0){
                                DrawText("(dead)", 310, posY, 30, textColor);
                            } else if (game.party.member[i].sick) {
                                DrawText("(sick)", 310, posY, 30, textColor);
                            }

                            DrawText(TextFormat("energy: %03d", game.party.member[i].energy), 410, posY, size, textColor);
                        }
                                                
                        { // Draw dog status bar (dog health)
                            int size = 30;
                            int posY = windowHeight - 300;
                            Color textColor = game.party.dog.health == 0 ? RED : WHITE;
                            DrawText(game.party.dog.name, 30, posY, 30, textColor);
                            DrawText(TextFormat("%03d", game.party.dog.health), 230, posY, size, textColor);
                            if (game.party.dog.health == 0) {
                                DrawText("(dead)", 310, posY, 30, textColor);
                                
                            } else if (game.party.dog.sick) {
                                DrawText("(sick)", 310, posY, 30, textColor);
                            }
                        }
                        
                        { // Draw Hours, Distance, Food, Party Count
                            int size = 30;
                            DrawText(TextFormat("Horas: %02d", game.hours),             30,  230, size, WHITE);
                            DrawText(TextFormat("Food: %d", game.party.inventory.food), 30,  260, size, WHITE);
                        }
                        break;
                    case 1 /* Faith */:  
                        GuiButton((Rectangle) {40, 40, width, height},  "Pray");
                        GuiButton((Rectangle) {40, 120, width, height}, "Read");
                        GuiButton((Rectangle) {40, 200, width, height}, "Speak");
                        break;
                    case 2 /* Supply */:  
                        GuiButton((Rectangle) {40, 40, width, height},  "Scavange");
                        GuiButton((Rectangle) {40, 120, width, height}, "Trade");
                        GuiButton((Rectangle) {40, 200, width, height}, "Rationing");
                        break;
                    case 3 /* Checkpoint */:  
                        { // Draw city menu
                            int size = 30;
                            int width = MeasureText(checkpoints.name[game.checkpointsVisited], size);
                            int x = windowWidth / 2 - width / 2;
                            int y =  windowHeight / 2;
                            DrawRectangle(x-10, y-10, width+20, size+20, WHITE);
                            DrawText(checkpoints.name[game.checkpointsVisited], x, y, size, BLACK);
                        }
                        switch (checkpoints.type[game.checkpointsVisited])
                        {
                        case TOWN:
                            if (GuiButton((Rectangle) {40, 40, width, height}, "Buy"))   activeStopSubmenu = SUBMENU_BUY;
                            if (GuiButton((Rectangle) {40, 120, width, height}, "Sell")) activeStopSubmenu = SUBMENU_SELL;
                            break;
                        case SITE:
                            if (GuiButton((Rectangle) {40, 40, width, height}, "Explore"));
                            if (GuiButton((Rectangle) {40, 120, width, height},   "Call"));
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
                else if (activeStopSubmenu == SUBMENU_BUY) {
                    
                    int margin = 60;
                    if (GuiButton((Rectangle) {windowWidth - width - margin, windowHeight - height - margin, width, height}, "Done")) activeStopSubmenu = SUBMENU_NONE;
                    DrawText(TextFormat("Money: %d", game.party.money), 30, windowHeight - 60, 30, WHITE);
                    // (Inventory) {food, ammo, weapon, footwear}
                    DrawText(TextFormat("FOOD:  %03d", checkpoints.inventory[game.checkpointsVisited].food),       60,  40, 30, WHITE);
                    if (GuiButton((Rectangle) {250, 40, 30, 30}, "+")) buy(&checkpoints.inventory[game.checkpointsVisited].food, &game.party.inventory.food, 5, &game.party.money);
                    DrawText(TextFormat("AMMO:  %03d", checkpoints.inventory[game.checkpointsVisited].ammo),       60,  80, 30, WHITE);
                    if (GuiButton((Rectangle) {250, 80, 30, 30}, "+")) buy(&checkpoints.inventory[game.checkpointsVisited].ammo, &game.party.inventory.ammo, 5, &game.party.money);
                    DrawText(TextFormat("GUNS:  %03d", checkpoints.inventory[game.checkpointsVisited].weapon),     60, 120, 30, WHITE);
                    if (GuiButton((Rectangle) {250, 120, 30, 30}, "+")) buy(&checkpoints.inventory[game.checkpointsVisited].weapon, &game.party.inventory.weapon, 10, &game.party.money);
                    DrawText(TextFormat("SHOES: %03d", checkpoints.inventory[game.checkpointsVisited].footwear),   60, 160, 30, WHITE);
                    if (GuiButton((Rectangle) {250, 160, 30, 30}, "+")) buy(&checkpoints.inventory[game.checkpointsVisited].footwear, &game.party.inventory.footwear, 10, &game.party.money);
                }
                else if (activeStopSubmenu == SUBMENU_SELL) {
                    int margin = 60;
                    if (GuiButton((Rectangle) {windowWidth - width - margin, windowHeight - height - margin, width, height}, "Done")) activeStopSubmenu = SUBMENU_NONE;
                }
            }
        EndDrawing();
    }

    UnloadTexture(spriteFamilia.texture);

    for (int i = 0; i < MAX_EVENTS; i++) {
        if (events[i].effects != NULL) {
            free(events[i].effects);
        }
    }

    CloseWindow();
    return 0;
}