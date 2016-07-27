#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

typedef struct {
    char comando [100];
} Comando;

typedef struct {
    char part_status; //Indica si la partición está activa o no
    char part_type; /*Indica el tipo de partición, primaria o extendida.
Tendrá los valores P o E*/
    char part_fit; /*Tipo de ajuste de la partición. Tendrá los valores B (Best), F (First) o
W (worst)*/
    int part_start; //Indica en que byte del disco inicia la partición
    int part_size; // Contiene el tamaño total de la partición en bytes.
    char part_name[16]; // Nombre de la partición
} Particion;


typedef struct {
    int mbr_tamanio; //Tamaño total del disco en bytes
    char mbr_fecha_creacion [25]; //Fecha y hora de creación del disco
    int mbr_disk_signature; //Número random, que identifica de forma única a cada disco
    /*Particion * mbr_partition_1; //Estructura con información de la partición 1
    Particion * mbr_partition_2; //Estructura con información de la partición 2
    Particion * mbr_partition_3; //Estructura con información de la partición 3
    Particion * mbr_partition_4; // Estructura con información de la partición 4*/
    Particion particiones[4];
} MBR;


Comando VComandos[25];

char * retornarfecha() {
    time_t t;
    struct tm *tm;
    char fechayhora[100];
    t = time(NULL);
    tm = localtime(&t);
    strftime(fechayhora, 100, "%d/%m/%Y %H:%M:%S", tm);
    // printf("%s\n", fechayhora);

    return fechayhora;
}

int contarcaracteres(char pal []) {
    int i;
    int cantidad = 0;
    for (i = 0; i < strlen(pal); i++) {
        if (pal[i] != '\0') {
            cantidad++;
        } else {
        }
    }
    return cantidad;
}

char** Splitpalabras(char* texto, const char cdelimitador) {
    char** cadena = 0;
    size_t contador = 0;
    char* temp = texto;
    char* apariciones = 0;
    char delimitador[2];
    delimitador[0] = cdelimitador;
    delimitador[1] = 0;

    //cuenta la cantidad de aparacianes del delimitadoritador para poder extraer los elementos
    while (*temp) {
        if (cdelimitador == *temp) {
            contador++;
            apariciones = temp;
        }
        /*if (*temp == '-') {
         *temp = ' ';
        }*/
        temp++;
    }
    contador += apariciones < (texto + strlen(texto) - 1);
    //printf("%d",contador);
    //Añadir el espacio para la terminación de cadena nula por lo que llama
    //sabe donde la lista de cadenas devueltas termina
    contador++;

    cadena = malloc(sizeof (char*) * contador);

    if (cadena) {
        size_t idx = 0;
        char* token = strtok(texto, delimitador);

        while (token) {
            if (idx < contador)
                *(cadena + idx++) = strdup(token);
            token = strtok(0, delimitador);
        }
        if (idx == contador - 1)
            *(cadena + idx) = 0;
    }

    return cadena;
}

void cambio(char pal[]) {
    int i;
    for (i = 0; i < 200; i++) {
        if (pal[i] == '\n' || pal[i] == '\r') {
            pal[i] = '\0';
            i = 200;
        }
    }
}

void limpiarcomando() {
    int i;
    for (i = 0; i < 25; i++) {
        strcpy(VComandos[i].comando, "VACIO");
    }
}

void limpiarvariables(char pal[], int intervalo) {
    int f;
    for (f = 0; f < intervalo; f++) {
        pal[f] = '\0';
    }
}


void Mkdisk(Comando cmd[]) {
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
    limpiarvariables(auxpath, 100);
    limpiarvariables(pat, 100);
    limpiarvariables(uni, 2);
    limpiarvariables(nombre,25);
    int i = 0;
    while (strcasecmp(cmd[i].comando, "vacio") != 0) {
        if (strcasecmp(cmd[i].comando, "size") == 0 || strcasecmp(cmd[i].comando, "-size") == 0) {
            size = true;
            tamanio = atoi(cmd[i + 1].comando);
            if (tamanio > 0) {
            } else {
                error = true;
                e = i;
                printf("ERROR! SOLO VALORES POSIVITOS MAYORES QUE 0\n");
                i = 20;
            }
        } else if (strcasecmp(cmd[i].comando, "unit") == 0 || strcasecmp(cmd[i].comando, "+unit") == 0) {
            unit = true;
            strcpy(uni, cmd[i + 1].comando);
            printf("LA UNIDAD DEL DISCO SERA:%s\n", uni);
            opcional = false;
            if (strcasecmp(uni, "k") == 0 || strcasecmp(uni, "m") == 0) {
                //PARAMETROS CORRECTOS
            } else {
                printf("ERROR! UNIDAD NO VALIDA\n");
                error = true;
                e = i;
                i = 20;
            }
        } else if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0) {
            path = true;
            strcpy(pat, cmd[i + 1].comando);
            printf("LA PATH DEL DISCO SERA: %s \n", pat);
            limpiarvariables(auxpath, 100);
            if (pat[0] == '\"') {
                int a;
                //CONCATENANDO TODA LA PATH COMPLETA
                for (a = i + 2; a < 25; a++) {
                    strcpy(auxpath, cmd[a].comando);
                    int b = contarcaracteres(auxpath);
                    if (auxpath[b - 1] != '\"') {
                        strcat(pat, " ");
                        strcat(pat, auxpath);
                        //limpiarvariables(auxpath, 100);
                    } else {
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
                for (l = 1; l < 100; l++) {
                    if (pat[l] == '\"') {
                        l = 100;
                    } else {
                        auxpath[cont] = pat[l];
                        cont++;
                    }
                }
                printf("LA CARPETA COMPLETA ES:%s\n", auxpath);
                //existepath(auxpath);
            } else {
                strcpy(auxpath, pat);
                //existepath(auxpath);
            }
        }else if(strcasecmp(cmd[i].comando,"-name")==0){
            char auxnombre[25];
            limpiarvariables(auxnombre,25);
            strcpy(auxnombre,cmd[i+1].comando);
            printf("EL NOMBRE DEL ARCHIVO SERA %s\n",auxnombre);
            char* token;
            token = strtok(auxnombre, "."); /*There are two delimiters here*/
            bool encontropunto = false;
            while (token != NULL) {
                if(strcasecmp(token,"dsk")==0){
                    encontropunto=true;
                    name=true;
               }
                token = strtok(NULL, ".");
            }
            if(encontropunto==true){
                strcpy(nombre,cmd[i+1].comando);
            }else{
                printf("ERROR!!! EL DISCO PARA CREAR NO CUENTA CON UNA EXTENSION ACEPTABLE\n");
              error=true;
                    e=i;
                    i=20;
            }

        }
        i++;
    }
    if (opcional == true) {
        strcpy(uni, "m");
    }

    if (size == false || path == false || error == true || name==false) {
        printf("ERROR EN EL COMANDO %s:%s\n", cmd[e].comando, cmd[e + 1].comando);

    } else {
        strcat(auxpath,"/");
        strcat(auxpath,nombre);
        existepath(auxpath);
        if (strcasecmp(uni, "k") == 0) {
            crearMBR(auxpath, tamanio * 1024);
        } else if (strcasecmp(uni, "m") == 0) {
            crearMBR(auxpath, (tamanio * 1024)*1024);
        }
    }
}

void crearMBR(char* nombreArchivo, int tamanio) {
    FILE *tmp;
    if ((tmp = fopen(nombreArchivo, "rb"))) {
        printf("DISCO YA EXISTE\n");
        fclose(tmp);
    } else {
        FILE *disco;
        if ((disco = fopen(nombreArchivo, "wb"))) {
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
            for (i = 0; i < 4; i++) {
                mbr.particiones[i] = particion;
            }
            fwrite(&mbr, sizeof (MBR), 1, disco);
            fseek(disco, tamanio - 1, SEEK_SET);
            char trash = '\0';
            fwrite(&trash, 1, 1, disco);
            fclose(disco);
        } else {
            printf("IMPOSIBLE CREAR EL DISCO\n");
        }

    }


}

void rmDisk(Comando cmd[]) {
    char pat[100];
    char auxpath[100];
    char con [2];
    printf("DESEA ELIMINAR EL DISCO [Y/N]\n");
    fgets(con, 3, stdin);
    cambio(con);
    limpiarvariables(pat, 100);
    int i = 0;
    if (strcasecmp(con, "y") == 0) {
        while (strcasecmp(cmd[i].comando, "vacio") != 0) {
            if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0) {
                strcpy(pat, cmd[i + 1].comando);
                printf("LA PATH DEL DISCO SERA: %s \n", pat);
                limpiarvariables(auxpath, 100);
                if (pat[0] == '\"') {
                    int a;
                    //CONCATENANDO TODA LA PATH COMPLETA
                    for (a = i + 2; a < 25; a++) {
                        strcpy(auxpath, cmd[a].comando);
                        int b = contarcaracteres(auxpath);
                        if (auxpath[b - 1] != '\"') {
                            strcat(pat, " ");
                            strcat(pat, auxpath);
                            //limpiarvariables(auxpath, 100);
                        } else {
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
                    for (l = 1; l < 100; l++) {
                        if (pat[l] == '\"') {
                            l = 100;
                        } else {
                            auxpath[cont] = pat[l];
                            cont++;
                        }
                    }
                    printf("LA CARPETA COMPLETA ES:%s\n", auxpath);
                    existepath(auxpath);
                } else {
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
    } else {
        printf("OK NO ELIMINA NADA");
    }
}

void existepath(char path[]) {
    char aux [500];
    strcpy(aux, path);
    char cmd[] = "mkdir";
    struct stat datosficheros;
    lstat(aux, &datosficheros);
    if (S_ISREG(datosficheros.st_mode)) {
        printf("YA EXISTE LA CARPETA\n");
    } else {

        /**************COMPROBANDO QUE CADA DIRECCION DEL PATH EXISTA*********************/
        char completo[500];
        strcpy(completo, path);
        char** variables;
        int i = sabercuantosslash(path);
        char value[500];
        //    printf("La canitdad de slash son:%d\n", i);
        //  printf("el comando para parsear el path es:%s\n", aux);
        int limite = i - 1;
        int j;
        int contador = 0;
        strcpy(value, "/home");
        variables = Splitpalabras(aux, '/');

        if (variables) {
            for (j = 0; *(variables + j); j++) {

                char * valor = *(variables + j);
                if (contador > 0) {
                    if (contador < limite) {
                        strcat(value, "/");
                        strcat(value, valor);
                    }
                    if (mkdir(value, S_IRWXU) == 0) {
                        printf("SE CREARA LA CARPETA %s\n", value);
                    }
                }
                contador++;
            }
            free(*(variables + j));
        }
        printf("\n");
        free(variables);
    }
}

int sabercuantosslash(char * path) {
    char * tampath;
    tampath = path;
    int cantidad = 0;
    while (*tampath) {
        if (*tampath == '/') {
            cantidad++;
        }
        tampath++;
    }
    return cantidad;
}

    int main() {
    int i;
    for (i = 0; i < 25; i++) {
        strcpy(VComandos[i].comando, "VACIO");
    }
    int salida = 1;
    while (salida != 0) {
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
        //printf("EL ASCII ES: %d",asci);
        if (asci == 47) {
            pal[final] = ' ';
            fgets(aux, 100, stdin);
            cambio(aux);
        }
        strcat(pal, aux);
        //printf("COMPLETA: %s",pal);
        if (strcasecmp(pal, "exit") == 0) {
            salida = 0;
            retornarfecha();
            printf("SALIENDO DEL SISTEMA DE CREACION DE DISCOS\n");
        } else {
            char* token;
            token = strtok(pal, " ");
            while (token != NULL) {
                strcpy(VComandos[count].comando, token);
                count++;
                token = strtok(NULL, " ::");
            }
            printf("\n");
            printf("\n");
            if (strcasecmp(VComandos[0].comando, "mkdisk") == 0) {
                Mkdisk(VComandos);
            } else if (strcasecmp(VComandos[0].comando, "rmdisk") == 0) {
                rmDisk(VComandos);
            } else if (strcasecmp(VComandos[0].comando, "clear") == 0) {
                system("clear");
            } else {
                printf("COMANDO INVALIDO\n");
            }
            limpiarcomando();
        }
    }
    return 0;
}
