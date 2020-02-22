/* 
 * File:   main.c
 * Author: Brayan
 *
 * Created on 11 de marzo de 2015, 02:07 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

struct Puente {
    int canCarrosPuente;
    int direccionPuente;
    int tamPuente;
    pthread_mutex_t *arreglo;
};
typedef enum {true, false} bool;


int semaforo = 0;
int tiempoSemaforoEste;
int tiempoSemaforoOeste;


int mediaExponencial;
int velocidadMedia;

int totalOeste;
int totalEste;

pthread_mutex_t canAmbOE;
pthread_mutex_t canAmbEO;

pthread_mutex_t mutexPuente;
pthread_mutex_t mutexEO;
pthread_mutex_t mutexOE;
pthread_cond_t condEO;
pthread_cond_t condOE;

pthread_mutex_t mutex;
pthread_cond_t cond;
int esperaoeste=0;
int esperaeste=0;
int pasadoeste=0;
int pasadooeste=0;
int dirTrafico = 0;

pthread_mutex_t imprimir;

struct Puente puente;
char* puenteGrafic;

int tiempoExponencial();
int aleatorio();
void *generarCarrosOE(void*X);
void *generarCarrosEO(void*X);

void *moverOesteEste(void*p);
void *moverEsteOeste(void*p);
void cambiarDireccionAumentarPuente(int valor);
void cambiarDireccionPuente(int valor);
void aumentarCarrosPuente();
void disminuirCarrosPuente();

int bolquearPosPuente(int pos,char caracter);
void desboloquearPosPuente(int pos);
void imprimirEstado();

void *cambiarSemaforo(void*X);
void adminSemaforoOE();
void adminSemaforoEO();

void adminCedaOE();
void adminCedaEO();
void cambiaraoeste();
void cambiaraeste();
void *ceda(void*X);

void adminTraficoOE();
void adminTraficoEO();
void *trafico(void*X);

int main(int argc, char** argv) {

    FILE *f = fopen("Datos.txt", "r");
    if(f==NULL){
        printf("%s\n","No se pudo leer el archivo");
    }else{
        fscanf (f, "%d", &puente.tamPuente);
        fscanf (f, "%d", &mediaExponencial);
        fscanf (f, "%d", &velocidadMedia);
    }

    fclose(f);
    
    puente.canCarrosPuente = 0;
    puente.direccionPuente = 0;
    puente.arreglo = malloc(puente.tamPuente * sizeof (pthread_mutex_t));
    puenteGrafic = (char *) malloc(puente.tamPuente);

    long i;
    for (i = 0; i < puente.tamPuente; i++) {
        pthread_mutex_init(&puente.arreglo[i], NULL);
        puenteGrafic[i] = '_';
    }

    pthread_mutex_init(&imprimir, NULL);
    pthread_mutex_init(&canAmbOE,NULL);
    pthread_mutex_init(&canAmbEO,NULL);
    pthread_mutex_init(&mutexEO, NULL);
    pthread_mutex_init(&mutexOE, NULL);
    pthread_mutex_init(&mutexPuente, NULL);
    pthread_cond_init(&condEO, NULL);
    pthread_cond_init(&condOE, NULL);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t hiloAdmin;
    pthread_t generarOE;
    pthread_t generarEO;

    void *status;
    pthread_attr_t atributo;
    pthread_attr_init(&atributo);
    pthread_attr_setdetachstate(&atributo, PTHREAD_CREATE_JOINABLE);

    puente.direccionPuente = 0;
    pthread_create(&hiloAdmin, &atributo, ceda, NULL);
    pthread_create(&generarEO, &atributo, generarCarrosEO, NULL);
    pthread_create(&generarOE, &atributo, generarCarrosOE, NULL);
    pthread_attr_destroy(&atributo);

    pthread_exit(NULL);
}

int tiempoExponencial() {
    srand(time(NULL));
    double me = 0.0f;
    int res = 0;
    do {
        double n = 0 + (1 - 0) * rand() / ((double) RAND_MAX);
        double resultado = log(1 - n);
        me = (-mediaExponencial) * resultado;
        res = me;
    } while (res == 0 || res > 9);
    return res;
}

int aleatorio() {
    srand(time(NULL));
    return rand() % 2;
}

void *ceda(void*X) {
    while (1) {
        if(esperaoeste > 0 && esperaeste == 0 && puente.direccionPuente == 0)
            cambiaraoeste();
        if(esperaeste > 0 && esperaoeste == 0 && puente.direccionPuente == 1)
            cambiaraeste();
    }
}

void cambiaraoeste(){
    while(puente.canCarrosPuente > 0)
        pthread_cond_wait(&cond, &mutex);
    printf("%s %i\n","no esperan al este y al oeste si",puente.canCarrosPuente);
    puente.direccionPuente=1;
    pasadooeste=0;
    pasadoeste=0;
    pthread_cond_broadcast(&condOE);
}

void cambiaraeste(){
    while(puente.canCarrosPuente > 0)
        pthread_cond_wait(&cond, &mutex);
    printf("%s %i\n","iguales al oeste",puente.canCarrosPuente);
    puente.direccionPuente=0;
    pasadoeste=0;
    pasadooeste=0;
    pthread_cond_broadcast(&condEO);
}

void adminCedaOE(){
    bool esperar = !puente.direccionPuente;
    if (esperar) {
        esperaoeste++;
        while (!puente.direccionPuente )
            pthread_cond_wait(&condOE, &mutexOE);
        esperaoeste--;
    }
}

void adminCedaEO(){
    bool esperar = puente.direccionPuente;
    if (esperar) {
        esperaeste++;
        while (puente.direccionPuente)
            pthread_cond_wait(&condEO, &mutexEO);
        esperaeste--;
    }
}

void *generarCarrosOE(void*X) {
    int res = tiempoExponencial();
    int t;
    int *esAmbulancia[1];
    esAmbulancia[0] = (int *) malloc(sizeof (int));
    t = 0;
    while (1) {
        *esAmbulancia[0] = 0;
        if (t > 0 && t % 15 == 0) {
            *esAmbulancia[0] = random();
        }
        pthread_t carroOE;
        pthread_create(&carroOE, NULL, moverOesteEste, (void*) esAmbulancia[0]);
        sleep(res);
        t++;
    }
    pthread_exit(NULL);
}

void *generarCarrosEO(void*X) {
    int res = tiempoExponencial();
    int t;
    int *esAmbulancia[1];
    esAmbulancia[0] = (int *) malloc(sizeof (int));
    t = 0;
    while (1) {
        *esAmbulancia[0] = 0;
        if (t > 0 && t % 15 == 0) {
            *esAmbulancia[0] = random();
        }
        pthread_t carroEO;
        pthread_create(&carroEO, NULL, moverEsteOeste, (void*) esAmbulancia[0]);
        sleep(res);
        t++;
    }
    pthread_exit(NULL);
}

void *moverOesteEste(void* p) {
    int *id_ptr, taskid;
    id_ptr = (int *) p;
    taskid = *id_ptr;
    if (!taskid) {
         pthread_mutex_lock(&mutexOE);
        adminCedaOE();
        cambiarDireccionAumentarPuente(1);
        int posActual = -1;
        posActual = bolquearPosPuente(posActual + 1,'>');
        sleep(velocidadMedia);
        while (posActual < puente.tamPuente - 1) {
            posActual = bolquearPosPuente(posActual + 1,'>');
            desboloquearPosPuente(posActual-1);
            if(posActual == 1){
                pthread_mutex_unlock(&mutexOE);
            }
            sleep(velocidadMedia);
        }
        desboloquearPosPuente(posActual);
        disminuirCarrosPuente();
        if (puente.canCarrosPuente == 0) {
            pthread_cond_broadcast(&condEO);
            pthread_cond_broadcast(&cond);

        }
    } 
    pthread_exit(NULL);
}

void *moverEsteOeste(void* p) {
    int *id_ptr, taskid;
    id_ptr = (int *) p;
    taskid = *id_ptr;
    if (!taskid) {
        pthread_mutex_lock(&mutexEO);
        adminCedaEO();
        cambiarDireccionAumentarPuente(0);
        int posActual = bolquearPosPuente(puente.tamPuente - 1, '<');
        sleep(velocidadMedia);
        while (posActual > 0) {
            posActual = bolquearPosPuente(posActual - 1, '<');
            desboloquearPosPuente(posActual+1);
            if(posActual == puente.tamPuente-2){
                pthread_mutex_unlock(&mutexEO);
            }
            sleep(velocidadMedia);
        }
        desboloquearPosPuente(posActual);
        disminuirCarrosPuente();
        if (puente.canCarrosPuente == 0) {
            pthread_cond_broadcast(&cond);                           
        }
    } 
    pthread_exit(NULL);
}


void cambiarDireccionAumentarPuente(int valor){
    pthread_mutex_lock(&mutexPuente);
    // if(valor == 0)
    //    pthread_mutex_lock(&mutexOE);
    // else
    //    pthread_mutex_lock(&mutexEO);
    cambiarDireccionPuente(valor);
    aumentarCarrosPuente();
    // if(valor == 0)
    // pthread_mutex_lock(&mutexOE);
    // else
    // pthread_mutex_lock(&mutexEO);
    pthread_mutex_unlock(&mutexPuente);
}

void cambiarDireccionPuente(int valor){
    puente.direccionPuente = valor;
}

void aumentarCarrosPuente(){
    puente.canCarrosPuente++;
}

void disminuirCarrosPuente(){
    pthread_mutex_lock(&mutexPuente);
    puente.canCarrosPuente--;
    pthread_mutex_unlock(&mutexPuente);
}

int bolquearPosPuente(int pos,char caracter){
    pthread_mutex_lock(&puente.arreglo[pos]);
    puenteGrafic[pos] = caracter;
    pthread_mutex_lock(&imprimir);
    imprimirEstado();
    pthread_mutex_unlock(&imprimir);
    return pos;
}

void desboloquearPosPuente(int pos){
    pthread_mutex_unlock(&puente.arreglo[pos]);
    puenteGrafic[pos] = '_';
    pthread_mutex_lock(&imprimir);
    imprimirEstado();
    pthread_mutex_unlock(&imprimir);
}

void imprimirEstado() {
    system("clear");
    system("clear");
    long i;

    printf("\t%s\n","Administración CEDA");        
    printf("\n%s %i\n","Cantidad de carros en espera al Oeste ",esperaoeste);
    printf("\n%s %i\n","Cantidad de carros en espera al Este ",esperaeste);
    printf("\n%s %i\n","Via actual del puente ",puente.direccionPuente);

    printf("\n\t%s\n", puenteGrafic);
}
