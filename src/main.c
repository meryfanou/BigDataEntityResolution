#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../include/mySpec.h"
#include "../include/myMatches.h"
#include "../include/myHash.h"

#define PATH_X "../camera_specs/2013_camera_specs/"
#define PATH_W "../sigmod_large_labelled_dataset.csv"

int main(void){

    int             propNum = -1;
    char            line[512];
    char            *dirpath = NULL, *filepath = NULL;
    char            ***properties = NULL;
    DIR             *datasetX = NULL, *sitePtr = NULL;
    FILE            *specFd = NULL;
    struct dirent   *siteDir = NULL, *specFile = NULL;

    // Open datasetX
    if((datasetX = opendir(PATH_X)) == NULL){
        perror("opendir");
        exit(-1);
    }

    // Read from datasetX
    while((siteDir = readdir(datasetX)) != NULL){
        // Ignore . and ..
        if(!strcmp(siteDir->d_name, ".") || !strcmp(siteDir->d_name, ".."))
            continue;

        // Keep the full path
        dirpath = (char*)malloc(strlen(PATH_X) + strlen(siteDir->d_name) + 1);
        strcpy(dirpath,PATH_X);
        strcat(dirpath, siteDir->d_name);

        // Open current directory (website)
        if((sitePtr = opendir(dirpath)) == NULL){
            perror("opendir");
            exit(-1);
        }

        // Read the files in the directory
        while((specFile = readdir(sitePtr)) != NULL){
            // Ignore . and ..
            if(!strcmp(specFile->d_name, ".") || !strcmp(specFile->d_name, ".."))
                continue;

            // Keep the full path
            filepath = (char*)malloc(strlen(dirpath) + strlen("/") + strlen(specFile->d_name) + 1);
            strcpy(filepath, dirpath);
            strcat(filepath, "/");
            strcat(filepath, specFile->d_name);

            // Open current file (spec)
            if((specFd = fopen(filepath, "r")) == NULL){
                perror("fopen");
                exit(-2);
            }

            char    *filename = strdup(specFile->d_name);
            char    *specID = NULL;

            // Keep specID (<website_name>//<spec_num>)
            filename = strtok(filename, ".");
            specID = malloc(strlen(siteDir->d_name) + strlen("//") + strlen(filename) + 1);
            strcpy(specID, siteDir->d_name);
            strcat(specID, "//");
            strcat(specID, filename);
            free(filename);

            // Read spec's properties
            while(fgets(line, sizeof(line), specFd) != NULL){
                // Ignore '{' and '}' lines in json file
                if(line[0] == '{' || line[0] == '}')
                    continue;

                char    *rest = line;
                char    *token = NULL;
                int     index = 0;

                // Seperate the key from the value in the ("key" : "value") pair
                token = strtok_r(rest, ":", &rest);
                while(index < 2){
                    char    *temp = NULL, *str = NULL, *extra = NULL;

                    // Remove extra \" and , from key string
                    if(index == 0){
                        while((temp = strtok_r(token, ",\"", &token)))
                            str = temp;
                    }   // Remove extra \" and , from value string
                    else{
                        //if(!strcmp(specID, "www.ebay.com//48630"))
                        //    printf("here\n");
                        int     need_extra = 0;

                        // If the string does not end in  \",\n   or  \"\n then there is more to read from the file
                        if(strlen(rest) > 2 && (rest[strlen(rest)-3] != '\"' || rest[strlen(rest)-2] != ',' || rest[strlen(rest)-1] != '\n') && (rest[strlen(rest)-2] != '\"' || rest[strlen(rest)-1] != '\n'))
                            need_extra = 1;

                        // If it is a string
                        if(strlen(rest) > 1 && (rest[0] == '\"' || rest[1] == '\"')){
                            char    *str_all = NULL;

                            while((temp = strtok_r(rest, "\"", &rest)))
                                if(strcmp(temp, ",") && strcmp(temp, ",\n"))
                                {
                                    if(str_all == NULL){
                                        str_all = strdup(temp);
                                    }
                                    else{
                                        int len = strlen(str_all);
                                        str_all = realloc(str_all, len + strlen(temp) + 1);
                                        strcat(str_all, temp);
                                    }
                                }

                            temp = strdup(str_all);
                            str = temp;
                            free(str_all);
                            str_all = NULL;
                        }   // If it is an 'array' of strings
                        else if(strlen(rest) > 1 &&  (rest[0] == '[' || rest[1] == '[')){
                            char    *temp = strdup(rest);
                            int     len = strlen(rest);

                            str = temp;
                            // Until the end of the 'array' (string is "]" or ends in either "]\n" or "],\n")
                            while((len == 1 && rest[0] != ']') || (len == 2 && rest[0] != ']' && rest[1] != ']') || (len >= 3 && !(rest[len-2] == ']' && rest[len-1] == '\n') && !(rest[len-3] == ']' && rest[len-2] == ',' && rest[len-1] == '\n'))){
                                fgets(line, sizeof(line), specFd);
                                extra = realloc(extra, strlen(str) + strlen(line) + 1);
                                strcpy(extra, str);
                                rest = line;

                                strcat(extra, rest);
                                free(temp);
                                temp = strdup(extra);
                                str = temp;
                                len = strlen(rest);
                            }

                            // Remove extra ','
                            if(extra[strlen(extra)-2] == ',' && extra[strlen(extra)-3] == ']')
                                extra[strlen(extra)-2] = extra[strlen(extra)-1];

                            need_extra = 0;
                            str = extra;
                            free(temp);
                        }

                        // If string's length is greater than sizeof(line)
                        while(need_extra){
                            fgets(line, sizeof(line), specFd);
                            rest = line;

                            // If it is the end of the string
                            if(!strcmp(rest, "\"\n") || !strcmp(rest, ",\n") || !strcmp(rest, "\",\n")){
                                if(extra != NULL)
                                    free(extra);

                                extra = strdup(str);
                                need_extra = 0;
                                break;
                            }

                            need_extra = 0;
                            // Check if there is still more to read (string does not end in  \",\n )
                            if(strlen(rest) > 2 && (rest[strlen(rest)-3] != '\"' || rest[strlen(rest)-2] != ',' || rest[strlen(rest)-1] != '\n'))
                                need_extra = 1;

                            extra = realloc(extra, strlen(str) + strlen(rest) + 1);
                            strcpy(extra, str);

                            if(str == temp){
                                free(temp);
                                temp = NULL;
                            }

                            str = "";
                            while((temp = strtok_r(rest, "\"", &rest)))
                                if(strcmp(temp, ",") && strcmp(temp, ",\n"))
                                    str = temp;

                            strcat(extra, str);
                            temp = strdup(extra);
                            str = temp;
                        }

                        if(extra != NULL){
                            if(str == temp){
                                free(temp);
                                temp = NULL;
                            }
                            str = extra;
                        }
                    }

                    // Will save key,value in a 2D array
                    if(index == 0){
                        propNum++;
                        properties = realloc(properties, (propNum+1)*sizeof(char**));
                        properties[propNum] = malloc(2*sizeof(char*));
                    }

                    //printf("%s\n", specID);
                    properties[propNum][index] = strdup(str);

                    if(str == temp){
                        free(temp);
                        temp = NULL;
                    }
                    if(extra != NULL){
                        free(extra);
                        extra = NULL;
                    }
                    index++;
                }
            }
            propNum++;

            // Create spec node
            mySpec *newSpec = specInit(specID, &properties, propNum);
            //printSpec(newSpec);
            deleteSpec(newSpec);

            fclose(specFd);
            free(filepath);
            filepath = NULL;

            free(specID);
            for(int i=0; i<propNum; i++){
                free(properties[i][0]);
                free(properties[i][1]);
                free(properties[i]);
            }
            free(properties);
            properties = NULL;
            propNum = -1;
        }

        closedir(sitePtr);
        free(dirpath);
        dirpath = NULL;
    }

    closedir(datasetX);

    return 0;
}