#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

void crearParticion(char nombreArchivo[], int tamanoreal, char nombre[], char tipo, char fit);
typedef struct
{
    char comando [100];
} Comando;

//variables para montar
int cuantosmontados = 0;
char IDA[6];
typedef struct
{
    int aparicion;
    bool ocupado;
} Posiciones;

typedef struct
{
    char id[10];
} idunmount;

typedef struct
{
    char path[100];
    Posiciones vda[100];
} Montados;

//VECTOR DE ID
Montados Mounts[25];
char letra [25] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

typedef struct
{
    int tamanio;
    int iniciopart;
    char id[10];
    Montados parametros;
    char nombreParticion[25];
} NodoMontados;

NodoMontados YAmontados[100];


typedef struct
{
    char part_status; //Indica si la partición está activa o no
    char part_type; /*Indica el tipo de partición, primaria o extendida.
Tendrá los valores P o E*/
    char part_fit; /*Tipo de ajuste de la partición. Tendrá los valores B (Best), F (First) o
W (worst)*/
    int part_start; //Indica en que byte del disco inicia la partición
    int part_size; // Contiene el tamaño total de la partición en bytes.
    char part_name[16]; // Nombre de la partición
} Particion;


typedef struct
{
    int mbr_tamanio; //Tamaño total del disco en bytes
    char mbr_fecha_creacion [25]; //Fecha y hora de creación del disco
    int mbr_disk_signature; //Número random, que identifica de forma única a cada disco
    /*Particion * mbr_partition_1; //Estructura con información de la partición 1
    Particion * mbr_partition_2; //Estructura con información de la partición 2
    Particion * mbr_partition_3; //Estructura con información de la partición 3
    Particion * mbr_partition_4; // Estructura con información de la partición 4*/
    Particion particiones[4];
} MBR;

typedef struct
{
    char part_status; // Indica si la partición está activa o no
    char part_fit; /*Tipo de ajuste de la partición. Tendrá los valores B (Best), F (First) o
W (worst)*/
    int part_start; //Indica en que byte del disco inicia la partición
    int part_size; // Contiene el tamaño total de la partición en bytes.
    int part_next; // Byte en el que está el próximo EBR. -1 si no hay siguiente
    char part_name [16]; // Nombre de la partición
} EBR;

Comando VComandos[25];

char * retornarfecha()
{
    time_t t;
    struct tm *tm;
    char fechayhora[100];
    t = time(NULL);
    tm = localtime(&t);
    strftime(fechayhora, 100, "%d/%m/%Y %H:%M:%S", tm);
    // printf("%s\n", fechayhora);

    return fechayhora;
}

int contarcaracteres(char pal [])
{
    int i;
    int cantidad = 0;
    for (i = 0; i < strlen(pal); i++)
    {
        if (pal[i] != '\0')
        {
            cantidad++;
        }
        else
        {
        }
    }
    return cantidad;
}


void cambio(char pal[])
{
    int i;
    for (i = 0; i < 200; i++)
    {
        if (pal[i] == '\n' || pal[i] == '\r')
        {
            pal[i] = '\0';
            i = 200;
        }
    }
}

void limpiarcomando()
{
    int i;
    for (i = 0; i < 25; i++)
    {
        strcpy(VComandos[i].comando, "VACIO");
    }
}

void limpiarvariables(char pal[], int intervalo)
{
    int f;
    for (f = 0; f < intervalo; f++)
    {
        pal[f] = '\0';
    }
}

int cuantasEXTENDIDAS(MBR mb)
{
    int extendidas = 0;
    int i;
    for (i = 0; i < 4; i++)
    {
        if ((mb.particiones[i].part_type == 'E' || mb.particiones[i].part_type == 'e') && mb.particiones[i].part_status == '1')
        {
            extendidas = 1;
        }
    }
    return extendidas;
}

int cuantasPRIMARIAS(MBR mb)
{
    int primaria = 0;
    int i;
    for (i = 0; i < 4; i++)
    {
        if ((mb.particiones[i].part_type == 'p' || mb.particiones[i].part_type == 'P') && mb.particiones[i].part_status == '1')
        {
            primaria++;
        }
    }
    return primaria;
}

void crearParticion(char nombreArchivo[], int tamanoreal, char nombre[], char tipo, char fit) {
    bool hayebr = false;
    FILE *disco;
    MBR mbr;
    disco = fopen(nombreArchivo, "rb+");
    if (disco != NULL) { //SI existe el disco
        fseek(disco, 0L, SEEK_SET);
        fread(&mbr, sizeof (MBR), 1, disco);
        int pririas = cuantasPRIMARIAS(mbr);
        int extendidas = cuantasEXTENDIDAS(mbr);
        int totalparticiones = extendidas + pririas;
        int i;
        //LA NUEVA PARTICION QUE SE VA A INSERTAR
        Particion particion;
        particion.part_fit = fit;
        strcpy(particion.part_name, nombre);
        particion.part_size = tamanoreal;
        particion.part_status = '1';
        int logic = 0;
        EBR auxlogicas[50];
        int tamanodisponible;
        bool existe = false;
        /**************validando nombres iguales*/
        int b;
        for (b = 0; b < 4; b++) {
            if (strcmp(mbr.particiones[b].part_name, nombre) == 0) {
                printf("IMPOSIBLE CREAR LA PARTICION %s YA EXISTE UNA CON EL MISMO NOMBRE\n", nombre);
                b = 5;
                existe = true;
            }
        }


        if (extendidas == 1) {
            hayebr = true;
            Particion extendaux;
            int ini;
            for (ini = 0; ini < 4; ini++) {
                if (mbr.particiones[ini].part_type == 'E' || mbr.particiones[ini].part_type == 'e') {
                    extendaux = mbr.particiones[ini];
                    break;
                }
            }
            /*ARREGLO DE EBR*/
            int j;
            EBR ebr;
            fseek(disco, extendaux.part_start, SEEK_SET);
            fread(&ebr, sizeof (EBR), 1, disco);
            j = 0;
            while (j < 50) {
                if (ebr.part_next == -1) {
                    if (ebr.part_status == '1') {
                        if (strcmp(ebr.part_name, nombre) == 0) {
                            printf("YA EXISTE UNA PARTICION CON EL NOMBRE %s", nombre);
                            existe = true;
                        }
                    }
                    break;
                }
                if (ebr.part_status == '1') {
                    if (strcmp(ebr.part_name, nombre) == 0) {
                        printf("YA EXISTE UNA PARTCION CON EL NOMBRE, %s\n", nombre);
                        existe = true;
                        break;
                    }
                }
                fseek(disco, ebr.part_next, SEEK_SET);
                fread(&ebr, sizeof (EBR), 1, disco);
                j++;
            }
        }


        //ORDENAN LAS PARTICIONES DE MAYOR A MENOR MEDIANTE EL BIT DE INICIO PARA VALIDAR LAS FRAGMENTACION
                    Particion auxpart[totalparticiones];
                    int j, p, cont = 0;
                    int espaciolibre;
                    for (j = 0; j < 4; j++) {
                        if (mbr.particiones[j].part_status == '1') {
                            auxpart[cont] = mbr.particiones[j];
                            cont++;
                        }
                    }
                    //PARTICIONES ORDENADAS :D
                    for (j = 0; j < totalparticiones; j++) {
                        for (p = 0; p < totalparticiones - 1; p++) {
                            if (auxpart[p].part_start > auxpart[p + 1].part_start) {
                                Particion aux = auxpart[p];
                                auxpart[p] = auxpart[p + 1];
                                auxpart[p + 1] = aux;
                            }
                        }
                    }

        /***************INICIA
                                CREACION DE PARTICIONES
         ************/
        if (tipo == 'P' || tipo == 'p') {
            particion.part_type = tipo;
            if (pririas < 3) {
                //NO EXISTE NINGUNA PARTICIAN ESTA ES LA PRIMERA
                if (totalparticiones == 0) {
                    tamanodisponible = mbr.mbr_tamanio - (sizeof (MBR) + 1);
                    int i;
                    if (mbr.particiones[0].part_status == '0') {
                        particion.part_start = sizeof (MBR) + 1;
                        particion.part_size = tamanoreal;
                        if (tamanodisponible > tamanoreal) {
                            mbr.particiones[0] = particion;
                            fseek(disco, 0L, SEEK_SET);
                            fwrite(&mbr, sizeof (MBR), 1, disco);
                            printf("SE CREO LA PARTICION %s DE TIPO PRIMARIA\n", nombre);
                        } else {
                            printf("NO HAY ESPACIO PARA CREAR LA PARTICION %s \n", nombre);
                        }
                    }
                } else {

                    if (existe == false) {
                        //SE PUEDE CREAR LA PARTICION NO HAY ERROR CON EL NOMBRE
                        int c;
                        for (c = 1; c < (totalparticiones + 1); c++) {
                            if (c == totalparticiones) {
                                //ESTA SITUADO EN LA ULTIMA PARTICION
                                espaciolibre = mbr.mbr_tamanio - (auxpart[c - 1].part_start + auxpart[c - 1].part_size); //SE LE RESTA EL TAMANIO TOTAL AL TAMANIO DE LA PARTICON ANTERIOR A LA NUEVA
                                if (espaciolibre >= tamanoreal) {
                                    particion.part_start = (auxpart[c - 1].part_start + auxpart[c - 1].part_size + 1);
                                    int i;
                                    for (i = 0; i < 4; i++) {
                                        if (mbr.particiones[i].part_status == '0') {
                                            mbr.particiones[i] = particion;
                                            fseek(disco, 0L, SEEK_SET);
                                            fwrite(&mbr, sizeof (MBR), 1, disco);
                                            printf("SE CREO LA PARTICION %s DE TIPO PRIMARIA\n", nombre);
                                            i = 4;
                                            c = 4;
                                        }
                                    }

                                }
                            } else {
                                //SI ESTA POSICINADO EN UNA PARTICION Y ADELANTE DE ELLA HAY OTRA SE CALCULA ESPACIO LIBRE
                                espaciolibre = auxpart[c].part_start - (auxpart[c - 1].part_start + auxpart[c - 1].part_size + 1);
                                if (espaciolibre >= tamanoreal) {
                                    particion.part_start = auxpart[c - 1].part_start + auxpart[c - 1].part_size + 1;
                                    int i;
                                    for (i = 0; i < 4; i++) {
                                        if (mbr.particiones[i].part_status == '0') {
                                            mbr.particiones[i] = particion;
                                            fseek(disco, 0L, SEEK_SET);
                                            fwrite(&mbr, sizeof (MBR), 1, disco);
                                            printf("SE CREO LA PARTICION %s DE TIPO PRIMARIA\n", nombre);
                                            i = 4;
                                            c = 4;
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            } else {
                printf("ESTE DISCO NO CUENTA CON ESPACIO PARA UNA PARTICION PRIMARIA HA ALCANZADO EL LIMETE DE 3 PRIMARIAS\n");
            }
        } else if (tipo == 'E' || tipo == 'e') {
            particion.part_type = tipo;
            if (extendidas == 0) {
                if (totalparticiones == 0) {
                    //EL DISCO ESTA TOTALMENTE VACIO
                    tamanodisponible = mbr.mbr_tamanio - (sizeof (MBR) + 1);
                    int i;
                    if (mbr.particiones[0].part_status == '0') {
                        particion.part_start = sizeof (MBR) + 1;
                        particion.part_size = tamanoreal;
                        if (tamanodisponible > tamanoreal) {
                            mbr.particiones[0] = particion;
                            fseek(disco, 0L, SEEK_SET);
                            fwrite(&mbr, sizeof (MBR), 1, disco);
                            EBR ebrprimero;
                            ebrprimero.part_fit = 'N';
                            strcpy(ebrprimero.part_name, "N");
                            ebrprimero.part_next = -1;
                            ebrprimero.part_size = sizeof (EBR);
                            ebrprimero.part_start = particion.part_start;
                            ebrprimero.part_status = '0';
                            fseek(disco, particion.part_start, SEEK_SET);
                            fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                            printf("SE CREO LA PARTICION %s DE TIPO EXTENDIDA\n", nombre);
                        } else {
                            printf("NO HAY ESPACIO PARA CREAR LA PARTICION %s \n", nombre);
                        }
                    }
                } else {

                    if (existe == false) {
                        bool inserto = false;
                        //SE PUEDE CREAR LA PARTICION NO HAY ERROR CON EL NOMBRE

                        //EL DISCO YA CONTIENE PARTICIONES
                        int c;
                        for (c = 1; c < (totalparticiones + 1); c++) {
                            if (c == totalparticiones) {
                                //ESTA SITUADO EN LA ULTIMA PARTICION
                                espaciolibre = mbr.mbr_tamanio - (auxpart[c - 1].part_start + auxpart[c - 1].part_size); //SE LE RESTA EL TAMANIO TOTAL AL TAMANIO DE LA PARTICON ANTERIOR A LA NUEVA
                                if (espaciolibre >= tamanoreal) {
                                    particion.part_start = (auxpart[c - 1].part_start + auxpart[c - 1].part_size + 1);
                                    int i;
                                    for (i = 0; i < 4; i++) {
                                        if (mbr.particiones[i].part_status == '0') {
                                            mbr.particiones[i] = particion;
                                            fseek(disco, 0L, SEEK_SET);
                                            fwrite(&mbr, sizeof (MBR), 1, disco);
                                            EBR ebrprimero;
                                            ebrprimero.part_fit = 'N';
                                            strcpy(ebrprimero.part_name, "N");
                                            ebrprimero.part_next = -1;
                                            ebrprimero.part_size = sizeof (EBR);
                                            ebrprimero.part_start = particion.part_start;
                                            ebrprimero.part_status = '0';
                                            fseek(disco, particion.part_start, SEEK_SET);
                                            fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                            printf("SE CREO LA PARTICION %s DE TIPO EXTENDIDA\n", nombre);
                                            i = 4;
                                            c = 4;
                                        }
                                    }

                                } else {
                                    printf("NO HAY ESPACIO PARA CREAR LA PARTICION %s \n", nombre);
                                }
                            } else {
                                //SI ESTA POSICINADO EN UNA PARTICION Y ADELANTE DE ELLA HAY OTRA SE CALCULA ESPACIO LIBRE
                                espaciolibre = auxpart[c].part_start - (auxpart[c - 1].part_start + auxpart[c - 1].part_size + 1);
                                if (espaciolibre >= tamanoreal) {
                                    particion.part_start = auxpart[c - 1].part_start + auxpart[c - 1].part_size + 1;
                                    int i;
                                    for (i = 0; i < 4; i++) {
                                        if (mbr.particiones[i].part_status == '0') {
                                            mbr.particiones[i] = particion;
                                            fseek(disco, 0L, SEEK_SET);
                                            fwrite(&mbr, sizeof (MBR), 1, disco);
                                            EBR ebrprimero;
                                            ebrprimero.part_fit = 'N';
                                            strcpy(ebrprimero.part_name, "N");
                                            ebrprimero.part_next = -1;
                                            ebrprimero.part_size = sizeof (EBR);
                                            ebrprimero.part_start = particion.part_start;
                                            ebrprimero.part_status = '0';
                                            fseek(disco, particion.part_start, SEEK_SET);
                                            fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                            printf("SE CREO LA PARTICION %s DE TIPO EXTENDIDA\n", nombre);
                                            i = 4;
                                            c = 4;
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            } else {
                printf("ESTE DISCO NO CUENTA CON ESPACIO PARA ALMACENAR UNA PARTICION DE TIPO EXTENDIDA ALCANZO LIMITE DE 1 EXTENDIDA\n");
            }

        } else if (tipo == 'L' || tipo == 'l') {
            particion.part_type = tipo;
            EBR ebr;
            Particion extendaux;
            if (extendidas == 1) {
                for (i = 0; i < 4; i++) {
                    if (mbr.particiones[i].part_type == 'E' || mbr.particiones[i].part_type == 'e') {
                        extendaux = mbr.particiones[i];
                        break;
                    }
                }
                if (tamanoreal <= extendaux.part_size) {
                    if (existe == false) {
                        //PARTICION QUE VA A SUSTITUIR
                        EBR veclogicas[50];
                        int logic = 0;
                        int j;
                        EBR ebr;

                        fseek(disco, extendaux.part_start, SEEK_SET);
                        fread(&ebr, sizeof (EBR), 1, disco);
                        j = 0;
                        while (j < 50) {
                            if (ebr.part_next == -1) {
                                if (ebr.part_status == '1') {
                                    veclogicas[logic] = ebr;
                                    logic++;
                                }
                                break;
                            }
                            if (ebr.part_status == '1') {
                                veclogicas[logic] = ebr;
                                logic++;

                            } else if (ebr.part_status == '0' && ebr.part_start == extendaux.part_start) {
                                //ES LA PRIMERA PARTICION PERO ESTA "ELIMINADA"
                                veclogicas[logic] = ebr;
                                logic++;

                            }
                            fseek(disco, ebr.part_next, SEEK_SET);
                            fread(&ebr, sizeof (EBR), 1, disco);
                            j++;
                        }
                        int logicas = logic - 1;
                        j = 0;
                        bool inserto = false;

                        EBR final, siguiente, anterior, actual;

                        //SI LA PRIMERA PARTICION EXISTE PERO ESTA INACTIVA
                        fseek(disco, extendaux.part_start, SEEK_SET);
                        fread(&actual, sizeof (EBR), 1, disco);
                        if (actual.part_status == '0' && actual.part_start == extendaux.part_start) {
                            EBR ebrprimero;
                            fseek(disco, extendaux.part_start, SEEK_SET);
                            fread(&actual, sizeof (EBR), 1, disco);
                            if (actual.part_next == -1) {
                                //SOLO EXISTE LA LOGICA POR DEFAUL, TAMANIO DE LA EXT - TAML +SIZEEBR
                                if (extendaux.part_size > (tamanoreal - sizeof (EBR))) {
                                    ebrprimero.part_fit = 'N';
                                    strcpy(ebrprimero.part_name, "N");
                                    ebrprimero.part_next = -1;
                                    ebrprimero.part_size = sizeof (EBR);
                                    actual.part_size = tamanoreal + sizeof (EBR);
                                    actual.part_start = extendaux.part_start;
                                    ebrprimero.part_start = (actual.part_start + actual.part_size + 1);
                                    ebrprimero.part_status = '0';
                                    actual.part_fit = fit;
                                    strcpy(actual.part_name, nombre);
                                    actual.part_status = '1';
                                    actual.part_next = ebrprimero.part_start; //la siguiente de la que se esta analizando esta donde ella misma termina
                                    fseek(disco, ebrprimero.part_start, SEEK_SET);
                                    fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                    fseek(disco, actual.part_start, SEEK_SET);
                                    fwrite(&actual, sizeof (EBR), 1, disco);
                                    printf("SE CREO LA PARTICION  %s CORRECTAMENTE \n", nombre);
                                    inserto = true;
                                } else {
                                    printf("NO HAY ESPACIO PARA CREAR ESTA PARTICION\n");
                                }

                            } else if (actual.part_next != -1) {
                                EBR ver = veclogicas[logicas];
                                //SI HAY ESPACIO EN MEDIO DE DOS PARTICIONES
                                int espacio = actual.part_next - (actual.part_start + 1);
                                if (espacio > tamanoreal - sizeof (EBR)) {
                                    actual.part_fit = fit;
                                    strcpy(actual.part_name, nombre);
                                    actual.part_size = tamanoreal + sizeof (ebr);
                                    actual.part_status = '1';
                                    fseek(disco, actual.part_start, SEEK_SET);
                                    fwrite(&actual, sizeof (EBR), 1, disco);
                                    printf("SE CREO LA PARTICION  %s CORRECTAMENTE  AL INICIO DEL DISCO LA PARTICION ESTA INACTIVA\n", nombre);
                                    inserto = true;

                                }
                            }
                        }



                        //SI ES LA PRIMER PARTICION
                        if (logic == 0 && inserto == false) {
                            EBR ebrprimero;
                            fseek(disco, extendaux.part_start, SEEK_SET);
                            fread(&actual, sizeof (EBR), 1, disco);
                            if (actual.part_next == -1) {
                                //SOLO EXISTE LA LOGICA POR DEFAUL, TAMANIO DE LA EXT - TAML +SIZEEBR
                                if (extendaux.part_size > (tamanoreal - sizeof (EBR))) {
                                    ebrprimero.part_fit = 'N';
                                    strcpy(ebrprimero.part_name, "N");
                                    ebrprimero.part_next = -1;
                                    ebrprimero.part_size = sizeof (EBR);
                                    actual.part_size = tamanoreal + sizeof (EBR);
                                    actual.part_start = extendaux.part_start;
                                    ebrprimero.part_start = (actual.part_start + actual.part_size + 1);
                                    ebrprimero.part_status = '0';
                                    actual.part_fit = fit;
                                    strcpy(actual.part_name, nombre);
                                    actual.part_status = '1';
                                    actual.part_next = ebrprimero.part_start; //la siguiente de la que se esta analizando esta donde ella misma termina
                                    fseek(disco, ebrprimero.part_start, SEEK_SET);
                                    fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                    fseek(disco, actual.part_start, SEEK_SET);
                                    fwrite(&actual, sizeof (EBR), 1, disco);
                                    printf("SE CREO LA PARTICION  %s CORRECTAMENTE \n", nombre);
                                    inserto = true;
                                } else {
                                    printf("NO HAY ESPACIO PARA CREAR ESTA PARTICION\n");
                                }

                            }

                        } else {
                            while (logicas >= 0) {


                                if (veclogicas[logicas].part_next != -1 && logic > 0)
                                    //CUANTO ADELANTE DE EL NO ESTA LA ULTIMA PARTICION
                                    if (logic > 0 && logicas <= (logic - 1)) {
                                        actual = veclogicas[logicas];
                                        fseek(disco, actual.part_next, SEEK_SET);
                                        fread(&siguiente, sizeof (EBR), 1, disco);
                                        int libre = (actual.part_size + actual.part_start + 1) - siguiente.part_start;
                                        if (libre > 0) {
                                            if (libre > tamanoreal - sizeof (EBR)) {
                                                EBR ebrprimero;
                                                ebrprimero.part_fit = fit;
                                                strcpy(ebrprimero.part_name, nombre);
                                                ebrprimero.part_next = siguiente.part_start;
                                                ebrprimero.part_size = tamanoreal + sizeof (EBR);
                                                ebrprimero.part_start = (actual.part_start + actual.part_size + 1);
                                                ebrprimero.part_status = '1';
                                                actual.part_next = ebrprimero.part_start;
                                                fseek(disco, actual.part_start, SEEK_SET);
                                                fwrite(&actual, sizeof (EBR), 1, disco);
                                                fseek(disco, ebrprimero.part_start, SEEK_SET);
                                                fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                                fseek(disco, siguiente.part_start, SEEK_SET);
                                                fwrite(&siguiente, sizeof (EBR), 1, disco);
                                                printf("SE CREO LA PARTICION %s CORRECTAMENTE AL INICIO DE LA PARTICION\n", nombre);
                                                inserto = true;
                                                break;

                                            }
                                        } else {
                                            int limit = 0;
                                            int index = logicas;
                                            while (limit <= logicas) {
                                                actual = veclogicas[limit];
                                                fseek(disco, actual.part_next, SEEK_SET);
                                                fread(&siguiente, sizeof (EBR), 1, disco);
                                                if (libre > 0) {
                                                    //SI HAY ESPACIO ENTRE PARTICIONES
                                                    if (libre > tamanoreal - sizeof (EBR)) {
                                                        EBR ebrprimero;
                                                        ebrprimero.part_fit = fit;
                                                        strcpy(ebrprimero.part_name, nombre);
                                                        ebrprimero.part_next = siguiente.part_start;
                                                        ebrprimero.part_size = tamanoreal + sizeof (EBR);
                                                        ebrprimero.part_start = (actual.part_start + actual.part_size + 1);
                                                        ebrprimero.part_status = '1';
                                                        actual.part_next = ebrprimero.part_start;
                                                        fseek(disco, actual.part_start, SEEK_SET);
                                                        fwrite(&actual, sizeof (EBR), 1, disco);
                                                        fseek(disco, ebrprimero.part_start, SEEK_SET);
                                                        fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                                        fseek(disco, siguiente.part_start, SEEK_SET);
                                                        fwrite(&siguiente, sizeof (EBR), 1, disco);
                                                        printf("SE CREO LA PARTICION %s CORRECTAMENTE ENTRE LA PARTICION %s Y %s \n", nombre, actual.part_name, siguiente.part_name);
                                                        inserto = true;
                                                        break;
                                                    }

                                                } else {
                                                    actual = siguiente;
                                                    if (siguiente.part_next != -1) {
                                                        fseek(disco, siguiente.part_next, SEEK_SET);
                                                        fread(&siguiente, sizeof (EBR), 1, disco);
                                                        libre = siguiente.part_start - (actual.part_size + actual.part_start + 1);

                                                    } else {
                                                        break;
                                                    }

                                                }
                                                index--;
                                                limit++;
                                            }
                                            if (inserto == false) {
                                                int librefinal = (extendaux.part_size + extendaux.part_start) - (siguiente.part_start + sizeof (EBR));
                                                if (librefinal > tamanoreal - sizeof (EBR)) {
                                                    EBR ebrprimero;
                                                    ebrprimero.part_fit = 'N';
                                                    strcpy(ebrprimero.part_name, "N");
                                                    ebrprimero.part_next = -1;
                                                    ebrprimero.part_size = sizeof (EBR);
                                                    siguiente.part_size = tamanoreal + sizeof (EBR);
                                                    ebrprimero.part_start = (siguiente.part_start + siguiente.part_size + 1);
                                                    ebrprimero.part_status = '0';
                                                    siguiente.part_fit = fit;
                                                    strcpy(siguiente.part_name, nombre);
                                                    siguiente.part_status = '1';
                                                    siguiente.part_next = ebrprimero.part_start; //la siguiente de la que se esta analizando esta donde ella misma termina
                                                    fseek(disco, ebrprimero.part_start, SEEK_SET);
                                                    fwrite(&ebrprimero, sizeof (EBR), 1, disco);
                                                    fseek(disco, siguiente.part_start, SEEK_SET);
                                                    fwrite(&siguiente, sizeof (EBR), 1, disco);
                                                    printf("SE CREO LA PARTICION %s CORRECTAMENTE AL FINAL DEL DISCO \n", nombre);
                                                    break;
                                                } else {
                                                    printf("NO EXISTE ESPACIO PARA ESTA PARTICION\n");
                                                    inserto = true;
                                                    break;

                                                }
                                            }
                                        }
                                    }
                                if (inserto == true) {
                                    break;
                                } else {
                                    logicas--;

                                }

                            }
                        }

                    }
                } else {
                    printf("TAMANO INCORRECTO ES MAS GRANDE EL TAMANO DE LA LOGICA \n");
                }
            } else {
                printf("NO SE CUENTA CON UNA PARTICION EXTENDIDA PARA ALMACENAR LA PARTICION LOGICA\n");
            }
        }
        fclose(disco);
    } else {
        printf("NO EXISTE EL DISCO\n");
    }

}

void eliminarParticion(char nombreArchivo [], char delete [], char nombre[]) {
    FILE * ar;
    bool encontro = false;
    ar = fopen(nombreArchivo, "rb+");
    if (ar != NULL) {
        MBR mb;
        fread(&mb, sizeof (MBR), 1, ar);
        int i;
        int totalparticiones = cuantasEXTENDIDAS(mb) + cuantasPRIMARIAS(mb);
        Particion auxpart[totalparticiones];
        //VECTOR DE PARTIICONES ACTIVAS
        int j, p, cont = 0;
        for (j = 0; j < 4; j++) {
            if (mb.particiones[j].part_status == '1') {
                auxpart[cont] = mb.particiones[j];
                cont++;
            }
        }
        //ORDENANDO PARTICIONES
        for (j = 0; j < totalparticiones; j++) {
            for (p = 0; p < totalparticiones - 1; p++) {
                if (auxpart[p].part_start > auxpart[p + 1].part_start) {
                    Particion aux = auxpart[p];
                    auxpart[p] = auxpart[p + 1];
                    auxpart[p + 1] = aux;
                }
            }
        }


        for (i = 0; i < 4; i++) {
            if (strcasecmp(auxpart[i].part_name, nombre) == 0) {
                int totalpar = (cuantasEXTENDIDAS(mb) + cuantasPRIMARIAS(mb)) - 1;
                if (strcasecmp(delete, "full") == 0) {
                    //PARTICION AUXILIAR QUE SE VA A SOBREESCRIBIR A LA QUE SE ELIMINARA
                    Particion particion;
                    particion.part_fit = 'N';
                    particion.part_status = '0';
                    particion.part_start = 0;
                    strcpy(particion.part_name, "\0");
                    particion.part_size = 0;
                    particion.part_type = 'N';
                    char relleno = '\0';
                    if (i == totalpar) {
                        //ES LA ULTIMA PARTICION
                        int a;
                        for (a = auxpart[i].part_start; a < mb.mbr_tamanio; a++) {
                            fseek(ar, a, SEEK_SET);
                            fwrite(&relleno, sizeof (char), 1, ar);
                        }
                    } else if (i < totalpar) {
                        //HAY UNA PARTICION ADELANTE DE ELLA
                        int a;
                        for (a = auxpart[i].part_start; a < auxpart[i + 1].part_start; a++) {
                            fseek(ar, a, SEEK_SET);
                            fwrite(&relleno, sizeof (char), 1, ar);
                        }
                    }
                    int j;
                    for (j = 0; j < 4; j++) {
                        if (strcasecmp(auxpart[i].part_name, mb.particiones[j].part_name) == 0) {
                            mb.particiones[j] = particion;
                            j = 4;
                        }
                    }
                    fseek(ar, 0L, SEEK_SET);
                    fwrite(&mb, sizeof (MBR), 1, ar);
                    i = 4;
                    encontro = true;
                    printf("SE ELIMINO LA PARTICION CON NOMBRE: %s CON FORMATO FULL\n", nombre);

                } else if (strcasecmp(delete, "fast") == 0) {
                    //PARTICION AUXILIAR QUE SE SOBREESCRIBIRA SOBRE LA QUE SE ELIMINARA
                    Particion particion;
                    particion.part_fit = 'N';
                    particion.part_status = '0';
                    particion.part_start = mb.particiones[i].part_start;
                    strcpy(particion.part_name, "\0");
                    particion.part_size = mb.particiones[i].part_size;
                    particion.part_type = 'N';
                    int j;
                    for (j = 0; j < 4; j++) {
                        if (strcasecmp(auxpart[i].part_name, mb.particiones[j].part_name) == 0) {
                            mb.particiones[j] = particion;
                        }
                    }
                    fseek(ar, 0L, SEEK_SET);
                    fwrite(&mb, sizeof (MBR), 1, ar);
                    printf("SE ELIMINO LA PARTICION CON NOMBRE: %s CON FORMATO FAST\n", nombre);
                    i = 4;
                    encontro = true;
                }
            }

/*************SI LA PARTICION ES UNA LOGICA*/
            else if (auxpart[i].part_type == 'E' || auxpart[i].part_type == 'e') {
                //PARTICION QUE VA A SUSTITUIR
                EBR veclogicas[50];
                int logic = 0;
                int j;
                EBR ebr;

                fseek(ar, auxpart[i].part_start, SEEK_SET);
                fread(&ebr, sizeof (EBR), 1, ar);
                j = 0;
                while (j < 50) {
                    if (ebr.part_next == -1) {
                        if (ebr.part_status == '1') {
                            veclogicas[logic] = ebr;
                            logic++;
                        }
                        break;
                    }
                    if (ebr.part_status == '1') {
                        veclogicas[logic] = ebr;
                        logic++;

                    }
                    fseek(ar, ebr.part_next, SEEK_SET);
                    fread(&ebr, sizeof (EBR), 1, ar);
                    j++;
                }
                int logicas = logic - 1;
                j = logic - 1;
                EBR final, siguiente, anterior, actual;
                while (logicas >= 0) {
                    if (strcmp(veclogicas[logicas].part_name, nombre) == 0) {
                        //SI ES LA PRIMER PARTICION
                        if (auxpart[i].part_start == veclogicas[logicas].part_start) {
                            //printf("NO SE PUEDE ELIMINAR LA PRIMER PARTICION DE LA EXTENDIDA\n");
                            fseek(ar, veclogicas[logicas].part_start, SEEK_SET);
                            fread(&actual, sizeof (EBR), 1, ar);
                            actual.part_status = '0';
                            fseek(ar, veclogicas[logicas].part_start, SEEK_SET);
                            fwrite(&actual, sizeof (EBR), 1, ar);
                            fseek(ar, 0L, SEEK_SET);
                            fwrite(&mb, sizeof (MBR), 1, ar);

                            printf("SE HA ELIMINADO LA PRIMERA PARTICION LOGICA CON NOMBRE %s\n", nombre);
                            i = 4;
                            encontro = true;
                            break;
                        }//SI ESTA EN MEDIO DE DOS PARTICIONES
                        else if (veclogicas[logicas].part_next != -1 && logicas > 0) {
                            //SI ESTA ATRAS DEL EBR NULO
                            if (logicas == (logic - 1)) {
                                actual = veclogicas[logicas];
                                anterior = veclogicas[logicas - 1];
                                fseek(ar, veclogicas[logicas].part_next, SEEK_SET);
                                fread(&final, sizeof (EBR), 1, ar);
                                if (final.part_next == -1) {
                                    anterior.part_next = final.part_start;
                                    if (strcasecmp(delete, "full") == 0) {
                                        char relleno = '\0';
                                        int fin = actual.part_start + actual.part_size;
                                        int step;
                                        for (step = actual.part_start; step < fin; step++) {
                                            fseek(ar, step, SEEK_SET);
                                            fwrite(&relleno, sizeof (char), 1, ar);
                                        }
                                        fseek(ar, anterior.part_start, SEEK_SET);
                                        fwrite(&anterior, sizeof (EBR), 1, ar);
                                        printf("SE ELIMINO CORRECTAMENTE LA PARTICION %s CON FORMATO FULL", nombre);
                                        i = 4;
                                        encontro = true;
                                        break;
                                    } else if (strcasecmp(delete, "fast") == 0) {
                                        anterior.part_next = final.part_start;
                                        actual.part_status = '0';
                                        fseek(ar, anterior.part_start, SEEK_SET);
                                        fwrite(&anterior, sizeof (EBR), 1, ar);
                                        fseek(ar, actual.part_start, SEEK_SET);
                                        fwrite(&actual, sizeof (EBR), 1, ar);
                                        printf("SE ELIMINO CORRECTAMENTE LA PARTICION %s CON FORMATO FAST", nombre);
                                        i = 4;
                                        encontro = true;
                                        break;
                                    }
                                }
                            }//CUANTO ADELANTE DE EL NO ESTA LA ULTIMA PARTICION
                            else if (logicas > 0 && logicas < (logic - 1)) {
                                anterior = veclogicas[logicas - 1];
                                actual = veclogicas[logicas];
                                siguiente = veclogicas[logicas + 1];
                                anterior.part_next = siguiente.part_start;
                                actual.part_status = '0';
                                if (strcasecmp(delete, "full") == 0) {
                                    char relleno = '\0';
                                    int fin = actual.part_start + actual.part_size;
                                    int step;
                                    for (step = actual.part_start; step < fin; step++) {
                                        fseek(ar, step, SEEK_SET);
                                        fwrite(&relleno, sizeof (char), 1, ar);
                                    }
                                    fseek(ar, anterior.part_start, SEEK_SET);
                                    fwrite(&anterior, sizeof (EBR), 1, ar);
                                    fseek(ar, siguiente.part_start, SEEK_SET);
                                    fwrite(&siguiente, sizeof (EBR), 1, ar);
                                    printf("SE ELIMINO CORRECTAMENTE LA PARTICION %s CON FORMATO FULL", nombre);
                                    encontro = true;
                                    i = 4;
                                    break;
                                } else if (strcasecmp(delete, "fast") == 0) {
                                    fseek(ar, anterior.part_start, SEEK_SET);
                                    fwrite(&anterior, sizeof (EBR), 1, ar);
                                    fseek(ar, siguiente.part_start, SEEK_SET);
                                    fwrite(&siguiente, sizeof (EBR), 1, ar);
                                    printf("SE ELIMINO CORRECTAMENTE LA PARTICION %s CON FORMATO FULL", nombre);
                                    encontro = true;
                                    i = 4;
                                    break;
                                }
                            }
                        }
                    }
                    logicas--;
                }
            }
        }
        if (encontro == false) {
            printf("LA PARTICION CON NOMBRE %s NO SE ENCUENTRA EN ESTE DISCO \n", nombre);
        }
        fclose(ar);
    } else {
        printf("NO EXISTE LA RUTA DEL ARCHIVO\n");
    }

}


void fdisk(Comando cmd[])
{
    bool error = false;
    bool unit = false;
    bool type = false;
    bool fit = false;
    bool delete = false;
    bool add = false;
    bool name = false;
    bool size = false;
    bool path = false;
    bool ejecuto = false;
    char uni;
    char adjust;
    char typec;
    char unidad [2];
    char tipo[5];
    char fitt[5];
    char deletee[5];
    int agregar;
    char nombre[100];
    int tamanio;
    char pat[100];
    bool existeunidad = false;
    char auxpath[100];
    char auxnombre[100];
    limpiarvariables(auxnombre, 100);
    limpiarvariables(auxpath, 100);
    limpiarvariables(deletee, 5);
    limpiarvariables(nombre, 100);
    limpiarvariables(pat, 100);
    limpiarvariables(unidad, 2);
    limpiarvariables(tipo, 5);
    limpiarvariables(fitt, 5);
    int i = 0;

    while (strcasecmp(cmd[i].comando, "vacio") != 0)
    {
        if (strcasecmp(cmd[i].comando, "name") == 0 || strcasecmp(cmd[i].comando, "-name") == 0)
        {
            name = true;
            strcpy(nombre, cmd[i + 1].comando);
            //printf("EL NOMBRE DE LA PARTICION SERA:%s\n", nombre);

            if(nombre[0]=='\"')
            {
                int cantidad = contarcaracteres(nombre);
                if(nombre[cantidad-1]=='\"')
                {
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxnombre, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (nombre[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxnombre[cont] = nombre[l];
                            cont++;
                        }
                    }
                    printf("EL NOMBRE DEL DISCO ES:%s\n", auxnombre);
                }
                else
                {
                    //TIENE ESPACIOS EN BLANCO
                    int a;
                    //CONCATENANDO TODA LA PATH COMPLETA
                    for (a = i + 2; a < 25; a++)
                    {
                        int b = contarcaracteres(auxnombre);
                        if (auxnombre[b - 1] != '\"')
                        {
                            strcat(nombre, " ");
                            strcat(nombre, auxnombre);
                        }
                        else
                        {
                            strcat(nombre, " ");
                            strcat(nombre, auxnombre);
                            a = 25;
                        }
                    }
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxnombre, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (nombre[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxnombre[cont] = nombre[l];
                            cont++;
                        }
                    }
                    printf("EL NOMBRE DE LA PARTICION SERA:%s\n", auxnombre);
                }
            }
            else
            {
                strcpy(auxnombre,nombre);
            }
        }
        else if (strcasecmp(cmd[i].comando, "unit") == 0 || strcasecmp(cmd[i].comando, "+unit") == 0)
        {
            unit = true;
            strcpy(unidad, cmd[i + 1].comando);
            existeunidad = true;
            printf("LA UNIDAD DEL DISCO SERA:%s\n", unidad);
            if (strcasecmp(unidad, "k") == 0)
            {
                uni = 'K';
            }
            else if (strcasecmp(unidad, "m") == 0)
            {
                uni = 'M';
            }
            else if (strcasecmp(unidad, "b") == 0)
            {
                uni = 'B';
            }
            else
            {
                printf("ERROR LA UNIDAD ES %s INCORRECTA SOLO ADMITE [M|B|K]\n", unidad);
                error = true;
            }
        }
        else if (strcasecmp(cmd[i].comando, "type") == 0 || strcasecmp(cmd[i].comando, "+type") == 0)
        {
            type = true;
            strcpy(tipo, cmd[i + 1].comando);
            printf("EL TIPO DE PARTICION SERA:%s\n", tipo);
            if (strcasecmp(tipo, "p") == 0)
            {
                typec = 'P';
            }
            else if (strcasecmp(tipo, "l") == 0)
            {
                typec = 'L';
            }
            else if (strcasecmp(tipo, "e") == 0)
            {
                typec = 'E';
            }
            else
            {
                error = true;
                printf("TIPO DE PARTICION NO VALIDA\n");
            }
        }
        else if (strcasecmp(cmd[i].comando, "fit") == 0 || strcasecmp(cmd[i].comando, "+fit") == 0)
        {
            fit = true;
            strcpy(fitt, cmd[i + 1].comando);
            printf("EL NOMBRE DE LA PARTICION SERA:%s\n", fitt);
            if (strcasecmp(fitt, "w") == 0 || strcasecmp(fitt, "wf") == 0)
            {
                adjust = 'W';
            }
            else if (strcasecmp(fitt, "b") == 0 || strcasecmp(fitt, "bf") == 0)
            {
                adjust = 'B';
            }
            else if (strcasecmp(fitt, "f") == 0 || strcasecmp(fitt, "ff") == 0)
            {
                adjust = 'F';
            }
            else
            {
                printf("ERROR EL AJUSTE %s NO ES VALIDO [B-W-F]\n", fitt);
                error = true;
            }
        }
        else if (strcasecmp(cmd[i].comando, "delete") == 0 || strcasecmp(cmd[i].comando, "+delete") == 0)
        {
            delete = true;
            strcpy(deletee, cmd[i + 1].comando);
            printf("LA PARTICION A ELIMINAR SERA:%s\n", deletee);
        }
        else if (strcasecmp(cmd[i].comando, "add") == 0 || strcasecmp(cmd[i].comando, "+add") == 0)
        {
            add = true;
            agregar = atoi(cmd[i + 1].comando);
            printf("EL ESPACIO A AGREGAR SERA:%d\n", agregar);
        }
        else if (strcasecmp(cmd[i].comando, "size") == 0 || strcasecmp(cmd[i].comando, "-size") == 0)
        {
            tamanio = atoi(cmd[i + 1].comando);
            if (tamanio > 0)
            {
                size = true;
                printf("EL TAMANIO DE LA PARTICION SERA:%d\n", tamanio);
            }
            else
            {
                error = true;
                printf("NO SE ADMITE VALORES NEGATIVOS\n");
            }
        }
        else if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0)
        {
            path = true;
            strcpy(pat, cmd[i + 1].comando);
            //printf("LA PATH DEL DISCO SERA: %s \n", pat);
            limpiarvariables(auxpath, 100);
            if (pat[0] == '\"')
            {
                int cantidad = contarcaracteres(pat);
                if(pat[cantidad-1]=='\"')
                {
                    /**********QUITNADO LAS COMILLAS************/
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (pat[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxpath[cont] = pat[l];
                            cont++;
                        }
                    }
                    printf("LA PATH DEL DISCO ES:%s\n", auxpath);
                }
                else
                {
                    int a;
                    //CONCATENANDO TODA LA PATH COMPLETA
                    for (a = i + 2; a < 25; a++)
                    {
                        strcpy(auxpath, cmd[a].comando);
                        int b = contarcaracteres(auxpath);
                        if (auxpath[b - 1] != '\"')
                        {
                            if (strcasecmp(auxpath, "vacio") == 0)
                            {
                                a = 25;
                            }
                            else
                            {
                                strcat(pat, " ");
                                strcat(pat, auxpath);
                            }
                        }
                        else
                        {
                            strcat(pat, " ");
                            strcat(pat, auxpath);
                            a = 25;
                        }
                    }
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxpath, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (pat[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxpath[cont] = pat[l];
                            cont++;
                        }
                    }
                    printf("LA PATH DEL DISCO ES:%s\n", auxpath);
                }

            }
            else
            {
                strcpy(auxpath, pat);
                printf("LA PATH DEL DISCO ES:%s\n", auxpath);

            }
        }
        i++;
    }

    /*********************CREANDO PARTICIONES*************/

    if (size == true && path == true && name == true && error == false && ejecuto == false)
    {
        ejecuto = true;
        int tamanoreal;
        if (existeunidad == false)
        {
            uni = 'K';
        }
        FILE * archivo;
        Particion * nueva = malloc(sizeof (Particion));
        int final, comienzo;
        strcpy(nueva->part_name, nombre);
        nueva->part_size = tamanio;
        char * cadena = ("\"%s\"", auxpath);
        archivo = fopen(cadena, "rb+");
        MBR * mb = malloc(sizeof (MBR));
        if (archivo != NULL)
        {
            if ((comienzo = ftell(archivo)) < 0)
            {
                printf("ERROR: ftell no ha funcionado\n");
            }
            else
            {
                if (type == false)
                {
                    typec = 'P';
                }
                if (fit == false)
                {
                    adjust = 'W';
                }
                if (unit == false)
                {
                    uni = 'K';
                    tamanoreal = tamanio * 1024;
                }
                else
                {
                    if (uni == 'K' || uni == 'k')
                    {
                        tamanoreal = tamanio * 1024;
                    }
                    else if (uni == 'B' || uni == 'b')
                    {
                        tamanoreal = tamanio;
                    }
                    else if (uni == 'M' || uni == 'm')
                    {
                        tamanoreal = (tamanio * 1024)*1024;
                    }

                }
                if(tamanoreal>=(2*1024*1024))
                {
                    crearParticion(cadena, tamanoreal, auxnombre, typec, adjust);
                }
                else
                {
                    printf("ERROR DE TAMANIO, TIENE QUE TENER POR LO MENOS 2 MB\n");
                }

            }
        }
        else
        {
            printf("ERROR EL DISCO NO EXISTE\n");
        }
    }
    else
    {
        if (delete == true && name == true && path == true && error == false)
        {

        }
        else
        {
            if (path == true && name == true && add == true && error == false)
            {

            }
            else
            {
                printf("FALTA PARAMETRO SIZE, PATH O NAME - IMPOSIBLE CREAR PARTICION\n");

            }
        }
    }


    /************************ELIMINAR PARTICION***********************************/
    if (ejecuto == false)
    {
        if (delete == true && name == true && path == true && error == false)
        {
            //METODO PARA ELIMINAR PARTICINES
            ejecuto = true;
        }
        else
        {
            if (path == true && name == true && add == true && error == false)
            {

            }
            else
            {
                printf("IMPOSIBLE ELIMINAR LA PARTICION FALTA PARAMETRO PATH, DELETE O NOMBRE DE LA PARTICION\n");

            }

        }
    }
    /**************************AGREGAR ESPACIO************************************/
    if (ejecuto == false)
    {
        if (path == true && name == true && add == true && error == false)
        {
            int valorreal;
            if (unit == false)
            {
                uni = 'K';
                valorreal = agregar * 1024;
            }
            else
            {

                if (uni == 'b' || uni == 'B')
                {
                    valorreal = agregar;
                }
                else if (uni == 'm' || uni == 'M')
                {
                    valorreal = (agregar * 1024)*1024;
                }
                else if (uni == 'k' || uni == 'K')
                {
                    valorreal = (agregar * 1024);
                }
            }
            //METODO PARA AGREGAR ESPACIO
        }
        else
        {
            printf("ERROR FALTA PARAMETRO [NAME-PATH-ADD]\n");
        }
    }

}


bool yafuemontado(char path[], char nombre[])
{
    int k;
    for (k = 0; k < 100; k++)
    {
        if (strcasecmp(YAmontados[k].nombreParticion, nombre) == 0 && strcasecmp(YAmontados[k].parametros.path, path) == 0)
        {
            limpiarvariables(IDA, 6);
            strcpy(IDA, YAmontados[k].id);
            return true;
        }
    }
    return false;
}

void mount(Comando cmd[])
{
    bool name = false;
    bool pat = false;
    char auxpath[100];
    char path[100];
    char nombre[25];
    limpiarvariables(auxpath, 100);
    limpiarvariables(path, 100);
    limpiarvariables(nombre, 25);
    int i = 0;
    while (strcasecmp(cmd[i].comando, "vacio") != 0)
    {
        if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0)
        {
            pat = true;
            strcpy(path, cmd[i + 1].comando);
            printf("LA PATH DEL DISCO SERA: %s \n", path);
            limpiarvariables(auxpath, 100);
            if (path[0] == '\"')
            {
                int a;
                //CONCATENANDO TODA LA PATH COMPLETA
                for (a = i + 2; a < 25; a++)
                {
                    strcpy(auxpath, cmd[a].comando);
                    int b = contarcaracteres(auxpath);
                    if (auxpath[b - 1] != '\"')
                    {
                        if (strcasecmp(auxpath, "vacio") == 0)
                        {
                            a = 25;
                        }
                        else
                        {
                            strcat(path, " ");
                            strcat(path, auxpath);
                        }
                    }
                    else
                    {
                        strcat(path, " ");
                        strcat(path, auxpath);
                        a = 25;
                        //limpiarvariables(auxpath, 100);
                    }
                }
                /**********QUITNADO LAS COMILLAS************/
                limpiarvariables(auxpath, 100);
                int l;
                int cont = 0;
                for (l = 1; l < 100; l++)
                {
                    if (path[l] == '\"')
                    {
                        l = 100;
                    }
                    else
                    {
                        auxpath[cont] = path[l];
                        cont++;
                    }
                }
                printf("LA CARPETA COMPLETA ES:%s\n", auxpath);
            }
            else
            {
                strcpy(auxpath, path);
            }
        }
        else if (strcasecmp(cmd[i].comando, "name") == 0 || strcasecmp(cmd[i].comando, "-name") == 0)
        {
            name = true;
            strcpy(nombre, cmd[i + 1].comando);
            printf("EL NOMBRE PARA MONTAR SERA:%s\n", nombre);
        }
        i++;
    }
    if (name == true && pat == true)
    {
        FILE * ar;
        MBR mb;
        ar = fopen(auxpath, "rb+");
        bool montado = false;
        if (ar != NULL)
        {
            bool encontro = false;
            fseek(ar, 0L, SEEK_SET);
            fread(&mb, sizeof (MBR), 1, ar);
            int j;
            for (j = 0; j < 4; j++)
            {
                if (strcmp(mb.particiones[j].part_name, nombre) == 0)
                {
                    char str[10];
                    limpiarvariables(str, 10);
                    int k;
                    //CANTIDAD DE DISCOS CON UN MAXIMO DE 25 DISCOS
                    if (yafuemontado(auxpath, nombre) == true)
                    {
                        printf("LA PARTICION %s YA FUE MONTADO CON EL ID %s \n", nombre, IDA);
                        montado = true;
                        encontro = true;
                        j = 4;
                    }
                    else
                    {
                        for (k = 0; k < 25; k++)
                        {
                            if (strcmp(Mounts[k].path, auxpath) == 0)
                            {
                                //COMPROBANDO EN QUE POSICION ESTA VACIA
                                int y;
                                int val;
                                //CANTIDAD DE ID DE UN MISMO DISCO
                                for (y = 0; y < 100; y++)
                                {
                                    if (Mounts[k].vda[y].ocupado == false)
                                    {
                                        val = Mounts[k].vda[y].aparicion;
                                        Mounts[k].vda[y].ocupado = true;
                                        y = 100;
                                    }
                                }
                                char id [10];
                                limpiarvariables(id, 10);
                                strcpy(id, "vd");
                                id[2] = letra[k];
                                sprintf(str, "%d", val);
                                strcat(id, str);
                                //strcpy(id,Mounts[k].aparicion);
                                printf("EL ID A CREAR ES:%s\n", id);
                                strcpy(YAmontados[cuantosmontados].id, id);
                                YAmontados[cuantosmontados].iniciopart = mb.particiones[j].part_start;
                                YAmontados[cuantosmontados].tamanio = mb.particiones[j].part_size;
                                strcpy(YAmontados[cuantosmontados].nombreParticion, nombre);
                                YAmontados[cuantosmontados].parametros = Mounts[k];
                                cuantosmontados++;
                                encontro = true;
                                k = 25;
                                montado = true;
                                j = 4;
                            }
                        }
                        if (encontro == false)
                        {
                            for (k = 0; k < 25; k++)
                            {
                                if (strcasecmp(Mounts[k].path, "vacio") == 0)
                                {
                                    Mounts[k].vda[0].ocupado = true;
                                    strcpy(Mounts[k].path, auxpath);
                                    char id [10];
                                    limpiarvariables(id, 10);
                                    strcat(id, "vd");
                                    id[2] = letra[k];
                                    sprintf(str, "%d", Mounts[k].vda[0].aparicion);
                                    strcat(id, str);
                                    printf("EL ID A CREAR ES:%s\n", id);
                                    strcpy(YAmontados[cuantosmontados].id, id);
                                    YAmontados[cuantosmontados].iniciopart = mb.particiones[j].part_start;
                                    YAmontados[cuantosmontados].tamanio = mb.particiones[j].part_size;
                                    strcpy(YAmontados[cuantosmontados].nombreParticion, nombre);
                                    YAmontados[cuantosmontados].parametros = Mounts[k];
                                    cuantosmontados++;
                                    k = 25;
                                    encontro = true;
                                    montado = true;
                                    j = 4;
                                }
                            }
                        }
                    }

                }
                else if (mb.particiones[j].part_type == 'E' || mb.particiones[j].part_type == 'e')
                {
                    if (yafuemontado(auxpath, nombre) == true)
                    {
                        printf("LA PARTICION %s YA FUE MONTADO CON EL ID %s \n", nombre, IDA);
                        montado = true;
                        encontro = true;
                        j = 4;
                    }
                    if (encontro == false)
                    {
                        EBR auxnombres;
                        int fin = mb.particiones[j].part_size + mb.particiones[j].part_start;
                        bool ultima = false;
                        for (i = mb.particiones[j].part_start; i < fin + 1 && ultima == false; i++)
                        {
                            fseek(ar, i, SEEK_SET);
                            fread(&auxnombres, sizeof (EBR), 1, ar);
                            if (strcasecmp(auxnombres.part_name, nombre) == 0)
                            {
                                char str[10];
                                limpiarvariables(str, 10);
                                int k;
                                //CANTIDAD DE DISCOS CON UN MAXIMO DE 25 DISCOS
                                if (yafuemontado(auxpath, nombre) == true)
                                {
                                    printf("La particion %s ya fue montada con el id %s \n", nombre, IDA);
                                }
                                else
                                {
                                    for (k = 0; k < 25; k++)
                                    {
                                        if (strcmp(Mounts[k].path, auxpath) == 0)
                                        {
                                            //COMPROBANDO EN QUE POSICION ESTA VACIA
                                            int y;
                                            int val;
                                            //CANTIDAD DE ID DE UN MISMO DISCO
                                            for (y = 0; y < 100; y++)
                                            {
                                                if (Mounts[k].vda[y].ocupado == false)
                                                {
                                                    val = Mounts[k].vda[y].aparicion;
                                                    Mounts[k].vda[y].ocupado = true;
                                                    y = 100;
                                                }
                                            }
                                            char id [10];
                                            limpiarvariables(id, 10);
                                            strcpy(id, "vd");
                                            id[2] = letra[k];
                                            sprintf(str, "%d", val);
                                            strcat(id, str);
                                            //strcpy(id,Mounts[k].aparicion);
                                            printf("EL ID A CREAR ES:%s\n", id);
                                            strcpy(YAmontados[cuantosmontados].id, id);
                                            strcpy(YAmontados[cuantosmontados].nombreParticion, nombre);
                                            YAmontados[cuantosmontados].iniciopart = mb.particiones[j].part_start;
                                            YAmontados[cuantosmontados].tamanio = mb.particiones[j].part_size;

                                            YAmontados[cuantosmontados].parametros = Mounts[k];
                                            cuantosmontados++;
                                            encontro = true;
                                            k = 25;
                                            montado = true;
                                            j = 4;
                                        }
                                    }
                                    if (encontro == false)
                                    {
                                        for (k = 0; k < 25; k++)
                                        {
                                            if (strcasecmp(Mounts[k].path, "vacio") == 0)
                                            {
                                                Mounts[k].vda[0].ocupado = true;
                                                strcpy(Mounts[k].path, auxpath);
                                                char id [10];
                                                limpiarvariables(id, 10);
                                                strcat(id, "vd");
                                                id[2] = letra[k];
                                                sprintf(str, "%d", Mounts[k].vda[0].aparicion);
                                                strcat(id, str);
                                                printf("EL ID A CREAR ES:%s\n", id);
                                                strcpy(YAmontados[cuantosmontados].id, id);
                                                strcpy(YAmontados[cuantosmontados].nombreParticion, nombre);
                                                YAmontados[cuantosmontados].iniciopart = mb.particiones[j].part_start;
                                                YAmontados[cuantosmontados].tamanio = mb.particiones[j].part_size;

                                                YAmontados[cuantosmontados].parametros = Mounts[k];
                                                cuantosmontados++;
                                                k = 25;
                                                montado = true;
                                                encontro = true;
                                                j = 4;
                                            }
                                        }
                                    }
                                }

                            }
                            if (auxnombres.part_next == -1)
                            {
                                ultima = true;
                            }
                            i = auxnombres.part_next - 1; //Se mueve i hasta donde termina la particion para leer otro bloque
                        }
                    }

                }

            }
            if (montado == false && encontro == false)
            {
                printf("LA PARTICION %s NO EXISTE EN EL DISCO\n", nombre);
            }


        }
        else
        {
            printf("NO EXISTE LA RUTA DEL ARCHIVO\n");
        }
    }
    else if (pat == false && name == false)
    {
        //printf("ERROR NO SE ENCONTRO LOS PARAMETROS [PATH-NAME]\n");
        int i;
        for (i = 0; i < 100; i++)
        {
            if (strcasecmp(YAmontados[i].id, "vacio") == 0 && strcasecmp(YAmontados[i].parametros.path, "vacio") == 0)
            {
            }
            else
            {
                printf("#id:%s -path:%s -name:%s\n", YAmontados[i].id, YAmontados[i].parametros.path, YAmontados[i].nombreParticion);
            }
        }
    }
    else
    {
        printf("ERROR NO SE ENCONTRO LOS PARAMETROS [PATH-NAME]\n");
    }
}

void generarreporte(char nombreArchivo[], char nombreSalida[]) {
    int tam;
    int fragini, fragfin, fragmidl;
    MBR mb;
    int count = 0;
    int contador = 0;
    FILE * ar;
    FILE * ardot;
    bool existe = false;
    ardot = fopen(nombreArchivo, "rb+");
    ar = fopen("/home/daniel/Escritorio/prueba.dot", "wt+");
    char str[15];
    if (ardot != NULL) {
        fread(&mb, sizeof (MBR), 1, ardot);
        existe=true;
    } else {
        printf("ERROR AL ABRIR EL ARCHIVO\n");
    }
    //COMENZANDO A LEER EL ARCHIVO
    if (ar != NULL && existe==true) {
        fflush(ar);
        int ext = cuantasEXTENDIDAS(mb);
        int pri = cuantasPRIMARIAS(mb);
        int totalparticiones = ext + pri;
        Particion auxpart[totalparticiones];
        int j, p, cont = 0;
	//GUARDANDO SOLO LAS PARTICIONES QUE ESTAN ACTIVAS
        for (j = 0; j < 4; j++) {
            if (mb.particiones[j].part_status =='1') {
                auxpart[cont] = mb.particiones[j];
                cont++;
            }
        }

        //ORDENANDO POR MEDIO DE BIT DE MAYOR A MENOR
        for (j = 0; j < totalparticiones; j++) {
            for (p = 0; p < totalparticiones - 1; p++) {
                if (auxpart[p].part_start > auxpart[p + 1].part_start) {
                    Particion aux = auxpart[p];
                    auxpart[p] = auxpart[p + 1];
                    auxpart[p + 1] = aux;
                }
            }
        }

        fprintf(ar, "digraph G {\n nodesep=.05;\n rankdir=BT;\n  node [shape=record,style=filled,fillcolor=goldenrod3]\n");
        fprintf(ar, "label=\"DISCO LWH\";\n");
        //GRAFICANDO EL MBR
        fprintf(ar, "\nsubgraph cluster%d{\n", count);
        count++;
        fprintf(ar, "label=\"MBR\";\n node%d", contador);
        contador++;
        fprintf(ar, "[label=\"ID:%d \nTAMANIO:%d \nFECHA:%s\", height=1];\n}", mb.mbr_disk_signature, mb.mbr_tamanio, mb.mbr_fecha_creacion);
        fprintf(ar, "\nsubgraph cluster%d {\n", count);
        count++;
        fprintf(ar, "label=\"PARTICIONES\";\n");
        /******************************************************
         *****************GRAFICANDO PARTICIONES**************
         *****************************************************/
        int h;
        for (h = totalparticiones - 1; h >= 0; h--) {
            if (h == 0) {
                bool ultima = false;
                tam = auxpart[h].part_start + auxpart[h].part_size;
                //SI ES LA ULTIMA PARTICION
                fragini = auxpart[h].part_start - (sizeof (MBR) + 1);

                if (totalparticiones == 1) {
                    ultima = true;
                    fragfin = mb.mbr_tamanio - tam;
                    fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragfin);
                    contador++;
                }

                if (totalparticiones > 1) {
                    fragmidl = (auxpart[h + 1].part_start)-(tam + 1);
                    if (fragmidl > 0 && fragini > 0 && ultima == false) {
                        fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragini);
                        contador++;
                    } else {
                        if (fragmidl > 0) {
                            fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragmidl);
                            contador++;
                        }
                    }
                }
                fprintf(ar, "node%d [label=\" NOMBRE:%s \n INICIO:%d \n TAMANIO:%d \n TIPO:%c \",height=1];\n", contador, auxpart[h].part_name, auxpart[h].part_start, auxpart[h].part_size, auxpart[h].part_type);
                contador++;

                if (fragini > 0) {
                    fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragini);
                    contador++;
                }


            } else if (h == 1) {
                tam = auxpart[h].part_start + auxpart[h].part_size;
                //SI ES LA ULTIMA PARTICION
                if (totalparticiones == 2) {
                    fragfin = mb.mbr_tamanio - tam;
                    fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragfin);
                    contador++;
                }

                //VALIDANDO FRAGMENTACIONES
                fragini = auxpart[h + 1].part_start - (tam + 1);
                if (fragini > 0) {
                    fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragini);
                    contador++;
                }
                fprintf(ar, "node%d [label=\" NOMBRE:%s \n INICIO:%d \n TAMANIO:%d \n TIPO:%c \",height=1];\n", contador, auxpart[h].part_name, auxpart[h].part_start, auxpart[h].part_size, auxpart[h].part_type);
                contador++;

               } else if (h == 2) {
                tam = auxpart[h].part_start + auxpart[h].part_size;
                //SI ES LA ULTIMA PARTICION
                if (totalparticiones == 3) {
                    fragfin = mb.mbr_tamanio - tam;
                    fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragfin);
                    contador++;
                    fragmidl = 0;
                } else {
                    fragmidl = auxpart[h + 1].part_start - (tam + 1);

                }

                if (fragmidl > 0) {
                    fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragmidl);
                    contador++;
                }

                fprintf(ar, "node%d [label=\" NOMBRE:%s \n INICIO:%d \n TAMANIO:%d \n TIPO:%c \",height=1];\n", contador, auxpart[h].part_name, auxpart[h].part_start, auxpart[h].part_size, auxpart[h].part_type);
                contador++;
            } else if (h == 3) {
                tam = auxpart[h].part_start + auxpart[h].part_size;
                //SI ES LA ULTIMA PARTICION
                fragfin = mb.mbr_tamanio - tam;
                if (totalparticiones == 4) {
                    if (fragfin > 0) {
                        //    printf("EL TOTAL DE ESPACIO LIBRE DEL FINAL DEL DISCO ES:%d\n", frag5);
                        fprintf(ar, "node%d[label=\"LIBRE:%d \",height=1,shape=record,style=filled,fillcolor=white];\n", contador, fragfin);
                        contador++;
                    }

                }
                fprintf(ar, "node%d [label=\" NOMBRE:%s \n INICIO:%d \n TAMANIO:%d \n TIPO:%c \",height=1];\n", contador, auxpart[h].part_name, auxpart[h].part_start, auxpart[h].part_size, auxpart[h].part_type);
                contador++;
            }
        }

        for(h=totalparticiones-1;h>=0;h--){
        if (auxpart[h].part_type == 'E' || auxpart[h].part_type == 'e') {
                    EBR veclogicas[50];
                    int logic = 0;
                    int j;
                    EBR ebr;

                    fseek(ardot, auxpart[h].part_start, SEEK_SET);
                    fread(&ebr, sizeof (EBR), 1, ardot);
                    j = 0;
                    while (j < 50) {
                        if (ebr.part_next == -1) {
                            if (ebr.part_status == '1') {
                                veclogicas[logic] = ebr;
                                logic++;
                            }
                            break;
                        }
                        if (ebr.part_status == '1') {
                            veclogicas[logic] = ebr;
                            logic++;

                        }
                        fseek(ardot, ebr.part_next, SEEK_SET);
                        fread(&ebr, sizeof (EBR), 1, ardot);
                        j++;
                    }


                    fprintf(ar, "subgraph cluster_0 {\n");
                    fprintf(ar, "rankdir = \"RL\";\n");
                    fprintf(ar, "label = \"LOGICAS\";\n");
                    fprintf(ar, "n_n[shape=\"box\",style=\"filled\",color=\"white\",label=\"\",width = 0.00001, heigth = 0.00001];\n");

                    j = logic - 1;
                    bool primera = false;
                    int a = 100;
                    while (j >= 0) {
                        if (primera == false) {
                            fprintf(ar, "n_%d_%d [shape=\"rectangle\",label=\"EBR\",fillcolor=orangered];\n", a, j);
                            a--;
                            fprintf(ar, "n_%d_%d [shape=\"rectangle\",label=\"NOMBRE:%s\n \n TAMANIO:%d \n INICIO:%d \n NEXT:%d \"];\n", a, j, veclogicas[j].part_name, veclogicas[j].part_size, veclogicas[j].part_start, veclogicas[j].part_next);
                            a--;
                            fprintf(ar, "n_%d_%d [shape=\"rectangle\",label=\"EBR\",fillcolor=orangered];\n", a, j);
                            a--;

                            primera = true;
                        } else {
                            fprintf(ar, "n_%d_%d [shape=\"rectangle\",label=\"NOMBRE:%s\n \n TAMANIO:%d \n INICIO:%d \n NEXT:%d \"];\n", a, j, veclogicas[j].part_name, veclogicas[j].part_size, veclogicas[j].part_start, veclogicas[j].part_next);
                            a--;
                            fprintf(ar, "n_%d_%d [shape=\"rectangle\",label=\"EBR\",fillcolor=orangered];\n", a, j);
                            a--;

                        }
                        j--;
                    }
                    fprintf(ar, "}\n");

                }

        }
        fprintf(ar, "\n} \n}");
        fclose(ar);


    } else {
        printf("NO EXISTE EL ARCHIVO\n");
    }

    char id[5];
    sprintf(id, "%d", mb.mbr_disk_signature);
    char cmd [200];
    char aux[100];
    limpiarvariables(aux, 100);
    limpiarvariables(cmd, 200);
    strcat(aux, "\"");
    strcat(aux, nombreSalida);
    strcat(aux, "\"");
    strcpy(cmd, "dot -Tjpg /home/daniel/Escritorio/prueba.dot -o");
    strcat(cmd, aux);
    system(cmd);
}
void generarreporteMBR(char nombreArchivo[], char nombreSalida[]) {
    MBR mb;
    int count = 0;
    int contador = 0;
    FILE * ar;
    FILE * ardot;
    ardot = fopen(nombreArchivo, "rb+");
    ar = fopen("/home/daniel/Escritorio/MBR.dot", "wt+");
    char str[15];
    if (ardot != NULL) {
        fread(&mb, sizeof (MBR), 1, ardot);
    } else {
        printf("ERROR AL ABRIR EL ARCHIVO\n");
    }
    //COMENZANDO A LEER EL ARCHIVO
    if (ar != NULL) {
        fprintf(ar, "digraph G {\n nodesep=.05;\n rankdir=LR;\n  node [shape=plaintext,width=.1,height=.1];\n");
        //GRAFICANDO EL MBR
        fprintf(ar, "node%d [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\"", contador);
        contador++;
        fprintf(ar, "> <tr><td><b>NOMBRE</b></td><td><b>VALOR</b></td></tr>");
        fprintf(ar, "<tr><td>mbr_tamanio</td><td>%d</td></tr>", mb.mbr_tamanio);
        fprintf(ar, "<tr><td>mbr_fecha</td><td>%s</td></tr>", mb.mbr_fecha_creacion);
        fprintf(ar, "<tr><td>mbr_disk_signature</td><td>%d</td></tr>", mb.mbr_disk_signature);
        int h;
        for (h = 0; h < 4; h++) {
            if(mb.particiones[h].part_status=='1'){
            //printf("POS %d START %d SIZE %d\n", h, auxpart[h].part_start, auxpart[h].part_size);
            fprintf(ar, "<tr><td>part_name_%d</td><td>%s</td></tr>\n", h, mb.particiones[h].part_name);
            fprintf(ar, "<tr><td>part_start_%d</td><td>%d</td></tr>\n", h,mb.particiones[h].part_start);
            fprintf(ar, "<tr><td>part_type_%d</td><td>%c</td></tr>\n", h, mb.particiones[h].part_type);
            fprintf(ar, "<tr><td>part_fit_%d</td><td>%c</td></tr>\n", h, mb.particiones[h].part_fit);
            fprintf(ar, "<tr><td>part_start_%d</td><td>%d</td></tr>\n", h, mb.particiones[h].part_start);
            fprintf(ar, "<tr><td>part_tamanio_%d</td><td>%d</td></tr>\n", h,mb.particiones[h].part_size);
            }
        }

        fprintf(ar, " </table>>];\n");

        //VALIDANDO QUE EXISTAN PARTICIONES LOGICAS
        for (h = 0; h < 4; h++) {
            int i;
            FILE * nuevodisco;
            int ecount = 0;
            if ((mb.particiones[h].part_type == 'e' || mb.particiones[h].part_type == 'E')&&mb.particiones[h].part_status=='1') {
                    EBR veclogicas[50];
                    int logic = 0;
                    int j;
                    EBR ebr;

                    fseek(ardot, mb.particiones[h].part_start, SEEK_SET);
                    fread(&ebr, sizeof (EBR), 1, ardot);
                    j = 0;
                    while (j < 50) {
                        if (ebr.part_next == -1) {
                            if (ebr.part_status == '1') {
                                veclogicas[logic] = ebr;
                                logic++;
                            }
                            break;
                        }
                        if (ebr.part_status == '1') {
                            veclogicas[logic] = ebr;
                            logic++;

                        }
                        fseek(ardot, ebr.part_next, SEEK_SET);
                        fread(&ebr, sizeof (EBR), 1, ardot);
                        j++;
                    }

                    j = 0;
                    bool primera = false;
                    int a = 100;
                    while (j < (logic-1) ) {

                      fprintf(ar, "node%d [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\"", contador);
                        contador++;
                        fprintf(ar, "> <tr><td><b>NOMBRE</b></td><td><b>VALOR</b></td></tr>\n");
                        fprintf(ar, "<tr><td>part_fit_%d</td><td>%c</td></tr>\n", ecount, veclogicas[j].part_fit);
                        fprintf(ar, "<tr><td>part_name_%d</td><td>%s</td></tr>\n", ecount, veclogicas[j].part_name);
                        fprintf(ar, "<tr><td>part_next_%d</td><td>%d</td></tr>\n", ecount, veclogicas[j].part_next);
                        fprintf(ar, "<tr><td>part_size_%d</td><td>%d</td></tr>\n", ecount, veclogicas[j].part_size);
                        fprintf(ar, "<tr><td>part_start_%d</td><td>%d</td></tr>\n", ecount, veclogicas[j].part_start);
                        fprintf(ar, "<tr><td>part_status_%d</td><td>%c</td></tr>\n", ecount, veclogicas[j].part_status);
                        ecount++;
                        fprintf(ar, " </table>>];\n");

                        j++;

                    }

            }
        }
        fprintf(ar, "\n}");
        fclose(ar);
        fclose(ardot);
    } else {
        printf("NO EXISTE EL ARCHIVO\n");
    }

    char cmd [200];
    char aux[100];
    limpiarvariables(aux, 100);
    limpiarvariables(cmd, 200);
    strcat(aux, "\"");
    strcat(aux, nombreSalida);
    strcat(aux, "\"");
    strcpy(cmd, "dot -Tpdf /home/daniel/Escritorio/MBR.dot -o");
    strcat(cmd, aux);
    system(cmd);
}

void rep(Comando cmd[]) {
    bool ide = false;
    bool pat = false;
    bool name = false;
    bool error = false;
    bool root = false;
    char id[10];
    char path[100];
    char nombre[10];
    char nombreP[25];
    char auxpath[100];
    limpiarvariables(nombreP, 25);
    limpiarvariables(path, 100);
    limpiarvariables(auxpath, 100);
    limpiarvariables(nombre, 10);
    limpiarvariables(id, 10);
    char ruta[100];
    limpiarvariables(ruta, 100);
    char auxruta[100];
    limpiarvariables(auxruta, 100);
    int bitinicio;
    int i = 0;
    while (strcasecmp(cmd[i].comando, "vacio") != 0) {
        if (strcasecmp(cmd[i].comando, "-id") == 0 || strcasecmp(cmd[i].comando, "id") == 0) {
            ide = true;
            strcpy(id, cmd[i + 1].comando);
            printf("LA ID PARA EL REPORTE SERA:%s\n", id);
            int k;
            for (k = 0; k < 100; k++) {
                if (strcasecmp(YAmontados[k].id, id) == 0) {
                    strcpy(nombreP, YAmontados[k].nombreParticion);
                    bitinicio = YAmontados[k].iniciopart;
                    error = false;
                    k = 100;
                } else {
                    error = true;
                }
            }
            if (error == true) {
                printf("EL ID %s NO HA SIDO MONTADO\n", id);
            }
        } else if (strcasecmp(cmd[i].comando, "-name") == 0 || strcasecmp(cmd[i].comando, "name") == 0) {
            strcpy(nombre, cmd[i + 1].comando);
            printf("EL TIPO DE REPORTE SERA:%s\n", nombre);
            if (strcasecmp(nombre, "mbr") == 0 || strcasecmp(nombre, "disk") == 0 ) {
                name = true;
            } else {
                error = true;
                printf("EL TIPO DE REPORTE:%s NO ESTA DEFINIDO\n", nombre);
            }
        } else if (strcasecmp(cmd[i].comando, "-path") == 0 || strcasecmp(cmd[i].comando, "path") == 0) {
            pat = true;
            strcpy(path, cmd[i + 1].comando);
            printf("EL NOMBRE DEL REPORTE SERA:%s\n", path);
            //limpiarvariables(auxpath, 100);
            if (path[0] == '\"') {
                int a;
                //CONCATENANDO TODA LA PATH COMPLETA
                for (a = i + 2; a < 25; a++) {
                    strcpy(auxpath, cmd[a].comando);
                    int b = contarcaracteres(auxpath);
                    if (auxpath[b - 1] != '\"') {
                        if (strcasecmp(auxpath, "vacio") == 0) {
                            a = 25;
                        } else {
                            strcat(path, " ");
                            strcat(path, auxpath);
                        }
                    } else {
                        strcat(path, " ");
                        strcat(path, auxpath);
                        a = 25;
                        //limpiarvariables(auxpath, 100);
                    }
                }
                /**********QUITNADO LAS COMILLAS************/
                limpiarvariables(auxpath, 100);
                int l;
                int cont = 0;
                for (l = 1; l < 100; l++) {
                    if (path[l] == '\"') {
                        l = 100;
                    } else {
                        auxpath[cont] = path[l];
                        cont++;
                    }
                }
                printf("LA CARPETA COMPLETA ES:%s\n", auxpath);
                existepath(auxpath);

            } else {
                strcpy(auxpath, path);
                existepath(auxpath);
            }
        } else if (strcasecmp(cmd[i].comando, "-ruta") == 0 || strcasecmp(cmd[i].comando, "ruta") == 0) {
            root = true;
            strcpy(ruta, cmd[i + 1].comando);
            printf("EL NOMBRE DE LA RUTA:%s\n", ruta);
            if (ruta[0] == '\"') {
                int a;
                //CONCATENANDO TODA LA PATH COMPLETA
                for (a = i + 2; a < 25; a++) {
                    strcpy(auxruta, cmd[a].comando);
                    int b = contarcaracteres(auxruta);
                    if (auxruta[b - 1] != '\"') {
                        if (strcasecmp(auxruta, "vacio") == 0) {
                            a = 25;
                        } else {
                            strcat(ruta, " ");
                            strcat(ruta, auxruta);
                        }
                    } else {
                        strcat(ruta, " ");
                        strcat(ruta, auxruta);
                        a = 25;
                        //limpiarvariables(auxpath, 100);
                    }
                }
                /**********QUITNADO LAS COMILLAS************/
                limpiarvariables(auxruta, 100);
                int l;
                int cont = 0;
                for (l = 1; l < 100; l++) {
                    if (ruta[l] == '\"') {
                        l = 100;
                    } else {
                        auxruta[cont] = ruta[l];
                        cont++;
                    }
                }
                printf("LA CARPETA COMPLETA ES:%s\n", auxruta);
                //                existepath(auxpath);

            } else {
                strcpy(auxruta, ruta);
                //              existepath(auxpath);
            }
        }
        i++;
    }
    if (error == false) {
        if (ide == true && pat == true && name == true) {
            int j;
            char nombreArchivo[100];
            char nombreparticion[25];
            limpiarvariables(nombreparticion, 25);
            limpiarvariables(nombreArchivo, 100);
            for (j = 0; j < 100; j++) {
                if (strcasecmp(YAmontados[j].id, id) == 0) {
                    strcpy(nombreArchivo, YAmontados[j].parametros.path);
                    strcpy(nombreparticion, YAmontados[j].nombreParticion);
                    j = 100;
                    printf("EL PATH DEL ID %s ES %s\n", id, nombreArchivo);
                    NodoMontados mt = YAmontados[j];
                }
            }
            if (strcasecmp(nombre, "disk") == 0) {
                generarreporte(nombreArchivo, auxpath);
            } else if (strcasecmp(nombre, "mbr") == 0) {
                generarreporteMBR(nombreArchivo,auxpath);
            }
        } else {
            printf("ERROR FALTAN PARAMETROS [ID-NOMBRE-PATH]\n");
        }

    }
}

void Mkdisk(Comando cmd[])
{
    bool size = false;
    bool unit = false;
    bool path = false;
    bool opcional = true;
    bool error = false;
    bool name = false;
    int e = 0;
    char nombre[25];
    char pat[100];
    char uni[2];
    int tamanio;
    char auxpath [100];
    char auxnombre[100];
    limpiarvariables(auxnombre, 100);
    limpiarvariables(auxpath, 100);
    limpiarvariables(pat, 100);
    limpiarvariables(uni, 2);
    limpiarvariables(nombre, 25);
    int i = 0;
    while (strcasecmp(cmd[i].comando, "vacio") != 0)
    {
        if (strcasecmp(cmd[i].comando, "size") == 0 || strcasecmp(cmd[i].comando, "-size") == 0)
        {
            size = true;
            tamanio = atoi(cmd[i + 1].comando);
            if (tamanio > 0)
            {
                printf("EL TAMANIO DEL DISCO SERA %d\n",tamanio);
            }
            else
            {
                error = true;
                e = i;
                printf("ERROR! SOLO VALORES POSIVITOS MAYORES QUE 0\n");
                i = 20;
            }
        }
        else if (strcasecmp(cmd[i].comando, "unit") == 0 || strcasecmp(cmd[i].comando, "+unit") == 0)
        {
            unit = true;
            strcpy(uni, cmd[i + 1].comando);
            printf("LA UNIDAD DEL DISCO SERA:%s\n", uni);
            opcional = false;
            if (strcasecmp(uni, "k") == 0 || strcasecmp(uni, "m") == 0)
            {
                //PARAMETROS CORRECTOS
            }
            else
            {
                printf("ERROR! UNIDAD NO VALIDA\n");
                error = true;
                e = i;
                i = 20;
            }
        }
        else if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0)
        {
            path = true;
            strcpy(pat, cmd[i + 1].comando);
            //printf("LA PATH DEL DISCO SERA: %s \n", pat);
            limpiarvariables(auxpath, 100);
            if (pat[0] == '\"')
            {
                int cantidad = contarcaracteres(pat);
                if(pat[cantidad-1]=='\"')
                {
                    /**********QUITNADO LAS COMILLAS************/
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (pat[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxpath[cont] = pat[l];
                            cont++;
                        }
                    }
                    printf("EL NOMBRE DEL DISCO ES:%s\n", auxpath);
                }
                else
                {
                    int a;
                    //CONCATENANDO TODA LA PATH COMPLETA
                    for (a = i + 2; a < 25; a++)
                    {
                        strcpy(auxpath, cmd[a].comando);
                        int b = contarcaracteres(auxpath);
                        if (auxpath[b - 1] != '\"')
                        {
                            if (strcasecmp(auxpath, "vacio") == 0)
                            {
                                a = 25;
                            }
                            else
                            {
                                strcat(pat, " ");
                                strcat(pat, auxpath);
                            }
                        }
                        else
                        {
                            strcat(pat, " ");
                            strcat(pat, auxpath);
                            a = 25;
                        }
                    }
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxpath, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (pat[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxpath[cont] = pat[l];
                            cont++;
                        }
                    }
                    printf("LA CARPETA COMPLETA ES:%s\n", auxpath);
                }

            }
            else
            {
                strcpy(auxpath, pat);
                printf("LA CARPETA COMPLETA ES:%s\n", auxpath);

            }
        }
        else if (strcasecmp(cmd[i].comando, "-name") == 0)
        {
            //  char auxnombre[25];
            //limpiarvariables(nombre, 25);
            strcpy(nombre, cmd[i + 1].comando);

            if(nombre[0]=='\"')
            {
                int cantidad = contarcaracteres(nombre);
                if(nombre[cantidad-1]=='\"')
                {
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxnombre, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (nombre[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxnombre[cont] = nombre[l];
                            cont++;
                        }
                    }
                    printf("EL NOMBRE DEL ARCHIVO SERA:%s\n", auxnombre);
                }
                else
                {
                    //TIENE ESPACIOS EN BLANCO
                    int a;
                    //CONCATENANDO TODA LA PATH COMPLETA
                    for (a = i + 2; a < 25; a++)
                    {
                        int b = contarcaracteres(auxnombre);
                        if (auxnombre[b - 1] != '\"')
                        {
                            strcat(nombre, " ");
                            strcat(nombre, auxnombre);
                        }
                        else
                        {
                            strcat(nombre, " ");
                            strcat(nombre, auxnombre);
                            a = 25;
                        }
                    }
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxnombre, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (nombre[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxnombre[cont] = nombre[l];
                            cont++;
                        }
                    }
                    printf("EL NOMBRE DEL ARCHIVO SERA:%s\n", auxnombre);
                }
            }
            else
            {
                strcpy(auxnombre, nombre);
                printf("EL NOMBRE DEL ARCHIVO SERA:%s\n", auxnombre);

            }

            char* token;
            char auxvalidar [100];
            strcpy(auxvalidar,auxnombre);
            token = strtok(auxvalidar, ".");
            bool encontropunto = false;
            while (token != NULL)
            {
                if (strcasecmp(token, "dsk") == 0)
                {
                    encontropunto = true;
                    name = true;
                }
                token = strtok(NULL, ".");
            }
            if(encontropunto==false)
            {
                printf("ERROR!!! EL DISCO PARA CREAR NO CUENTA CON UNA EXTENSION ACEPTABLE\n");
                error = true;
                e = i;
                i = 20;

            }


        }
        i++;
    }
    if (opcional == true)
    {
        strcpy(uni, "m");
    }

    if (size == false || path == false || error == true || name == false)
    {
        printf("ERROR EN EL COMANDO %s:%s\n", cmd[e].comando, cmd[e + 1].comando);

    }
    else
    {
        strcat(auxpath, "/");
        strcat(auxpath, auxnombre);
        existepath(auxpath);
        if (strcasecmp(uni, "k") == 0)
        {
            crearMBR(auxpath, tamanio * 1024);
        }
        else if (strcasecmp(uni, "m") == 0)
        {
            crearMBR(auxpath, (tamanio * 1024)*1024);
        }
    }
}

void crearMBR(char* nombreArchivo, int tamanio)
{
    FILE *tmp;
    if ((tmp = fopen(nombreArchivo, "rb")))
    {
        printf("DISCO YA EXISTE\n");
        fclose(tmp);
    }
    else
    {
        FILE *disco;
        if ((disco = fopen(nombreArchivo, "wb")))
        {
            MBR mbr;
            char output[20];
            strcpy(output, retornarfecha());
            strcpy(mbr.mbr_fecha_creacion, output);
            printf("FECHA DE CREACION:%s\n", output);
            printf("TAMANIO DEL DISCO:%d\n", tamanio);
            mbr.mbr_tamanio = tamanio;
            srand(time(NULL));
            int r = (rand() % 50);
            printf("NUMERO DEL DISCO=%d\n", r);
            mbr.mbr_disk_signature = r;
            Particion particion;
            particion.part_fit = 'N';
            particion.part_status = '0';
            particion.part_start = 0;
            strcpy(particion.part_name, "\0");
            particion.part_size = 0;
            particion.part_type = 'N';
            int i;
            for (i = 0; i < 4; i++)
            {
                mbr.particiones[i] = particion;
            }
            fwrite(&mbr, sizeof (MBR), 1, disco);
            fseek(disco, tamanio - 1, SEEK_SET);
            char trash = '\0';
            fwrite(&trash, 1, 1, disco);
            fclose(disco);
        }
        else
        {
            printf("IMPOSIBLE CREAR EL DISCO\n");
        }

    }


}

void rmDisk(Comando cmd[])
{
    char pat[100];
    char auxpath[100];
    char con [2];
    printf("DESEA ELIMINAR EL DISCO [Y/N]\n");
    fgets(con, 3, stdin);
    cambio(con);
    limpiarvariables(pat, 100);
    int i = 0;
    if (strcasecmp(con, "y") == 0)
    {
        while (strcasecmp(cmd[i].comando, "vacio") != 0)
        {
            if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0)
            {
                strcpy(pat, cmd[i + 1].comando);
                printf("LA PATH DEL DISCO SERA: %s \n", pat);
                limpiarvariables(auxpath, 100);
                if (pat[0] == '\"')
                {
                    int a;
                    //CONCATENANDO TODA LA PATH COMPLETA
                    for (a = i + 2; a < 25; a++)
                    {
                        strcpy(auxpath, cmd[a].comando);
                        int b = contarcaracteres(auxpath);
                        if (auxpath[b - 1] != '\"')
                        {
                            strcat(pat, " ");
                            strcat(pat, auxpath);
                            //limpiarvariables(auxpath, 100);
                        }
                        else
                        {
                            strcat(pat, " ");
                            strcat(pat, auxpath);
                            a = 25;
                            //limpiarvariables(auxpath, 100);
                        }
                    }
                    /**********QUITNADO LAS COMILLAS************/
                    limpiarvariables(auxpath, 100);
                    int l;
                    int cont = 0;
                    for (l = 1; l < 100; l++)
                    {
                        if (pat[l] == '\"')
                        {
                            l = 100;
                        }
                        else
                        {
                            auxpath[cont] = pat[l];
                            cont++;
                        }
                    }
                    printf("LA CARPETA COMPLETA ES:%s\n", auxpath);
                    existepath(auxpath);
                }
                else
                {
                    strcpy(auxpath, pat);
                    existepath(auxpath);
                }
            }
            i++;
        }
        char * cadena = ("\"%s\"", auxpath);
        if (remove(cadena) == 0) // Eliminamos el archivo
            printf("EL ARCHIVO: %s FUE ELIMINADO SATISFACTORIAMENTE\n", cadena);
        else
            printf("PROBLEMAS PARA ELIMAR ARCHIVO\n");
    }
    else
    {
        printf("OK NO ELIMINA NADA");
    }
}

void existepath(char path[])
{
    char aux [500];
    strcpy(aux, path);
    struct stat datosficheros;
    lstat(aux, &datosficheros);
    if (S_ISREG(datosficheros.st_mode))
    {
        printf("YA EXISTE LA CARPETA\n");
    }
    else
    {
        /**************COMPROBANDO QUE CADA DIRECCION DEL PATH EXISTA*********************/
        char completo[500];
        strcpy(completo, path);
        int i = sabercuantosslash(path);
        char value[500];
        int limite = i - 1;
        int j;
        int contador = 0;
        strcpy(value, "/home");
        char * token;
        token = strtok(completo,"/");
        while(token!=NULL)
        {
            if (contador > 0)
            {
                if (contador < limite)
                {
                    strcat(value, "/");
                    strcat(value, token);
                }
                if (mkdir(value, S_IRWXU) == 0)
                {
                    printf("SE CREARA LA CARPETA %s\n", value);
                }
            }
            contador++;
            token = strtok(NULL,"/");
        }
        printf("\n");
    }
}

int sabercuantosslash(char * path)
{
    char * tampath;
    tampath = path;
    int cantidad = 0;
    while (*tampath)
    {
        if (*tampath == '/')
        {
            cantidad++;
        }
        tampath++;
    }
    return cantidad;
}

int main()
{
    int i;
    for (i = 0; i < 25; i++)
    {
        strcpy(VComandos[i].comando, "VACIO");
        strcpy(Mounts[i].path, "VACIO");
        int j;
        for (j = 0; j < 100; j++)
        {
            Posiciones auxllenar;
            auxllenar.aparicion = 0;
            auxllenar.ocupado = false;
            Mounts[i].vda[j].aparicion = j + 1;
            Mounts[i].vda[j].ocupado = false;
        }

    }
    for (i = 0; i < 100; i++)
    {
        strcpy(YAmontados[i].id, "vacio");
        YAmontados[i].parametros = Mounts[0];
    }
    int salida = 1;
    while (salida != 0)
    {
        char pal[200];
        printf("SHELL:$ ");
        fgets(pal, 200, stdin);
        cambio(pal);
        int count = 0;
        char comando[100];
        limpiarvariables(comando, 100);
        char aux[100];
        limpiarvariables(aux, 100);
        //COMPROBANDO SI ES MULTILINEA
        int final = strlen(pal) - 1;
        int asci = pal[final];
        char c = pal[final];
        if (asci == 92)
        {
            pal[final] = ' ';
            fgets(aux, 100, stdin);
            cambio(aux);
        }
        strcat(pal, aux);
        if (strcasecmp(pal, "exit") == 0)
        {
            salida = 0;
            retornarfecha();
            printf("SALIENDO DEL SISTEMA DE CREACION DE DISCOS\n");
        }
        else
        {
            char* token;
            token = strtok(pal, " ");
            while (token != NULL)
            {
                strcpy(VComandos[count].comando, token);
                count++;
                token = strtok(NULL, " ::");
            }
            printf("\n");
            printf("\n");
            if (strcasecmp(VComandos[0].comando, "mkdisk") == 0)
            {
                Mkdisk(VComandos);
            }
            else if (strcasecmp(VComandos[0].comando, "rmdisk") == 0)
            {
                rmDisk(VComandos);
            }
            else if (strcasecmp(VComandos[0].comando, "clear") == 0)
            {
                system("clear");
            }
            else if (strcasecmp(VComandos[0].comando, "fdisk") == 0)
            {
                fdisk(VComandos);
            }
            else if (strcasecmp(VComandos[0].comando, "mount") == 0)
            {
                mount(VComandos);
            }
            else if (strcasecmp(VComandos[0].comando, "mount") == 0)
            {
                mount(VComandos);
            }else if (strcasecmp(VComandos[0].comando, "rep") == 0) {
                rep(VComandos);
            }
            else
            {
                printf("COMANDO INVALIDO\n");
            }
            limpiarcomando();
        }
    }
    return 0;
}
