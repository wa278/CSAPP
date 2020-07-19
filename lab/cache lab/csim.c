/*
 write by wang ao
     2020 04 14
 */
#include "cachelab.h"
#include "getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
char* option_string = "hvs:E:b:t:";
void usage();
typedef struct Line {
    int valid;
    int tag;
    long counter;
}line;
int main(int argc, char* argv[])
{
    int ch;
    int display_trace = 0;
    int s = 0;
    int E = 0;
    int b = 0;
    char trace_file[20] = {0};
    char* tf = trace_file;
    while ((ch = getopt(argc, argv, option_string)) != -1) {
        switch (ch) {
            case 'v':
                display_trace = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
		        E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tf = optarg;
                break;
            case 'h':
            default:
                usage();
        }
    }
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL){
        usage();
    }
    int S = (int) (pow(2,s));
    line** mem = (line**) malloc(sizeof(line*)*S);
    for(int i = 0 ; i < S; i++){
        mem[i] = malloc(sizeof(line)*E);
    }
    for (int i = 0 ; i < S; i++){
        for (int j = 0; j < E; j++){
            mem[i][j].valid = 0;
            mem[i][j].counter = 0;
        }
    }

    FILE* fp = fopen(tf, "r");
    int re;
    char op;
    unsigned long address;
    int size;
    unsigned long num = 1;
    int hit = 0;
    int miss = 0;
    int evic = 0;

    while((re = fscanf(fp,"%c %lx,%d",&op,&address,&size))!=EOF){
        int set = ((address<<(64-b-s))>>(64-s));
        int tag = (address>>(s+b));
        int isHit = 0;
        int hasEmpty = 0;
        char message[30] = {0};
        char* str = message;
        sprintf(str,"%c %lx,%d",op,address,size);
        switch(op){
            case 'I':
                break;
            case 'L':
                for (int i = 0 ; i < E; i++){
                    if(mem[set][i].valid == 1 && mem[set][i].tag == tag){
                        hit++;
                        strcat(str," hit");
                        mem[set][i].counter = num;
                        isHit = 1;
                        break;
                    }
                }
                if (isHit){
                    if(display_trace) {
                        printf("%s \n",str);
                    }
                    break;
                }else{
                    strcat(str, " miss");
                    miss++;
                }
                for (int i = 0; i < E; i++){
                    if (mem[set][i].valid == 0){
                        mem[set][i].valid = 1;
                        mem[set][i].tag = tag;
                        mem[set][i].counter = num;
                        hasEmpty = 1;
                        break;
                    }
                }
                if (!hasEmpty){
                    int min = 0;
                    for (int i = 1; i < E; i++){
                        if (mem[set][i].counter < mem[set][min].counter){
                            min = i;
                        }
                    }
                    mem[set][min].tag = tag;
                    mem[set][min].counter = num;
                    strcat(str, " eviction");
                    evic++;
                }
                if(display_trace) {
                    printf("%s \n",str);
                }
                break;
            case 'S':
                for (int i = 0 ; i < E; i++){
                    if(mem[set][i].valid == 1 && mem[set][i].tag == tag){
                        hit++;
                        strcat(str," hit");
                        mem[set][i].counter = num;
                        isHit = 1;
                        break;
                    }
                }
                if (isHit){
                    if(display_trace) {
                        printf("%s \n",str);
                    }
                    break;
                }else{
                    miss++;
                    strcat(str, " miss");
                }
                for (int i = 0; i < E; i++){
                    if (mem[set][i].valid == 0){
                        mem[set][i].valid = 1;
                        mem[set][i].tag = tag;
                        mem[set][i].counter = num;
                        hasEmpty = 1;
                        break;
                    }
                }
                if (!hasEmpty){
                    int min = 0;
                    for (int i = 1; i < E; i++){
                        if (mem[set][i].counter < mem[set][min].counter){
                            min = i;
                        }
                    }
                    mem[set][min].tag = tag;
                    mem[set][min].counter = num;
                    evic++;
                    strcat(str, " eviction");
                }
                if(display_trace) {
                    printf("%s \n",str);
                }
                break;
            case 'M':
                for (int i = 0 ; i < E; i++){
                    if(mem[set][i].valid == 1 && mem[set][i].tag == tag){
                        hit++;
                        strcat(str," hit");
                        mem[set][i].counter = num;
                        isHit = 1;
                        break;
                    }
                }
                if (isHit){
                    hit++;
                    strcat(str," hit");
                    if(display_trace) {
                        printf("%s \n",str);
                    }
                    break;
                }else{
                    miss++;
                    strcat(str, " miss");
                }

                for (int i = 0; i < E; i++){
                    if (mem[set][i].valid == 0){
                        mem[set][i].valid = 1;
                        mem[set][i].tag = tag;
                        mem[set][i].counter = num;
                        hasEmpty = 1;
                        break;
                    }
                }
                if (!hasEmpty){
                    int min = 0;
                    for (int i = 1; i < E; i++){
                        if (mem[set][i].counter < mem[set][min].counter){
                            min = i;
                        }
                    }
                    mem[set][min].tag = tag;
                    mem[set][min].counter = num;
                    evic++;
                    strcat(str, " eviction");
                }
                hit++;
                strcat(str," hit");
                if(display_trace) {
                    printf("%s \n",str);
                }
                break;
            default:
                break;
        }
        num++;
    }
    printSummary(hit, miss, evic);
    for(int i = 0; i < S; i++){
        free(mem[i]);
    }
    free(mem);
    return 0;
}
void usage(){
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file> \n");
    printf("Options: \n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
    exit(0);
}
