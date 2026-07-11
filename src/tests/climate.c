#include <stdio.h>
#include <time.h>
#include "raylib.h"

enum Weather {
    VERY_HOT,     // 0
    HOT,          // 1
    COOL,         // 2
    RAINY,        // 3
    VERY_RAINY,   // 4
};


void simulateWeather(enum Weather *weather, int currentMonth){

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

int main(){
    SetRandomSeed((unsigned int)time(NULL));
    const char* meses[12] = {"JAN", "FEV", "MAR", "ABR", "MAI", "JUN", "JUL", "AGO", "SET", "OUT", "NOV", "DEZ"};
    const char* weatherText[5] = {"VERY_HOT", "HOT", "WARM", "RAINY", "VERY_RAINY"};
    enum Weather weather = HOT;

    for (int i = 1; i < 2; i++){
        printf("ANO %d\n", i);
        for (int i = 0; i < 12; i++){
            int month = i % 12;
            printf("%s\n", meses[i]);
            for (int i = 0; i < 62; i++){
                simulateWeather(&weather, month);
                printf("%s|", weatherText[weather]);

            }
            printf("\n-------\n");
        }
        printf("-----------------\n");

    }
    
}