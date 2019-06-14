#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

int listo = 1, hola = 1;

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
monitorHebra* init_monitorHebra(int tamano,int hebra){
    monitorHebra* mH = (monitorHebra*)malloc(sizeof(monitorHebra));
    mH->buffer = (float*)malloc(sizeof(float)*tamano*3);
    mH->mediaReal = 0.0;
    mH->tamanoBuffer = tamano;
    mH->mediaImaginaria = 0.0;
    mH->potencia = 0.0;
    mH->ruido = 0.0;
    mH->sumImaginarios = 0.0;
    mH->sumReales = 0.0;
    mH->datosLeidos = 0;
    mH->datosEnBuffer = 0;
    mH->hebra= hebra;
    pthread_mutex_init(&mH->vacio,NULL);
    pthread_mutex_init(&mH->lleno,NULL);
    pthread_cond_init(&mH->llenoCond,NULL);
    pthread_cond_init(&mH->vacioCond,NULL);
    //pthread_cond_wait(&mH->llenoCond, &mH->lleno);
    
    return mH;

}

 
monitorEscritura* init_monitorEscritura (){
    monitorEscritura* mE =(monitorEscritura*)malloc(sizeof(monitorEscritura));
    mE->hebra = 0;
    mE->mediaReal = 0.0;
    mE->mediaImaginaria = 0.0;
    mE->potencia = 0.0;
    mE->ruido = 0.0;
    pthread_mutex_init(&mE->enUso,NULL);
    pthread_cond_init(&mE->enUsoCond,NULL);
    pthread_mutex_init(&mE->enUso,NULL);
    pthread_cond_init(&mE->enUsoCond,NULL);
    
    return mE;
}

monitorEscritura* mE ;

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
float calcularPotencia(float real, float imaginario){

   
    float potencia = sqrtf(powf(real, 2.0) + powf(imaginario, 2.0));
    
    return potencia;
}

//Funcion que se encarga de calcular la distancia para poder enviar los datos al disco correspondiente
float calcularDistancia(char** lista){
    float distancia =  sqrtf( powf(atof(lista[0]),2.0) +  powf(atof(lista[1]),2.0) );
    return distancia;
}

monitorHebra* calculadora(monitorHebra* monitor){
    int i =0;
    monitor->datosLeidos=monitor->datosLeidos + monitor->datosEnBuffer;
    while(i<monitor->datosEnBuffer){
        monitor->sumReales= monitor->sumReales + monitor->buffer[i];
        monitor->sumImaginarios=monitor->sumImaginarios + monitor->buffer[i+1];
        monitor->ruido=monitor->ruido + monitor->buffer[i+2];
        i=i+3;

    }
    monitor->mediaReal=calcularMediaReal(monitor->sumReales,monitor->datosLeidos);
    monitor->mediaImaginaria=calcularMediaImaginaria(monitor->sumImaginarios,monitor->datosLeidos);
    monitor->potencia=calcularPotencia(monitor->sumReales,monitor->sumImaginarios);

    return monitor;
}



void * consumir(void* monitor){
    int i;
    monitorHebra* mH =(monitorHebra*) monitor;
    while (listo)
    {
    
    pthread_cond_wait(&mH->llenoCond, &mH->lleno);
    pthread_mutex_lock(&mH->enUso);
    for (i=0; i<mH->tamanoBuffer*3 ; i = i+3 ){
        if(mH->buffer[i] != 0.0){
            mH->mediaReal += mH->buffer[i];
        }
        if(mH->buffer[i+1] != 0.0){
            mH->mediaImaginaria += mH->buffer[i+1];
        }
        if(mH->buffer[i+2] != 0.0){
            mH->ruido += mH->buffer[i+2];
        }
        float potencia = calcularPotencia(mH->buffer[i],mH->buffer[i+1]);
        mH->potencia += potencia;
        
    }
    mH->buffer = vaciarBuffer(mH->buffer, mH->tamanoBuffer*3 ); 
    mH->datosEnBuffer = 0;
    pthread_mutex_unlock(&mH->enUso);
    pthread_cond_signal(&mH->vacioCond);
    }
    mH->mediaImaginaria = mH->mediaImaginaria / mH->datosLeidos * 3;
    mH->mediaReal = mH->mediaReal/mH->datosLeidos * 3; 
 
    // espero a que se desocupe la struct global
    pthread_mutex_lock(&mE->enUso);
    mE->mediaReal = mH->mediaReal;
    mE->mediaImaginaria = mH->mediaImaginaria;
    mE->potencia = mH->potencia;
    mE->ruido = mH->ruido;
    mE->hebra = mH->hebra;
    pthread_cond_signal(&mE->escribendoArchivoCond);

}

void* producir (monitorHebra* monitor,int tamano, float dato){

    if(monitor->datosEnBuffer == tamano*3 ){
        pthread_cond_signal(&monitor->llenoCond);
        pthread_cond_wait(&monitor->vacioCond, &monitor->vacio);
    }
    pthread_mutex_lock(&monitor->enUso);
    monitor->buffer[monitor->datosEnBuffer] = dato;
    monitor->datosEnBuffer++;
    monitor->datosLeidos++;
    pthread_mutex_unlock(&monitor->enUso);

}




int main(int argc, char const *argv[]){
    int i, numeroDiscos, tamano, radio;
    tamano = 3; 
    radio = 100; 
    float distancia; 
    char buffer [100];
    numeroDiscos = 2;
    monitorHebra** arregloMonitores;
    mE = init_monitorEscritura();
    char* linea= (char*)malloc(sizeof(char)*100);
    char** lista= (char**)malloc(sizeof(char*)*5);
    FILE* archivo;

    arregloMonitores = (monitorHebra**)malloc(sizeof(monitorHebra*)*numeroDiscos);

    for(i = 0; i<numeroDiscos ; i++){
        arregloMonitores[i] = init_monitorHebra(tamano,i);
    }

    pthread_t arregloHebras[numeroDiscos];
    for(i=0;  i< numeroDiscos; i++){
        
        pthread_create(&arregloHebras[i], NULL, consumir,(void*) arregloMonitores[i]);
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
                    producir(arregloMonitores[i],tamano,atof(lista[j]));
                    j++;
                }
                break;
            }
            else if(distancia<max && distancia>=min){
                int j = 2 ;
                while(j<5){
                    producir(arregloMonitores[i],tamano,atof(lista[j]));
                    j++;

                }
                break;
            }
        }

    }
    for(i = 0; i<numeroDiscos; i++){
            pthread_cond_signal( &arregloMonitores[i]->llenoCond);
        }
    listo = 0;
    int numeroHebra = 0;
    while(hola){
        pthread_cond_wait(&mE->escribendoArchivoCond,&mE->escribendoArchivo);
        printf("Hebra %d\n",mE->hebra +1 );
        printf("Media Real: %f\n",mE->mediaReal );
        printf("Media Imaginaria: %f\n", mE->mediaImaginaria);
        printf("Potencia: %f\n",mE->potencia);
        printf("Ruido: %f\n", mE->ruido);
        pthread_mutex_unlock(&mE->enUso);
        numeroHebra++;
        if(numeroHebra == numeroDiscos){
            hola = 0;
        }
    
    }
    
    for(i=0 ; i<numeroDiscos ; i++){
        pthread_join(arregloHebras[i],NULL);    
    } 

    for(i=0; i<numeroDiscos; i++){
        mE->mediaReal = arregloMonitores[i]->mediaReal;
        mE->mediaImaginaria = arregloMonitores[i]->mediaImaginaria;
        mE->potencia = arregloMonitores[i]->potencia;
        mE->ruido = arregloMonitores[i]->ruido;
        mE->hebra = i+1;

        

    }
    


    fclose(archivo);

    return 0;
    }