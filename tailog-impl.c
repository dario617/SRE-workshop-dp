#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Usage
 * tailog file t(mms) [xip|xdom] [dom|ip]
 *  Optimizar con -o 3
 * 
 * Comando completo es :
 *      grep "info_a_filtrar" | awk -F, ""
 **/

//fseek

/**
 * Estrategia:
 * Leer el archivo de abajo hacia arriba, guardando un contador con la pos.
 * Cerrar el archivo y abrirlo desde el offset.
 */

char *options[] = {
    "xip",
    "xdom"
};

char *command[] = {
    "tail",
    "awk -F, '{print $3}'", // Show only ips
    "awk -F, '{print $4}'", // Show only doms
    "grep", // Find the specific ip or dom
    "|"
};

int do_doms(char **argv){
    char** args = malloc(2 * sizeof(char*));
    printf("%s",argv[1]);
    args[0] = argv[1];
    args[1] = NULL;
    printf("%s",command[0]);
    if(execvp(command[0],args) == -1){
        printf("Failed\n");
        free(args);
        return EXIT_FAILURE;
    }
    free(args);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv){

    // Error checking
    if(argc < 3){
        printf("Not enough arguments!\n");
        printf("Usage: tailog file t(mms) [xip|xdom] [dom|ip]\n");
        return EXIT_FAILURE;
    }

    // Specific search
    if(argc > 3){
        // IP
        if(strcmp(argv[3],options[0]) == 0){
            printf("Print IPs\n");

        // Dom
        }else if(strcmp(argv[3],options[1]) == 0){
            printf("Print Doms\n");
            return do_doms(argv);
        }else{
            printf("Option not valid!\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}