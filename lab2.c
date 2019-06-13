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
    float datosLeidos;
    int datosEnBuffer;   
    pthread_mutex_t termina, lleno;
    pthread_cond_t terminaC, llenoC;

}monitorHebra;

typedef struct {
    int hebra;
    float mediaReal;
    float mediaImaginaria;
    float potencia;
    float ruido;
    pthread_mutex_t enUso;
}monitorEscritura;


//funcion que es encarga de inicializar la estructura monitorHebra recibiendo como parametro tamano que corresponde al tamanio del buffer entregado por el usuario
monitorHebra* init_monitorHebra(int tamano){
    monitorHebra* mH = (monitorHebra*)malloc(sizeof(monitorHebra));
    mH->buffer = (float*)malloc(sizeof(float)*tamano*3);
    mH->mediaReal = 0.0;
    mH->mediaImaginaria = 0.0;
    mH->potencia = 0.0;
    mH->ruido = 0.0;
    mH->sumImaginarios = 0.0;
    mH->sumReales = 0.0;
    mH->datosLeidos = 0.0;
    mH->datosEnBuffer = 0;
    pthread_mutex_init(&mH->termina,NULL);
    pthread_mutex_init(&mH->lleno,NULL);
    pthread_mutex_lock(&mH->lleno);
    return mH;

}

float* vaciarBuffer(float* buffer, int n ){
    int i;
    for(i=0;i<n ; i++){
        buffer[i] = 0.0;
    }
    return buffer;
}


//funcion que se encarga de transformar la linea leida del documento de texto y convertirla en una lista de numeros
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
//Funciones que se encargan de calcular los datos pedidos en el enunciado
//Funcion que calcula la media real
float calcularMediaReal(float numerosReales, int n ){
    
    float media = 0.0; 


    media= numerosReales / n; 
    return media;
}
//funcion que calcula la media imaginaria
float calcularMediaImaginaria(float numerosImaginarios, int n ){
  

    float media = media / n ;
    return media;
}
//calcular potencia, calcula la potencia de forma individual antes de ser sumadas a todos los datos
float calcularPotencia(float real, float imaginario,float n){
    float i;
   
    
    
    float potencia = sqrtf(powf(real, 2.0) + powf(imaginario, 2.0));
    
    return potencia;
}

//Funcion que se encarga de calcular la distancia para poder enviar los datos al disco correspondiente
float calcularDistancia(char** lista){
    float distancia =  sqrtf( powf(atof(lista[0]),2.0) +  powf(atof(lista[1]),2.0) );
    return distancia;
}



void * imprime(void* numero){
    int* num=(int*)numero;
    printf("cacaca %i \n",*num);
}






int main(int argc, char const *argv[]){
    int i, numeroDiscos, tamano, radio;
    pthread_mutex_t trabajoHijo; // Cuando el buffer de un monitor se llene, este mutex
    tamano = 3; // se encargará de hacer esperar al padre hasta que la hebra termine de
    radio = 100; // procesar los datos 
    float distancia; 
    int numeroHebras=5;
    char buffer [100];
    numeroDiscos = 2;
    monitorHebra** arregloMonitores;
    char* linea= (char*)malloc(sizeof(char)*100);
    char** lista= (char**)malloc(sizeof(char*)*5);
    FILE* archivo;

    monitorHebra* mh = init_monitorHebra(2);
    arregloMonitores = (monitorHebra**)malloc(sizeof(monitorHebra*)*2);
    
    for(i = 0; i<numeroDiscos ; i++){
        arregloMonitores[i] = init_monitorHebra(tamano);
    }

    archivo = fopen("prueba.csv","r");
    while(!feof(archivo)){
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

        lista = procesarLinea(linea,lista); // Lista = [Vu,Vv,V(u,v).r,V(u,v).i,V(u,v).w]
        distancia = calcularDistancia(lista);
        printf("la distancia es %f \n", distancia);
        for(i = 0; i< numeroDiscos ; i++){
            float min = radio * i;
            float max= (radio*(i+1));
            if(i==numeroDiscos-1 ){
                
                int j = 2; // J = 2 Para ignorar los dos primeros elementos de la lista
                while(j<5){
                    arregloMonitores[i]->buffer[arregloMonitores[i]->datosEnBuffer] = atof(lista[j]);
                    j++;
                    arregloMonitores[i]->datosEnBuffer++;
                    arregloMonitores[i]->datosLeidos++;
                    if( (tamano*3) == arregloMonitores[i]->datosEnBuffer){
                        pthread_mutex_unlock(&arregloMonitores[i]->lleno);
                        pthread_mutex_lock(&trabajoHijo);
                        arregloMonitores[i]->datosEnBuffer = 0;
                        arregloMonitores[i]->buffer = vaciarBuffer(arregloMonitores[i]->buffer,tamano*3);
                        printf("el buffer se llenó \n");
                    }
                    

                }
                break;
            }
            else if(distancia<max && distancia>=min){
                int j = 2 ;
                while(j<5){
                    arregloMonitores[i]->buffer[arregloMonitores[i]->datosEnBuffer] = atof(lista[j]);
                    j++;
                    arregloMonitores[i]->datosEnBuffer++;
                    arregloMonitores[i]->datosLeidos++;
                    if((tamano*3) == arregloMonitores[i]->datosEnBuffer){
                        pthread_mutex_unlock(&arregloMonitores[i]->lleno);
                        pthread_mutex_lock(&trabajoHijo);
                        arregloMonitores[i]->datosEnBuffer = 0;
                        arregloMonitores[i]->buffer = vaciarBuffer(arregloMonitores[i]->buffer,tamano*3);
                        printf("el buffer se llenó \n");
                    }

                }
                break;
            }


        }
        
        
    }
    int algo = 5 ;
    pthread_t thr[3];
    int j=0;

    for(i = 0; i <tamano*3 ; i++){
        printf("%f\n",arregloMonitores[0]->buffer[i] );
    }
    for(i = 0; i <tamano*3 ; i++){
        printf("%f\n",arregloMonitores[1]->buffer[i] );
    }
    //En esta parte el proceso padre inicializa las hebras que corresponden al numero de discos
    while(j < numeroHebras)
    {
        pthread_create(&thr[j], NULL, imprime,  &algo);
        j++;
    }
    fclose(archivo);

    return 0;
    }