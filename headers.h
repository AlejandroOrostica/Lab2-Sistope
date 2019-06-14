#ifndef headers_h
#define headers_h

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>

int listo , esperandoHijas ;

typedef struct 
{
    float* buffer;
    float mediaReal;
    float mediaImaginaria;
    float potencia;
    float ruido;
    float sumReales;
    float sumImaginarios;
    int datosLeidos, hebra;
    int datosEnBuffer, tamanoBuffer;
    pthread_mutex_t vacio, lleno, enUso;
    pthread_cond_t vacioCond, llenoCond;
 
}monitorHebra;

typedef struct {
    int hebra;
    float mediaReal;
    float mediaImaginaria;
    float potencia;
    float ruido;
    pthread_mutex_t enUso, escribendoArchivo;
    pthread_cond_t enUsoCond, escribendoArchivoCond;
}monitorEscritura;


//funcion que es encarga de inicializar la estructura monitorHebra recibiendo como parametro tamano que corresponde al tamanio del buffer entregado por el usuario
monitorHebra* init_monitorHebra(int tamano,int hebra);

 
monitorEscritura* init_monitorEscritura ();

monitorEscritura* mE ;

float* vaciarBuffer(float* buffer, int n );

//funcion que se encarga de transformar la linea leida del documento de texto y convertirla en una lista de numeros
char** procesarLinea(char* linea,char** lista);
//Funciones que se encargan de calcular los datos pedidos en el enunciado
//Funcion que calcula la media real
float calcularMediaReal(float numerosReales, int n );
//funcion que calcula la media imaginaria
float calcularMediaImaginaria(float numerosImaginarios, int n );
//calcular potencia, calcula la potencia de forma individual antes de ser sumadas a todos los datos
float calcularPotencia(float real, float imaginario);

//Funcion que se encarga de calcular la distancia para poder enviar los datos al disco correspondiente
float calcularDistancia(char** lista);

monitorHebra* calculadora(monitorHebra* monitor);



void * consumir(void* monitor);

void* producir (monitorHebra* monitor,int tamano, float dato);
void crearSalida(char* nombreArchivo, int hijo,float mediaReal,float mediaImaginaria,float potencia, float ruido);
#endif