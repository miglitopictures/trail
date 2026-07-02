#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include <time.h>

enum State {
    PLAYING,
    GAMEOVER
};

typedef struct {
    char name[16];
    bool sick;
    int health;
} Person;


int randomBetween(int min, int max){
    return (rand() % (max - min + 1)) + min;
}

void printStatus(Person person){
    
    printf("%-7s - %03d ", person.name, person.health);
    if (person.health == 0){
        printf("(dead) ");

    } else if (person.sick){
        printf("(sick) ");

    }
    printf("\n");
}

int main(){
    srand(time(NULL)); // RNG seed based on current time

    enum State gameState = PLAYING;
    int day = 1;

    Person party[4];

    printf("======= VIDAS SECAS =======\n");


    // Start Config

    for (int i = 0; i < 4; i++){
            if (i == 0) {
                printf("Vossa desgraçensa: ");
            } else {
                printf("Nome do parente %02d: ", i+1);
            }
            
            scanf("%s%*c", party[i].name);
            party[i].health = 100;
            party[i].sick = false;
    }

    

    // print status party
    printf("\n================\n");
    for (int i = 0; i < 4; i++){
        printStatus(party[i]);        
    }
    printf("\n================\n\n");


    while(true){
        printf("Avançar jornada (S/N)? ");
        char responseBuffer[16];
        scanf("%s%*c", responseBuffer);

        if (strcasecmp(responseBuffer, "N") == 0) gameState = GAMEOVER; //deseja sair

        // simular
        for (int i = 0; i < 4; i++){
            if (!party[i].sick){
                int roll = randomBetween(1,100);
                if (roll < 10){
                    party[i].sick = true;
                }
            } else {
                int roll = randomBetween(0,100);
                if (roll < 50){
                    party[i].sick = false;
                }
            }


            int damage = randomBetween(5,10) * (party[i].sick ? 2 : 1);
            party[i].health -= damage;

            if (party[i].health < 0) {party[i].health = 0;}
                    
        }


        // check health sum for gameover
        int healthSum = 0;
        for (int i = 0; i < 4; i++){
            healthSum += party[i].health;
        }
        if (healthSum == 0)
        {
            gameState = GAMEOVER;
        }
        
        


        

        printf("\n\n== Dia %02d ==", day);

        // print status party
        printf("\n================\n");
        for (int i = 0; i < 4; i++){
            printStatus(party[i]);        
        }
        printf("\n================\n\n");

        day++;

        if (gameState == GAMEOVER) break;
    }

    printf("\n== End ==\n");


    return 0;
}