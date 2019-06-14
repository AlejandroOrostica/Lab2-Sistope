#include <headers.h>
int listo = 1, esperandoHijas = 1;

int main(int argc, char  **argv){
    int pid, radio, numeroDiscos, ppid,opt, flag;
    float distancia;
    flag = 0;
    char* archivoEntrada = (char*)calloc(40,sizeof(char));
    char* archivoSalida = (char*)calloc(40,sizeof(char));
    
    int i,tamano;
 
    
    
    numeroDiscos = 2;
    monitorHebra** arregloMonitores;
    mE = init_monitorEscritura();
    char* linea= (char*)calloc(100,sizeof(char));
    char** lista= (char**)calloc(5,sizeof(char*));
    FILE* archivo;
        while((opt = getopt(argc, argv, "i:o:d:n:s:b")) != -1){
        switch(opt) {
            case 'i':
                
                strcpy(archivoEntrada,optarg);
                archivo=fopen(archivoEntrada,"r");
                if (archivo == NULL)
                        {
                            printf("\nError de apertura del archivo.Puede que el archivo no exista \n\n");
                            exit(-1);
                            
                            
                        }

                        fclose(archivo);  
            break;
            case 'o':
                strcpy(archivoSalida,optarg);
                break;
            case 'n':
                numeroDiscos = atoi(optarg);
                if(numeroDiscos<=0){
                    printf("Error al ingresar el numero de discos, el numero debe ser mayor a 0");
                    exit(-1);
                }
            break;
            case 'd':
                radio = atoi(optarg);
                if(radio<=0){
                    printf("Error al ingresar el tamaño de los radios, el numero debe ser mayor a 0");
                    exit(-1);
                }
                break;
            case 's':
                tamano = atoi(optarg);
                if(tamano<=0){
                    printf("Error al ingresar el tamaño de los buffers, el numero debe ser mayor a 0");
                    exit(-1);
                }
                break;
            case 'b':
                flag = 1;
                break;
        }    

    }


    arregloMonitores = (monitorHebra**)calloc(numeroDiscos,sizeof(monitorHebra*));

    for(i = 0; i<numeroDiscos ; i++){
        arregloMonitores[i] = init_monitorHebra(tamano,i);
    }

    pthread_t arregloHebras[numeroDiscos];
    for(i=0;  i< numeroDiscos; i++){
        
        pthread_create(&arregloHebras[i], NULL, consumir,(void*) arregloMonitores[i]);
    }
    //cambiar por archivoentrada
    archivo = fopen(archivoEntrada,"r");
    while(!feof(archivo)){
        int i=0;
        int pos=0;
        while (i<1){
            char letra=fgetc(archivo);
            
            if(letra=='\n'||letra==EOF){
                
                i++;

            }
            else{
                linea[pos]=letra;
                pos++;
            }
    
        }
        
        lista = procesarLinea(linea,lista);
        
         // Lista = [Vu,Vv,V(u,v).r,V(u,v).i,V(u,v).w]
        distancia = calcularDistancia(lista);
        
        
               
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
    fclose(archivo);
    for(i = 0; i<numeroDiscos; i++){
            pthread_cond_signal( &arregloMonitores[i]->llenoCond);
        }
    listo = 0;
    int numeroHebra = 0;
    while(esperandoHijas){
        pthread_cond_wait(&mE->escribendoArchivoCond,&mE->escribendoArchivo);
        crearSalida(archivoSalida,mE->hebra, mE->mediaReal,mE->mediaImaginaria,mE->potencia,mE->ruido);
        pthread_mutex_unlock(&mE->enUso);
        numeroHebra++;
        if(numeroHebra == numeroDiscos){
            esperandoHijas = 0;
        }
    
    }
    
    for(i=0 ; i<numeroDiscos ; i++){
        pthread_join(arregloHebras[i],NULL);    
    } 
    /*
    for(i=0; i<numeroDiscos; i++){
        mE->mediaReal = arregloMonitores[i]->mediaReal;
        mE->mediaImaginaria = arregloMonitores[i]->mediaImaginaria;
        mE->potencia = arregloMonitores[i]->potencia;
        mE->ruido = arregloMonitores[i]->ruido;
        mE->hebra = i+1;

    } 
    */
 
    

    
    

    return 0;
    }