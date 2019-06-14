#include <headers.h>
monitorHebra* init_monitorHebra(int tamano,int hebra){
    monitorHebra* mH = (monitorHebra*)calloc(1,sizeof(monitorHebra));
    mH->buffer = (float*)calloc(tamano*3,sizeof(float));
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
    monitorEscritura* mE =(monitorEscritura*)calloc(1,sizeof(monitorEscritura));
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
  

    float media=0.0;
    media = numerosImaginarios / n ;
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
        printf("aca si\n");
        pthread_cond_wait(&monitor->vacioCond, &monitor->vacio);
        
        printf("aca no\n");
    }
    pthread_mutex_lock(&monitor->enUso);
    monitor->buffer[monitor->datosEnBuffer] = dato;
    monitor->datosEnBuffer++;
    monitor->datosLeidos++;
    pthread_mutex_unlock(&monitor->enUso);

}
void crearSalida(char* nombreArchivo, int hijo,float mediaReal,float mediaImaginaria,float potencia, float ruido){
    FILE* salida;
    salida = fopen(nombreArchivo, "a");

    fprintf(salida,"Disco: %i\n",hijo+1);
    fprintf(salida,"Media Real: %f\n",mediaReal);
    fprintf(salida,"Media Imaginaria: %f\n",mediaImaginaria);
    fprintf(salida,"Potencia: %f\n",potencia);
    fprintf(salida,"Ruido: %f\n",ruido);
    
    
    
    
    




    fclose(salida);
    
    
}