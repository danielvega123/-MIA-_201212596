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
void mount(Comando cmd[]) {
    bool name = false;
    bool pat = false;
    char auxpath[100];
    char path[100];
    char nombre[25];
    limpiarvariables(auxpath, 100);
    limpiarvariables(path, 100);
    limpiarvariables(nombre, 25);
    int i = 0;
    while (strcasecmp(cmd[i].comando, "vacio") != 0) {
        if (strcasecmp(cmd[i].comando, "path") == 0 || strcasecmp(cmd[i].comando, "-path") == 0) {
            pat = true;
            strcpy(path, cmd[i + 1].comando);
            printf("LA PATH DEL DISCO SERA: %s \n", path);
            limpiarvariables(auxpath, 100);
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
            } else {
                strcpy(auxpath, path);
            }
        } else if (strcasecmp(cmd[i].comando, "name") == 0 || strcasecmp(cmd[i].comando, "-name") == 0) {
            name = true;
            strcpy(nombre, cmd[i + 1].comando);
            printf("EL NOMBRE PARA MONTAR SERA:%s\n", nombre);
        }
        i++;
    }
     if (name == true && pat == true) {
        //ACCIONES PARA MONTAR UNA PARTICION
        }
}

void fdisk(Comando cmd[]) {
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
    limpiarvariables(auxpath, 100);
    limpiarvariables(deletee, 5);
    limpiarvariables(nombre, 100);
    limpiarvariables(pat, 100);
    limpiarvariables(unidad, 2);
    limpiarvariables(tipo, 5);
    limpiarvariables(fitt, 5);
    int i = 0;

    while (strcasecmp(cmd[i].comando, "vacio") != 0) {
        if (strcasecmp(cmd[i].comando, "name") == 0 || strcasecmp(cmd[i].comando, "-name") == 0) {
            name = true;
            strcpy(nombre, cmd[i + 1].comando);
            printf("EL NOMBRE DE LA PARTICION SERA:%s\n", nombre);
        } else if (strcasecmp(cmd[i].comando, "unit") == 0 || strcasecmp(cmd[i].comando, "+unit") == 0) {
            unit = true;
            strcpy(unidad, cmd[i + 1].comando);
            existeunidad = true;
            printf("LA UNIDAD DEL DISCO SERA:%s\n", unidad);
            if (strcasecmp(unidad, "k") == 0) {
                uni = 'K';
            } else if (strcasecmp(unidad, "m") == 0) {
                uni = 'M';
            } else if (strcasecmp(unidad, "b") == 0) {
                uni = 'B';
            } else {
                printf("ERROR LA UNIDAD ES %s INCORRECTA SOLO ADMITE [M|B|K]\n", unidad);
                error = true;
            }
        } else if (strcasecmp(cmd[i].comando, "type") == 0 || strcasecmp(cmd[i].comando, "+type") == 0) {
            type = true;
            strcpy(tipo, cmd[i + 1].comando);
            printf("EL TIPO DE PARTICION SERA:%s\n", tipo);
            if (strcasecmp(tipo, "p") == 0) {
                typec = 'P';
            } else if (strcasecmp(tipo, "l") == 0) {
                typec = 'L';
            } else if (strcasecmp(tipo, "e") == 0) {
                typec = 'E';
            } else {
                error = true;
                printf("TIPO DE PARTICION NO VALIDA\n");
            }
        } else if (strcasecmp(cmd[i].comando, "fit") == 0 || strcasecmp(cmd[i].comando, "+fit") == 0) {
            fit = true;
            strcpy(fitt, cmd[i + 1].comando);
            printf("EL NOMBRE DE LA PARTICION SERA:%s\n", fitt);
            if (strcasecmp(fitt, "w") == 0 || strcasecmp(fitt, "wf") == 0) {
                adjust = 'W';
            } else if (strcasecmp(fitt, "b") == 0 || strcasecmp(fitt, "bf") == 0) {
                adjust = 'B';
            } else if (strcasecmp(fitt, "f") == 0 || strcasecmp(fitt, "ff") == 0) {
                adjust = 'F';
            } else {
                printf("ERROR EL AJUSTE %s NO ES VALIDO [B-W-F]\n", fitt);
                error = true;
            }
        } else if (strcasecmp(cmd[i].comando, "delete") == 0 || strcasecmp(cmd[i].comando, "+delete") == 0) {
            delete = true;
            strcpy(deletee, cmd[i + 1].comando);
            printf("LA PARTICION A ELIMINAR SERA:%s\n", deletee);
        } else if (strcasecmp(cmd[i].comando, "add") == 0 || strcasecmp(cmd[i].comando, "+add") == 0) {
            add = true;
            agregar = atoi(cmd[i + 1].comando);
            printf("EL ESPACIO A AGREGAR SERA:%d\n", agregar);
        } else if (strcasecmp(cmd[i].comando, "size") == 0 || strcasecmp(cmd[i].comando, "-size") == 0) {
            tamanio = atoi(cmd[i + 1].comando);
            if (tamanio > 0) {
                size = true;
                printf("EL TAMANIO DE LA PARTICION SERA:%d\n", tamanio);
            } else {
                error = true;
                printf("NO SE ADMITE VALORES NEGATIVOS\n");
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
                        if (strcasecmp(auxpath, "vacio") == 0) {
                            a = 25;
                        } else {
                            strcat(pat, " ");
                            strcat(pat, auxpath);
                        }
                    } else {
                        strcat(pat, " ");
                        strcat(pat, auxpath);
                        a = 25;
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
            } else {
                strcpy(auxpath, pat);
            }
        }
        i++;
    }

    /*********************CREANDO PARTICIONES*************/

    if (size == true && path == true && name == true && error == false && ejecuto == false) {
        ejecuto = true;
        int tamanoreal;
        if (existeunidad == false) {
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
        if (archivo != NULL) {
            if ((comienzo = ftell(archivo)) < 0) {
                printf("ERROR: ftell no ha funcionado\n");
            } else {
                    if (type == false) {
                        typec = 'P';
                    }
                    if (fit == false) {
                        adjust = 'W';
                    }
                    if (unit == false) {
                        uni = 'K';
                        tamanoreal = tamanio * 1024;
                    } else {
                        if (uni == 'K' || uni == 'k') {
                            tamanoreal = tamanio * 1024;
                        } else if (uni == 'B' || uni == 'b') {
                            tamanoreal = tamanio;
                        } else if (uni == 'M' || uni == 'm') {
                            tamanoreal = (tamanio * 1024)*1024;
                        }

                    }
                   //METODO PARA CREAR PARTICIONES
                            }
        }
    } else {
        if (delete == true && name == true && path == true && error == false) {

        } else {
            if (path == true && name == true && add == true && error == false) {

            } else {
                printf("FALTA PARAMETRO SIZE, PATH O NAME - IMPOSIBLE CREAR PARTICION\n");

            }
        }
    }


    /************************ELIMINAR PARTICION***********************************/
    if (ejecuto == false) {
        if (delete == true && name == true && path == true && error == false) {
          //METODO PARA ELIMINAR PARTICIONES
            ejecuto = true;
        } else {
            if (path == true && name == true && add == true && error == false) {

            } else {
                printf("IMPOSIBLE ELIMINAR LA PARTICION FALTA PARAMETRO PATH, DELETE O NOMBRE DE LA PARTICION\n");

            }

        }
    }
    /**************************AGREGAR ESPACIO************************************/
    if (ejecuto == false) {
        if (path == true && name == true && add == true && error == false) {
            int valorreal;
            if (unit == false) {
                uni = 'K';
                valorreal = agregar * 1024;
            } else {

                if (uni == 'b' || uni == 'B') {
                    valorreal = agregar;
                } else if (uni == 'm' || uni == 'M') {
                    valorreal = (agregar * 1024)*1024;
                } else if (uni == 'k' || uni == 'K') {
                    valorreal = (agregar * 1024);
                }
            }
          //METODO PARA GREGAR ESPACIO
        } else {
            printf("ERROR FALTA PARAMETRO [NAME-PATH-ADD]\n");
        }
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
    struct stat datosficheros;
    lstat(aux, &datosficheros);
    if (S_ISREG(datosficheros.st_mode)) {
        printf("YA EXISTE LA CARPETA\n");
    } else {
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
        while(token!=NULL){
               if (contador > 0) {
                    if (contador < limite) {
                        strcat(value, "/");
                        strcat(value, token);
                    }
                    if (mkdir(value, S_IRWXU) == 0) {
                        printf("SE CREARA LA CARPETA %s\n", value);
                    }
                }
                contador++;
                token = strtok(NULL,"/");
        }
        printf("\n");
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
        if (asci == 92) {
            pal[final] = ' ';
            fgets(aux, 100, stdin);
            cambio(aux);
        }
        strcat(pal, aux);
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
            } else if (strcasecmp(VComandos[0].comando, "fdisk") == 0) {
                fdisk(VComandos);
            }  else if (strcasecmp(VComandos[0].comando, "mount") == 0) {
                mount(VComandos);
            } else {
                printf("COMANDO INVALIDO\n");
            }
            limpiarcomando();
        }
    }
    return 0;
}
