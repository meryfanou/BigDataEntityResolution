#include "../include/pretty_prints.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

ppa* ppa_create(char* type){

    ppa* newppa = malloc(sizeof(ppa));

    if(strcmp(type, "LARGE") == 0){
        newppa->type = strdup("LARGE");
        newppa->full_length = LARGE;
    }

    else if(strcmp(type, "MEDIUM") == 0){
        newppa->type = strdup("MEDIUM");
        newppa->full_length = MEDIUM;
    }


    else if(strcmp(type, "SMALL") == 0){
        newppa->type = strdup("SMALL");
        newppa->full_length = SMALL;
    }

    newppa->middle = (newppa->full_length) / 2;
    newppa->white_space_left = ((newppa->full_length - 4) / 2) -1;
    newppa->white_space_right = newppa->white_space_left;
    newppa->curr = 0;

    // printf("full l: %d\n", newppa->full_length);
    // printf("mid: %d\n", newppa->middle);
    // printf("white_left: %d\n", newppa->white_space_left);


    return newppa;
}


void ppa_destroy(ppa* pp){
    free(pp->type);
    free(pp);
}

void ppa_print_start(ppa* pp, char* title){

    printf("\n\n\n");
    ppa_add_full_line(pp);
    ///////

    printf("      ||");
    int empty = pp->full_length - 4;
    printf(YEL);
    ppa_put_at_mid(pp, empty, title);
    printf(RESET);
    printf("||\n");
    

    //////////
    ppa_add_full_line(pp);

}

void ppa_put_at_mid(ppa* pp, int full, char* line){
    // printf("curr: %d\n", pp->curr);
    int target_empty = (((full  - strlen(line))/2));
    
    int small = 0;

    while(pp->curr % 8 != 0){
        printf(" ");
        pp->curr++;
        small++;
    }
    

    ppa_add_blanks(target_empty - small);
    pp->curr += target_empty- small;
    
    printf("%s", line);
    pp->curr += strlen(line);

    small = 0;
    while(pp->curr % 8 != 0){
        printf(" ");
        pp->curr++;
        small++;
    }
    

    if((full - strlen(line)) %2 == 0){
        ppa_add_blanks(target_empty - small);
    }
    else{
        ppa_add_blanks(target_empty+1 - small);
        pp->curr ++;
    }
    pp->curr += target_empty - small;
}

void ppa_add_full_line(ppa* pp){

     printf(RESET "      ||");
    int i = 0;
    while(i < pp->full_length-4){
        printf("=");
        i++;
    }
    printf("||\n");
}

void ppa_add_line_left(ppa* pp, char* line){

    pp->curr = 0;
    printf("      ||");

    printf(CYN);
    ppa_put_at_mid(pp, pp->white_space_left, line);
    printf(RESET);
    printf("||");
    pp->curr +=2;
    // printf("\n\n\n\n");
    fflush(stdout);

}

void ppa_add_line_right(ppa* pp, char* line, char* clr){
    // printf("\b\b\b\b");
    printf(clr);
    ppa_put_at_mid(pp, pp->white_space_right, line);
    printf(RESET);
    printf("||\n");
    pp->curr = 0;
}

void ppa_add_blanks(int total){
    int tabs = total / 8;
    int spaces = total % 8;

    int i = 0;
    while(i < tabs){
        printf("\t");
        i++;
    }
    i = 0;
    while(i < spaces){
        printf(" ");
        i++;
    }        
}

void ppa_print_end(ppa* pp, char* line){
    ppa_add_full_line(pp);

    if(line != NULL){
        printf("      ||");
        int empty = pp->full_length - 4;
        printf(YEL);
        ppa_put_at_mid(pp, empty, line);
        printf(RESET);
    }

    printf("||\n");
    ppa_add_full_line(pp);
    printf("\n\n");
}

// int main(void){
//     ppa* pp = ppa_create("MEDIUM");

//     ppa_print_start(pp, "PROGRAM STATUS");

//     ppa_add_line_left(pp, "mple");
//     ppa_add_line_right(pp, "roz");

//     ppa_add_line_left(pp, "mple");
//     ppa_add_line_right(pp, "MPOMPA");

//     ppa_add_line_left(pp, "SKATAAAAAA");
//     ppa_add_line_right(pp, "rozzzzzzzz");

//     ppa_print_end(pp);

//     ppa_destroy(pp);
//     return 0;
// }