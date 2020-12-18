/* Logistic Regrassion  .c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <string.h>

#include "../include/logistic.h"
#include "../include/functs.h"
#define PATH "./Outputs"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

logM* logistic_create(){
    logM* newModel = malloc(sizeof(logM));

    newModel->size_totrain = 0;
    newModel->weights_count = 0;
    newModel->trained_times = 0;

    newModel->finalWeights = weights_create();

    return newModel;
}

void logistic_destroy(logM* model){
    
    weights_destroy(model->finalWeights);
    free(model);

}

int logistic_fit(logM* model, int vector_rows, int vector_cols, float** vector , int* labels){
        // set model
    model->size_totrain = vector_rows;

        // set weights
    weights_set(model->finalWeights, vector_cols);
    model->weights_count = vector_cols;

        // train
    return logistic_regression(model, vector, vector_rows, vector_cols, labels);

}

int logistic_fit_spars(logM* model, int spars_size, float** spars, int* labels, int labels_size, int dimensions){
        // set model
    model->size_totrain = labels_size;

        // set weights
    weights_set(model->finalWeights, dimensions);
    model->weights_count = dimensions;

        // train
    return logistic_regression_spars(model, spars, spars_size, labels, labels_size, dimensions);

}

int logistic_regression(logM* model, float** vector, int vector_rows, int vector_cols, int* tags){

    // Signal handling
    struct sigaction    act;
    sigset_t            block_mask;
    int                 received_signal = 0;
    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    printf("vector_rows: %d, vector_cols: %d\n", vector_rows, vector_cols);

    float limit = 1.000;

    model->trained_times = 1;
    while(limit > model->finalWeights->limit){

        if(received_signal == 1)
            return -1;

            // 1. Build predicts table
        float* predicts = logistic_predict_proba(model, vector, vector_rows, vector_cols);

            // 2. Calc weights
                // 2.1 Build Missed Table
        float b_grad = 0.0;
        float* missed_by = malloc(vector_rows*sizeof(float));
        int i = 0;
        while(i < vector_rows){
            if(received_signal == 1){
                // FREE MEM
                free(predicts);
                free(missed_by);
                return -1;
            }

            missed_by[i] = predicts[i] - (float) tags[i];
            b_grad += missed_by[i];
            // printf("my_pred: %.4f, target: %d\n", predicts[i], tags[i]);
            i++;
        }

        b_grad /= (float) i;

                // 2.2 Calc Grad - PER WEIGHT !!!
        float* grad = malloc((vector_cols+1)*sizeof(float));
        grad[0] = b_grad;

        int y = 0;
        while(y < vector_cols){
            if(received_signal == 1){
                // FREE MEM
                free(predicts);
                free(missed_by);
                free(grad);
                return -1;
            }

            int x = 0;
            float magic_num = 0.0;
            while(x < vector_rows){
                if(received_signal == 1){
                    // FREE MEM
                    free(predicts);
                    free(missed_by);
                    free(grad);
                    return -1;
                }

                magic_num += vector[x][y] * missed_by[x];
                x++;
            }
            grad[1+y] = magic_num;
            y++;
        }

            // 3 Update Weights
        weights_update(model->finalWeights, grad, vector_cols);
        limit = active_mean(missed_by, vector_rows);
        printf("\t\t~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("Time: %d\n", model->trained_times);
        printf("limit: %.4f\n", limit);
        // weights_print(model->finalWeights);
        

        model->trained_times++;

        // ΜAX ITERATIONS
        if(model->trained_times > 200)
            limit = 0.0;
        

        // FREE MEM
        free(predicts);
        free(missed_by);
        free(grad);
    }

    if(received_signal == 1)
        return -1;

    // printf("limit: %.4f\n", limit);
    // TESTS
    // printf("FINISHED !!!\n");
    // weights_print(model->finalWeights);
    int* final_predicts = logistic_predict(model, vector, vector_rows, vector_cols);
    printf("\tScore after train: %.4f\n", logistic_score(model, final_predicts, tags, vector_rows));
    free(final_predicts);

    return 0;
}


int logistic_regression_spars(logM* model, float** spars, int spars_size, int* tags, int tags_size, int dimensions){

    // Signal handling
    struct sigaction    act;
    sigset_t            block_mask;
    int                 received_signal = 0;
    sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;
    act.sa_handler = sig_int_quit_handler;
    sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGINT);
	sigaddset(&block_mask,SIGQUIT);

    // printf("spars_rows: %d, dimensions: %d\n", spars_size, dimensions);

    float limit = 1.000;

    model->trained_times = 1;
    while(limit > model->finalWeights->limit){

        if(received_signal == 1)
            return -1;

            // 1. Build predicts table
        float* predicts = logistic_predict_proba_spars(model, spars, spars_size, dimensions, tags_size);

            // 2. Calc weights
                // 2.1 Build Missed Table
        float b_grad = 0.0;
        float* missed_by = malloc(tags_size*sizeof(float));
        int i = 0;
        while(i < tags_size){
            if(received_signal == 1){
                // FREE MEM
                free(predicts);
                free(missed_by);
                return -1;
            }

            missed_by[i] = predicts[i] - (float) tags[i];
            b_grad += missed_by[i];
            // printf("my_pred: %.4f, target: %d\n", predicts[i], tags[i]);
            i++;
        }

        b_grad /= (float) i;

                // 2.2 Calc Grad - PER WEIGHT !!!
        float* grad = malloc((dimensions+1)*sizeof(float));
        grad[0] = b_grad;

        int y = 0;
        while(y < dimensions){
            if(received_signal == 1){
                // FREE MEM
                free(predicts);
                free(missed_by);
                free(grad);
                return -1;
            }

            int x = 0;
            float magic_num = 0.0;
            while(x < spars_size){
                if(received_signal == 1){
                    // FREE MEM
                    free(predicts);
                    free(missed_by);
                    free(grad);
                    return -1;
                }

                if(spars[x][1] == (float) y)
                    magic_num += spars[x][2] * missed_by[(int)spars[x][0]];
                x++;
            }

            //
            // I CAN UPDATE WEIGHTS HERE SAVING TIME !!!!
            //
            grad[1+y] = magic_num;
            y++;
        }

            // 3 Update Weights
        weights_update(model->finalWeights, grad, dimensions);
        limit = active_mean(missed_by, tags_size);
        // printf("\t\t~~~~~~~~~~~~~~~~~~~~~~~~\n");
        // printf("Time: %d\n", model->trained_times);
        // printf("limit: %.4f\n", limit);
        // weights_print(model->finalWeights);
        

        model->trained_times++;

        // ΜAX ITERATIONS
        if(model->trained_times > 1)
            limit = 0.0;
        

        // FREE MEM
        free(predicts);
        free(missed_by);
        free(grad);
    }

    if(received_signal == 1)
        return -1;

    // printf("limit: %.4f\n", limit);
    // TESTS
    // printf("FINISHED !!!\n");
    // weights_print(model->finalWeights);
    int* final_predicts = logistic_predict_spars(model, spars, spars_size, dimensions, tags_size);
    printf("\tScore after train: %.4f\n", logistic_score(model, final_predicts, tags, tags_size));
    free(final_predicts);

    return 0;
}


float* logistic_predict_proba(logM* model, float** vector, int vector_rows, int vector_cols){
    if(model->finalWeights == NULL){
        printf("Error - Untrained model !!\n");
        return NULL;
    }

    if(vector_cols > model->weights_count){
        printf("Error ~ Invalid Size Array !!");
        return NULL;
    }

    float* predicts = malloc(vector_rows*sizeof(float*));
    int i = 0;
    while(i < vector_rows){
        predicts[i] = calc_s(model->finalWeights, vector[i]);
        i++;
    }

    return predicts;
}

float* logistic_predict_proba_spars(logM* model, float** spars, int spars_size, int dimensions, int tags_size){
    if(model->finalWeights == NULL){
        printf("Error - Untrained model !!\n");
        return NULL;
    }

    if(dimensions > model->weights_count){
        printf("Error ~ Invalid Size Array !!");
        return NULL;
    }

    float* predicts = malloc(tags_size*sizeof(float*));
    int i = 0;
    int temp = 0;
    while(i < tags_size){
        // printf("i: %d, size: %d\n", i, tags_size);
        predicts[i] = calc_s_spars(model->finalWeights, spars, spars_size, i, &temp);
        i++;
    }

    // printf("vgainei apo proba\n");
    return predicts;
}

int* logistic_predict(logM* model, float** vector, int vector_rows, int vector_cols){
    float* probs = logistic_predict_proba(model, vector, vector_rows, vector_cols);
    
    int* predicts = NULL;
    if(probs != NULL){
        predicts = malloc(vector_rows*sizeof(int));
        int i = 0;
        while(i < vector_rows){
            // printf("probs[%d]: %.4f  ||  ", i, probs[i]);
            if(probs[i] < model->finalWeights->threshold){
                predicts[i] = 1;
            }
            else
                predicts[i] = 0;
            i++;
        }
    }

    free(probs);
    return predicts;
}

// model, spars, spars_size, dimensions, tags_size
int* logistic_predict_spars(logM* model, float** spars, int spars_size, int dimensions, int tags_size){
    float* probs = logistic_predict_proba_spars(model, spars, spars_size, dimensions, tags_size);
    
    int* predicts = NULL;
    if(probs != NULL){
        predicts = malloc(tags_size*sizeof(int));
        int i = 0;
        while(i < tags_size){
            // printf("probs[%d]: %.4f  ||  ", i, probs[i]);
            if(probs[i] > model->finalWeights->threshold){
                predicts[i] = 1;
            }
            else
                predicts[i] = 0;
            i++;
        }
    }

    free(probs);
    return predicts;
}



float logistic_score(logM* model, int* labels1, int* labels2, int size){
    float score = 0.0;

    int i = 0;
    while(i < size){
        // printf("predict: %d, target: %d\n", labels1[i], labels2[i]);
        if(labels1[i] == labels2[i]){
            score++;
        }
        i++;
    }
    
    printf("\tcorrects: %.f, total: %d\n", score, size);
    return score / (float) size;
}

void logistic_extract(logM* model){
    if(chdir(PATH) == -1){
		if(mkdir(PATH, S_IRWXU|S_IRWXG|S_IROTH)){ 
		  error(EXIT_FAILURE, errno, "Failed to create directory");
   		}
   	}
   	else{
   		chdir("..");
   	}

    int target_size = strlen(EXTRACT_FILE)+strlen(PATH)+2;
    char* target = malloc(target_size);
    memset(target, 0, target_size);

    strcat(target, PATH);
	strcat(target, "/");
	strcat(target, EXTRACT_FILE);

    

    FILE* fpout = NULL;
    fpout = fopen(target, "w");
    if(fpout == NULL){
        printf("Can't create extraction file ~ model\n");
        return;
    }

    fprintf(fpout, "MODEL:\n");
    fprintf(fpout, "\tsize_totrain: %d\n", model->size_totrain);
    fprintf(fpout, "\tweights_count: %d\n", model->weights_count);
    fprintf(fpout, "\ttrained_times: %d\n", model->trained_times);

    fclose(fpout);

    weights_extract(model->finalWeights);

    free(target);

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

weights* weights_create(){
    weights* newWeight = malloc(sizeof(weights));

    newWeight->entries = 0;
    newWeight->limit = STOP_LIMIT;
    newWeight->rate = LEARING_RATE;
    newWeight->threshold = THRESHOLD;

    newWeight->b = 0.0000;
    newWeight->weightsT = NULL;

    return newWeight;
}


void weights_destroy(weights* myWeights){
    if(myWeights->entries > 0)
        free(myWeights->weightsT);

    free(myWeights);
}

void weights_set(weights* myWeights, int size){
    myWeights->entries = size;
    myWeights->weightsT = malloc(size*sizeof(float));

    int i = 0;
    while(i < size){
        myWeights->weightsT[i++] = 0.0000;
    }
}

float weights_update(weights* myWeights, float* grad, int size){
    
    float dif = 0.0000;
    int active = 1;
// printf("size: %d, grad_test[1]: %.4f\n", size, grad[1]);
    
    float new_b = myWeights->b - myWeights->rate*grad[0];
    dif += new_b - myWeights->b;
    myWeights->b = new_b;

    int i = 0;

    while(i < size){
        float new = myWeights->weightsT[i] - ( myWeights->rate * grad[i+1] );

        if(grad[i+1] != 0){
            // printf("grad[%d]: %.4f, old: %.4f, new: %.4f\n", i+1, grad[i+1], myWeights->weightsT[i], new);
            active++;
        }

        dif += new - myWeights->weightsT[i];
        myWeights->weightsT[i] = new;

        i++;
    }
// printf("dif inside: %.4f, active: %d\n", dif, active);
    dif = dif / (float) active;

    return dif;
}

void weights_print(weights* myWeights){
    printf("Weights:\n\t");
    printf("b: %.4f\n\t", myWeights->b);
    int i = 0;
    while(i < myWeights->entries){
        if(myWeights->weightsT[i] != 0.0)
            printf("w[%d]: %.4f  ||  ", i , myWeights->weightsT[i]);
        i++;
    }
    printf("\n");
}

void weights_extract(weights* myWeights){
    FILE* fpout = NULL;

    if(chdir(PATH) == -1){
		if(mkdir(PATH, S_IRWXU|S_IRWXG|S_IROTH)){ 
		  error(EXIT_FAILURE, errno, "Failed to create directory");
   		}
   	}
   	else{
   		chdir("..");
   	}

    int target_size = strlen(EXTRACT_FILE)+strlen(PATH)+2;
    char* target = malloc(target_size);
    memset(target, 0, target_size);

    strcat(target, PATH);
	strcat(target, "/");
	strcat(target, EXTRACT_FILE);

    fpout = fopen(target, "r+");
    if(fpout == NULL){
        printf("Cant open file - weights\n");
        return;
    }

    fseek(fpout, 0, SEEK_END);
    fprintf(fpout, "WEIGHTS:\n");
    fprintf(fpout, "\tentries: %d\n", myWeights->entries);
    fprintf(fpout, "\tlimit: %.4f\n", myWeights->limit);
    fprintf(fpout, "\trate: %.4f\n", myWeights->rate);
    fprintf(fpout, "\tthreshold: %.4f\n", myWeights->threshold);
    fprintf(fpout, "\tb: %.4f\n", myWeights->b);
    int i = 0;
    while( i < myWeights->entries){
        fprintf(fpout, "\tw[%d]: %.4f\n", i, myWeights->weightsT[i]);
        i++;
    }

    fclose(fpout);
    free(target);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

float calc_f(weights* myWeights, float* values){
    float sum = myWeights->b;

    int i = 0;
    while(i < myWeights->entries){
        sum += myWeights->weightsT[i] * values[i];
        i++;
    }

    // printf("\t f: %.4f\n", sum);
    return sum;
}

float calc_f_spars(weights* myWeights, float** spars, int spars_size, int target, int *temp){
    float sum = myWeights->b;
    int i = 0;
    
   while(spars[*temp][0] != (float) target){
        *temp = *temp + 1;
    }
    while(i < myWeights->entries){
        if(*temp < (float)spars_size){
            if(spars[*temp][0] == (float)target){
                if(spars[*temp][1] == (float) i){
                    sum += myWeights->weightsT[i] * spars[*temp][2];
                    *temp = *temp + 1;
                }
            }
        }
        i++;
    }

    return sum;
}

float calc_s(weights* myWeights, float* values){
    float f = 0.0;
    f =  calc_f(myWeights, values);

    double fixed = 0.0; 
    fixed = -1.0000*( (double) f);

    float sum = 0.0;
    sum =  1.0000 / ( 1.0000+ (float)exp(fixed));

   
    return sum;
}

float calc_s_spars(weights* myWeights, float** spars, int spars_size, int target, int* temp){
    float f = 0.0;
    f =  calc_f_spars(myWeights, spars, spars_size, target, temp);

    double fixed = 0.0;
    fixed = -1.0000*( (double) f);

    float sum = 0.0;
    sum =  1.0000 / ( 1.0000+ (float)exp(fixed));
//  printf("vgainei apo calc_s\n");
    return sum;
}

float calc_L_WB(weights* weights, float* values, int tag){

    double L = 0;
    double p_x =  (double)calc_s(weights, values);

    if(tag == 1){
        L = -1.0 * log(p_x);
    }
    else if(tag == 0){
        L = -1.0 * log(1.0 - p_x);
    }

    return (float) L;
}

float active_mean(float* vec, int size){
    float m = 0.0;
    int i = 0;
    // int active = 0;
    while(i < size){
        if(vec[i] > 0.0)
            m+= vec[i];
        else
            m += -1.0*vec[i];
            // active ++;
        // }
        i++;
    }
    
    // if(active == 0)
        // return 1;
    return m / (float) i;
}