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
    newModel->fit1 = 0;
    newModel->fit0 = 0;
    newModel->dataList = NULL;

    newModel->finalWeights = weights_create();

    return newModel;
}

void logistic_destroy(logM* model){
    
    weights_destroy(model->finalWeights);
    if(model->dataList != NULL)
        dataI_destroy(model->dataList);
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

int logistic_fit_dataList(logM* model, dataI* info){
        // set model
    model->size_totrain = info->all_pairs;
        
        // set weights
    weights_set(model->finalWeights, info->dimensions);
    model->weights_count = info->dimensions;
    // printf("info->dim: %d, weights_count: %d\n", info->dimensions, model->finalWeights->entries);
        // train
    return logistic_regression_dataList(model, info);
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

    // printf("vector_rows: %d, vector_cols: %d\n", vector_rows, vector_cols);

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
        // printf("\t\t~~~~~~~~~~~~~~~~~~~~~~~~\n");
        // printf("Time: %d\n", model->trained_times);
        // printf("limit: %.4f\n", limit);
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

    float limit = 1.000;

    logistic_overfit(model, tags, tags_size);

    model->trained_times = 1;
    while(limit > model->finalWeights->limit){

        if(received_signal == 1)
            return -1;

            // 1. Build predicts table
        float* predicts = logistic_predict_proba_spars(model, spars, spars_size, dimensions, tags_size);
        printf("PREDICTS ..\n");

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
            if(missed_by[i]> 0)
                b_grad += missed_by[i];
            else
                b_grad += -1.0*missed_by[i];
            i++;
        }
        printf("MISSED ..\n");
        b_grad /= (float) i;

                // 2.2 Calc Grad - PER WEIGHT !!!
                int mytime = 0;
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
                mytime++;
                if(spars[x][1] == (float) y){
                    magic_num += spars[x][2] * missed_by[(int)spars[x][0]];
                    // break;
                }
                x++;
            }

            //
            //
            // I CAN UPDATE WEIGHTS HERE SAVING TIME !!!!
            //
            //
            grad[1+y] = magic_num;
            y++;
        }
        printf("GRADS .. time: %d\n", mytime);
            // 3 Update Weights
        weights_update(model->finalWeights, grad, dimensions);
        limit = active_mean(missed_by, tags_size);
        model->trained_times++;
        printf("WEIGHTS ..\n");
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

    // TESTS
    // weights_print(model->finalWeights);
   
//    weights_print(model->finalWeights);

    int* final_predicts = logistic_predict_spars(model, spars, spars_size, dimensions, tags_size);
    printf("\tScore after train: %.4f\n", logistic_score(model, final_predicts, tags, tags_size));
    free(final_predicts);

    return 0;
}

int logistic_regression_dataList(logM* model, dataI* info){

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

    logistic_overfit_dataList(model, info);

    model->trained_times = 1;
    while(limit > model->finalWeights->limit){

        if(received_signal == 1)
            return -1;

            // 1. Make Predicts
        logistic_predict_proba_dataList(model, info);
        // printf("PREDICTS ..\n");
            // 2. Calc weights
                // 2.1 Build Missed Table
        float b_grad = 0.0;
        float* missed_by = malloc(info->all_pairs*sizeof(float));
        int i = 0;
        while(i < info->all_pairs){
            if(received_signal == 1){
                // FREE MEM
                free(missed_by);
                return -1;
            }

            dataN* to_train = dataI_pop(info);

            missed_by[i] = to_train->proba - to_train->label;
            if(missed_by[i]> 0)
                b_grad += missed_by[i];
            else
                b_grad += -1.0*missed_by[i];
            i++;
        }
        dataI_rewind_pop(info);
        // printf("MISSED ..\n");
        b_grad /= (float) i;

        int mytime = 0;
                // 2.2 Calc Grad - PER WEIGHT !!!
        float* grad = malloc((info->dimensions+1)*sizeof(float));
        grad[0] = b_grad;

        int y = 0;
        while(y < info->dimensions){
            if(received_signal == 1){
                // FREE MEM
                free(missed_by);
                free(grad);
                return -1;
            }

            float magic_num = 0.0;

            int count_trained = 0;
            while(count_trained < info->all_pairs){
                // SIGS
                if(received_signal == 1){
                    // FREE MEM
                    free(missed_by);
                    free(grad);
                    return -1;
                }
                
                dataN* to_train = dataI_pop(info);

                int x = 0;
                while(x < to_train->spars_size){
                    mytime++;
                    if(to_train->spars[x][1] == (float) y){
                        magic_num += to_train->spars[x][2] *missed_by[count_trained];
                        break;
                    }
                    x++;
                }

                grad[1+y] = magic_num;
                count_trained++;
            }
            y++;
            dataI_rewind_pop(info);
        }
        // printf("GRDAS .. time: %d\n", mytime);
            // 3 Update Weights
        weights_update(model->finalWeights, grad, info->dimensions);
        limit = active_mean(missed_by, info->all_pairs);
        // printf("WEIGHTS ..\n");
        model->trained_times++;

        // ΜAX ITERATIONS
        if(model->trained_times > 1)
            limit = 0.0;
        

        // FREE MEM
        free(missed_by);
        free(grad);
    }

    if(received_signal == 1)
        return -1;

    // weights_print(model->finalWeights);

    logistic_predict_proba_dataList(model, info);
    printf("\tScore after train: %.4f\n", logistic_score_dataList(model, info));

    return 0;
}

float* logistic_predict_proba(logM* model, float** vector, int vector_rows, int vector_cols){
    if(model->finalWeights == NULL){
        printf("Error - Untrained model !!\n");
        return NULL;
    }

    if(vector_cols > model->weights_count){
        printf("Error ~ Invalid Size Array !!\n");
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

void logistic_predict_proba_dataList(logM* model, dataI* info){
    if(model->finalWeights == NULL){
        printf("Error - Untrained model !!\n");
        return;
    }

    if(info->dimensions > model->weights_count){
        printf("Error ~ Invalid Size Array !!\n");
        return;
    }

    info->corrects = 0;

    dataN* to_calc = dataI_pop(info);
    while(to_calc != NULL){
        calc_s_dataList(model->finalWeights, to_calc);
        
        if(to_calc->proba >= model->finalWeights->threshold)
            to_calc->predict = 1;
        else
            to_calc->predict = 0;
        if(to_calc->label == to_calc->predict)
            info->corrects++;
        to_calc = dataI_pop(info);
    }
    dataI_rewind_pop(info);

    if(info->corrects != 0){
        logistic_score_dataList(model, info);
    }
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

float logistic_score_dataList(logM* model, dataI* info){
   info->score = (float) info->corrects / (float) info->all_pairs;
   return info->score;
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

void logistic_overfit(logM* model, int* tags, int tags_size){
    int i = 0;
    while(i < tags_size){
        if(tags[i] == 1)
            model->fit1++;
        else
            model->fit0++;
        i++;
    }

    float rate = 0.0;
    if(model->fit1 > model->fit0)
        rate = (float)model->fit1 / (float)model->fit0;
    else
        rate = -1.0* (float)model->fit0 / (float)model->fit1;
    
    
    model->finalWeights->threshold += (model->finalWeights->threshold/rate) * model->finalWeights->threshold;

    printf("\tOverfit: %.4f\n", rate);
    printf("\tnew thrshold: %.4f\n", model->finalWeights->threshold);
}

void logistic_overfit_dataList(logM* model, dataI* info){
    dataN* node = info->head;
    while(node != NULL){
        if(node->label == 1)
            model->fit1++;
        else
            model->fit0++;
        node = node->next;
    }

    float rate = 0.0;
    if(model->fit1 > model->fit0)
        rate = (float)model->fit1 / (float)model->fit0;
    else
        rate = -1.0* (float)model->fit0 / (float)model->fit1;
    
    
    model->finalWeights->threshold += (model->finalWeights->threshold/rate) * model->finalWeights->threshold;

    printf("\tOverfit: %.4f\n", rate);
    printf("\tnew thrshold: %.4f\n", model->finalWeights->threshold);

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

//    while(spars[*temp][0] != (float) target){
//         *temp = *temp + 1;
//     }

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

float calc_f_dataList(weights* myWeights, dataN* node){
    int sum = myWeights->b;
    int i = 0;
    int count_d = 0;
    while(i < myWeights->entries){
        if(count_d < node->spars_size){
            if(node->spars[count_d][1] == (float) i){
                sum += myWeights->weightsT[i] * node->spars[count_d][2];
                count_d++;
            }
        }
        i++;
    }
    // printf(">>>f[]: %.4f\n", sum);
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

void calc_s_dataList(weights* myWeights, dataN* node){
    float f = 0.0;
    f =  calc_f_dataList(myWeights, node);

    double fixed = 0.0;
    fixed = -1.0000*( (double) f);

    node->proba =  1.0000 / ( 1.0000+ (float)exp(fixed));
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


/////////////// ANOTHER LIST /////////////////

dataI* dataI_create(int dimensions){
    dataI* newI = malloc(sizeof(dataI));

    newI->all_pairs = 0;
    newI->corrects = 0;
    newI->score = 0.0;
    newI->dimensions = dimensions;
    newI->head = NULL;
    newI->poped = NULL;
    return newI;
}

void dataI_destroy(dataI* info){
    while(info->head != NULL){
        dataN_destroy(info, info->head);
    }
    free(info);
}

dataN* dataI_pop(dataI* info){
    if(info->poped == NULL)
        info->poped = info->head;
    else if(info->poped->next == NULL)
        return NULL;
    else
        info->poped = info->poped->next;

    return info->poped;
}


void dataI_push(dataI* info, mySpec* spec1, mySpec* spec2, float** spars, int spars_size, int tag){
    dataN* to_add = dataN_create();
    to_add->label = tag;
    to_add->spec1 = spec1;
    to_add->spec2 = spec2;
    to_add->spars = spars;
    to_add->spars_size = spars_size;


    if(info->head != NULL)
        info->head->prev = to_add;

    to_add->next = info->head;
    info->head = to_add;
    
    info->all_pairs++;
}

void dataI_remove_weak(dataI* info , logM* model){
    float limit1 = 1.0 - ( (1.0 - model->finalWeights->threshold) / 3.0 );
    float limit0 = 0.0 + ( (0.0 + model->finalWeights->threshold) / 3.0 );

    dataN* temp = info->head;
    while(temp != NULL){
        if(temp->proba >= model->finalWeights->threshold){
            if(temp->proba < limit1){
                dataN* keep  = temp->next;
                dataN_destroy(info, temp);
                temp = keep;
                continue;
            }       
        }
        else{
            if(temp->proba > limit0){
                dataN* keep = temp->next;
                dataN_destroy(info, temp);
                temp = keep;
                continue;
            }
        }
        temp = temp->next;
    }

}

void dataI_rewind_pop(dataI* info){
    info->poped = NULL;
}

dataN* dataN_create(){
    dataN* newN = malloc(sizeof(dataN));
    
    newN->label = -1;
    newN->predict = -1;
    newN->proba = -1.0;

    newN->next = NULL;
    newN->prev = NULL;

    newN->spars_size = 0;
    newN->spars = NULL;

    newN->spec1 = NULL;
    newN->spec2 = NULL;

    return newN;
}

void dataN_destroy(dataI* info, dataN* node){
    if(node == NULL)
        return;
    if(node == info->head){
        info->head = node->next;
        if(node->next != NULL){
            node->next->prev = NULL;
            info->head = node->next;
        }
    }
    else{
        if(node->next != NULL){
            node->next->prev = node->prev;
            node->prev->next = node->next;
        }
        else{
            node->prev->next = NULL;
        }
    }

    if(node->spars_size > 0){
        int i = 0;
        while(i < node->spars_size){
            free(node->spars[i++]);
        }
        free(node->spars);
    }
    free(node);

    info->all_pairs--;
}