#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

/**
 * Usage
 * tailog file t(mms) [xip|xdom] [dom|ip]
 *  Optimizar con -o 3
 **/

#define READSIZE 100

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

int line_length(char* s){
    int i = 0;
    while(s[i] != '\0'){
        i++;
    }
    return i;
}

int checktime(char* s,struct tm *t){
    char tmp[3];
    tmp[0] = s[0];
    tmp[1] = s[1];
    tmp[2] = '\0';
    if(t->tm_hour == atoi(tmp)){
        tmp[0] = s[3];
        tmp[1] = s[4];
        if(t->tm_min == atoi(tmp)){
            return 0;
        }else if(t->tm_min < atoi(tmp)){
            return 1;
        }else{
            return -1;
        }
    }else if(t->tm_hour < atoi(tmp)){
        return 1;
    }else{
        return -1;
    }
}

/**
 * Returns -1 if the date s is smaller than t
 *          1 if the date s is bigger than t
 *          0 if both are equal
 **/
int checkdate(char* s,struct tm *t){
    // Set to year
    s[4] = '\0';
    if(t->tm_year+1900 == atoi(s)){
        // If ok go ahead with month
        char tmp[3];
        tmp[0] = s[5];
        tmp[1] = s[6];
        tmp[2] = '\0';
        if(t->tm_mon+1 == atoi(tmp)){
            // Month OK
            tmp[0] = s[8];
            tmp[1] = s[9];
            if(t->tm_mday == atoi(tmp)){
                return 0;
            }else if(t->tm_mday < atoi(tmp)){
                // Given day is bigger
                return 1;
            }else{
                // Given day is smaller
                return -1;
            }
        }else if(t->tm_mon+1 < atoi(tmp)){
            // Given month is bigger    
            return 1;
        }else{
            // Given month is smaller
            return -1;
        }
    }else if(t->tm_year+1900 < atoi(s)){
        // Given year is bigger
        return 1;
    }else{
        // Given year is smaller
        return -1;
    }
    return 0;
}

/**
 * Do binary search on a file by looking into a pivot chunk
 * then for the chunk looking at the values before doing recursion
 * @param i leftmost chunk
 * @param j rightmost chunk
 * @param blocksize chunk size
 * @param f openfile
 * @param target time in minutes
 * @return the offset to read the file
 **/
int binaryFind(int i, int j, int blocksize, FILE *f, struct tm *t){

    int pivotOffset = i+(j-i)/2;
    fseek(f, pivotOffset*blocksize, SEEK_SET);
    char buf[READSIZE];
    int chunk_offset = 0;
    printf("AT %i \n",pivotOffset);
    char *tmp, *copy;
    int res, len;
    // Find upperlimit
    while(chunk_offset < blocksize){
        //fgets(test, READSIZE, f);
        printf("HERE: '%s'", fgets(buf, READSIZE, f));
        chunk_offset = chunk_offset + line_length(buf);

        copy = strdup(buf);
        tmp = strsep(&copy,",");
        len = line_length(tmp);
        //If date
        if(len == 10){
            res = checkdate(tmp,t);
            // If ok check time
            if(res == 0){
                tmp = strsep(&copy,",");
                res = checktime(tmp,t);
                if(res == 0){
                    // If we have it and it is at the beginning
                    if(chunk_offset - line_length(buf) == 0){
                        // Go Up the file
                        return binaryFind(i,pivotOffset,blocksize,f,t);
                    }else{
                        // We have it!
                        return chunk_offset - line_length(buf);
                    }
                }
            }
        }

    }
    if(chunk_offset - line_length(buf) == 0){
        // Go Up through the file
    }
    if(chunk_offset >= blocksize){
        // Go Down through the file
    }
    return EXIT_SUCCESS;
}

int check_valid(FILE *f, int filesize, int blocksize, struct tm *t){
    char buf[READSIZE];
    fseek(f,filesize-blocksize,SEEK_SET);
    int chunk_offset = 0;
    char *tmp, *copy;
    int res, len;
    while(chunk_offset < blocksize){
        fgets(buf, READSIZE, f);
        chunk_offset = chunk_offset + line_length(buf);
        if(buf[0] == '\0'){
            break;
        }
        copy = strdup(buf);
        tmp = strsep(&copy,",");
        len = line_length(tmp);
        // If date
        if(len == 10){
            res = checkdate(tmp,t);
            // If ok check time
            if(res == 0){
                tmp = strsep(&copy,",");
                res = checktime(tmp,t);
                if(res == 0){
                    // If same time we are good.
                    break;
                }
            }
        }
    }
    // Give the line offset on the file
    return chunk_offset;
}

int find_position(char *filename, struct tm *t){
    struct stat st;
    FILE *f = fopen(filename,"r");

    if(stat(filename, &st) != 0){
        return EXIT_FAILURE;
    }

    int blocksize = st.st_blksize;
    int filesize = st.st_size;
    int chunks = st.st_blocks;
    printf("Filesize %i, Blocksize %i, Blocks %i \n",filesize,blocksize,chunks);

    // Check if at the last block we have data
    int last_block_offset = check_valid(f,filesize,blocksize,t);
    if(last_block_offset == blocksize || last_block_offset == 0){
        printf("No data\n");
        return EXIT_FAILURE;
    }

    // Look for upper limit
    //int position = binaryFind(0, chunks, blocksize, f, t);
    printf("blocks division %f\n",(double)filesize/(double)blocksize);
    fclose(f);
    //return position;
    return EXIT_SUCCESS;
}

int do_ips(char **argv, struct tm *t){
    int result = find_position(argv[1], t);
    if(result == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int do_doms(char **argv,struct tm *t){
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
        printf("Usage: tailog file t(minutes) [xip|xdom] [dom|ip]\n");
        return EXIT_FAILURE;
    }
    // Get our time
    time_t t = time(NULL) - atoi(argv[2])*60;
    struct tm *timeinfo = localtime(&t);
    // Example test
    timeinfo->tm_year = 116;
    timeinfo->tm_mon = 10;
    timeinfo->tm_mday = 28;
    timeinfo->tm_hour = 10;
    timeinfo->tm_min = 59;
    printf ("Looking for time and date: %i-%i-%i,%i:%i:%i\n", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,timeinfo->tm_sec);

    // Specific search
    if(argc > 3){
        // IP
        if(strcmp(argv[3],options[0]) == 0){
            printf("Print IPs\n");
            return do_ips(argv,timeinfo);
        // Dom
        }else if(strcmp(argv[3],options[1]) == 0){
            printf("Print Doms\n");
            return do_doms(argv,timeinfo);
        }else{
            printf("Option not valid!\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}