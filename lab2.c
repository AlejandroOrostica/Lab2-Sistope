#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>


typedef struct 
{
    float* buffer;
    float mediaReal;
    float mediaImaginaria;
    float potencia;
    float ruido;
    float sumReales;
    float sumImaginarios;
    int contador;   
    pthread_mutex_t termina, lleno;

}monitorHebra;

typedef struct 
{
    pthread_cond_t enUso;
}monitorEscritura;

monitorHebra* init_monitorHebra(int tamano){
    monitorHebra* mH = (monitorHebra*)malloc(sizeof(monitorHebra));
    mH->buffer = (float*)malloc(sizeof(float)*tamano*3);
    mH->mediaReal = 0.0;
    mH->mediaImaginaria = 0.0;
    mH->potencia = 0.0;
    mH->ruido = 0.0;
    mH->sumImaginarios = 0.0;
    mH->sumReales = 0.0;
    mH->contador = 0;
    pthread_mutex_init(&mH->termina,NULL);
    pthread_mutex_init(&mH->lleno,NULL);

}

char** procesarLinea(char* linea,char** lista){

    int tamano = strlen(linea);
	char delimitador[2] = ",";

	char *token = strtok(linea, delimitador);
    int j =0;
	while(token != NULL)
	{
		
		lista[j]=token;
        token = strtok(NULL, delimitador);
        
        j++;
	}


	
    return lista;
	
}

float calcularDistancia(char** lista){
    float distancia =  sqrtf( powf(atof(lista[0]),2.0) +  powf(atof(lista[1]),2.0) );
    return distancia;
}

int main(int argc, char const *argv[]){
    int i; 
    char buffer [100];
    char* linea= (char*)malloc(sizeof(char)*100);
    char** lista= (char**)malloc(sizeof(char*)*5);
    FILE* archivo;
    archivo = fopen("prueba.csv","r");
    while(!feof){
        int i=0;
        int pos=0;
        while (i<1){
            fread(buffer,1,1, archivo);
            if(strcmp(buffer,"\n")==0||feof(archivo)){
                i++;
            }
            else{
                linea[pos]=buffer[0];
                pos++;
            }
    
        }
    }

    return 0;
}