/*
* File: main.c
* Author: Willard Zamora
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

struct Bridge {
    int length;
    int direction;
    int occupied;
    char* state;
    pthread_mutex_t* spot;
};

struct Vehicle{
    int direction;
    int startPosition;
    char symbol;
};

double expMeanW;
double expMeanE;
int vehiclesW;
int vehiclesE;
int vehiclesWO;
int vehiclesEO;
int waitingW;
int waitingE;
int speed;

pthread_mutex_t bridgeOccupied;
pthread_mutex_t yieldDirection;
pthread_mutex_t sysOut;
pthread_cond_t directionChanged;

struct Bridge bridge;

void* spawnW();
void* spawnE();
void* crossBridge(void* vehicle);
void printState();
void parseAndValidateParams();

int main(int argc, char** argv) {
    parseAndValidateParams(argc, argv);

    bridge.occupied = 0;
    bridge.direction = 1;
    bridge.spot = malloc(bridge.length * sizeof (pthread_mutex_t));
    bridge.state = (char *) malloc(bridge.length);

    for (int i = 0; i < bridge.length; i++) {
        pthread_mutex_init(&bridge.spot[i], NULL);
        bridge.state[i] = '_';
    }

    srand(time(NULL));

    pthread_mutex_init(&bridgeOccupied, NULL);
    pthread_mutex_init(&yieldDirection, NULL);
    pthread_mutex_init(&sysOut, NULL);
    pthread_cond_init(&directionChanged, NULL);

    pthread_t spawnerW;
    pthread_t spawnerE;

    pthread_attr_t joinable;
    pthread_attr_init(&joinable);
    pthread_attr_setdetachstate(&joinable, PTHREAD_CREATE_JOINABLE);

    pthread_create(&spawnerW, &joinable, spawnW, NULL);
    pthread_create(&spawnerE, &joinable, spawnE, NULL);
    pthread_attr_destroy(&joinable);

    pthread_exit(NULL);
    free(bridge.spot);
    free(bridge.state);
}

void parseAndValidateParams(int argc, char** argv){
    int withErrors = 0;

    expMeanE = expMeanW = 0.25;
    speed = 1;
    bridge.length = 5;

    if(argc == 2 && strcmp(argv[1], "-h") == 0){
        printf("\n%s\n","Application receives up to 6 parameters, with the first two being mandatory, in the following order: \n\t Vehicles coming East : +int \n\t Vehicles coming West : +int \n\t East Spawn Mean : +double \n\t West Spawn Mean : +double \n\t Bridge Length : +int \n\t Vehicle Speed : +int\n");
        exit(EXIT_SUCCESS);
    }else{
        if(argc < 3){
            printf("\n%s\n\n","Minimum number of parameter not meet, please use -h to see usage.");
            exit(EXIT_FAILURE);
        }else{
            vehiclesEO = vehiclesE = atoi(argv[1]);
            if(vehiclesE <= 0){
                withErrors = 1;
                printf("\n%s\n","Vehicles coming East must be a positive integer.");
            }
            vehiclesWO = vehiclesW = atoi(argv[2]);
            if(vehiclesW <= 0){
                withErrors = 1;
                printf("\n%s\n","Vehicles coming West must be a positive integer.");
            }
            if(argc >= 4 ){
                expMeanE = atof(argv[3]);
                if(expMeanE <= 0){
                    withErrors = 1;
                    printf("\n%s\n","East Spawn Mean must be a positive double.");
                }
            }
            if(argc >= 5){
                expMeanW = atof(argv[4]);
                if(expMeanW <= 0){
                    withErrors = 1;
                    printf("\n%s\n","West Spawn Mean must be a positive double.");
                }
            }
            if(argc >= 6){
                bridge.length = atoi(argv[5]);
                if(bridge.length <= 0){
                    withErrors = 1;
                    printf("\n%s\n","Bridge Length must be a positive integer.");
                }
            }
            if(argc == 7){
                speed = atoi(argv[6]);
                if(speed <= 0){
                    withErrors = 1;
                    printf("\n%s\n","Vehicle Speed must be a positive integer.");
                }
            }
        }
    }
    if(withErrors){
        printf("\n%s\n\n","Parameters type does not match, please use -h to see usage.");
        exit(EXIT_FAILURE);
    }
}

double expDist(double lambda) {
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

void* spawnW() {
    while (vehiclesW > 0) {
        vehiclesW--;
        pthread_t vehicleW;
        struct Vehicle* vehicle = malloc(sizeof(struct Vehicle));
        vehicle->direction = 1;
        vehicle->symbol = '>';
        vehicle->startPosition = 0;
        pthread_create(&vehicleW, NULL, crossBridge, (void*) vehicle);
        sleep(expDist(expMeanW));
    }
    pthread_exit(NULL);
}

void* spawnE() {
    while (vehiclesE > 0) {
        vehiclesE--;
        pthread_t vehicleE;
        struct Vehicle* vehicle = malloc(sizeof(struct Vehicle));
        vehicle->direction = -1;
        vehicle->symbol = '<';
        vehicle->startPosition = bridge.length - 1;
        pthread_create(&vehicleE, NULL, crossBridge, (void*) vehicle);
        sleep(expDist(expMeanE));
    }
    pthread_exit(NULL);
}

void* crossBridge(void* vehicle){
    struct Vehicle* currentVehicle = (struct Vehicle*) vehicle;
    pthread_mutex_lock(&yieldDirection);
    if(currentVehicle->direction == 1){
        waitingW++;
    }else{
        waitingE++;
    }
    if(!bridge.occupied && currentVehicle->direction != bridge.direction){
        bridge.direction *= -1;
    }
    while (currentVehicle->direction != bridge.direction){
        pthread_cond_wait(&directionChanged, &yieldDirection);
    }
    pthread_mutex_unlock(&yieldDirection);
    int position = currentVehicle->startPosition;
    int goingIn = 1;
    while (position > -1 && position < bridge.length){
        pthread_mutex_lock(&bridge.spot[position]);
        if(goingIn){
            pthread_mutex_lock(&bridgeOccupied);
            bridge.occupied++;
            pthread_mutex_unlock(&bridgeOccupied);
            if(currentVehicle->direction == 1){
                waitingW--;
            }else{
                waitingE--;
            }
            goingIn = 0;
        }
        bridge.state[position] = currentVehicle->symbol;
        printState();
        sleep(speed);
        bridge.state[position] = '_';
        pthread_mutex_unlock(&bridge.spot[position]);
        printState();
        position+= currentVehicle->direction;
    }
    pthread_cond_broadcast(&directionChanged);
    pthread_mutex_lock(&bridgeOccupied);
    bridge.occupied--;
    pthread_mutex_unlock(&bridgeOccupied);
    pthread_mutex_lock(&yieldDirection);
    if(!bridge.occupied && (waitingW || waitingE)){
        bridge.direction *= -1;
    }
    pthread_mutex_unlock(&yieldDirection);
    free(currentVehicle);
    pthread_exit(NULL);
}

void printState() {
    pthread_mutex_lock(&sysOut);
    system("clear");
    system("clear");

    printf("%s\n","Parameters");
    printf("\n\t%s %i\n","Vehicles coming East", vehiclesEO);
    printf("\n\t%s %i\n","Vehicles coming West", vehiclesWO);
    printf("\n\t%s %lf\n","Vehicles East Spawn Mean", expMeanE);
    printf("\n\t%s %lf\n","Vehicles West Spawn  Mean", expMeanW);
    printf("\n\t%s %i\n","Bridge Length", bridge.length);
    printf("\n\t%s %i\n","Vehicle Speed", speed);

    printf("\n\n%s\n","Bridge Status");
    printf("\n\t%s %i\n","Vehicles remaining west", vehiclesW);
    printf("\n\t%s %i\n","Vehicles remaining east", vehiclesE);
    printf("\n\t%s %i\n","Vehicles waiting west", waitingW);
    printf("\n\t%s %i\n","Vehicles waiting east", waitingE);
    pthread_mutex_lock(&yieldDirection);
    printf("\n\t%s %s\n","Bridge is going ", (bridge.direction == 1) ? "West to East" : "East to West");
    pthread_mutex_unlock(&yieldDirection);
    printf("\n\t\t%s\n", bridge.state);
    pthread_mutex_unlock(&sysOut);
}
