/* Logistic Regrassion  .c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/logistic.h" 

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

void logistic_fit(logM* model, int vector_rows, int vector_cols, float** vector , int* labels){
        // set model
    model->size_totrain = vector_rows;

        // set weights
    weights_set(model->finalWeights, vector_cols);
    model->weights_count = vector_cols;

        // train
    logistic_regression(model, vector, vector_rows, vector_cols, labels);

}

void logistic_regression(logM* model, float** vector, int vector_rows, int vector_cols, int* tags){
    
    printf("vector_rows: %d, vector_cols: %d\n", vector_rows, vector_cols);

    float limit = 1.000;

    while(limit > model->finalWeights->limit){

            // 1. Build predicts table
        float* predicts = malloc(vector_rows*sizeof(float));
        int i = 0;
        while(i < vector_rows){
            predicts[i] = logistic_predict(model, vector[i], vector_cols);
            i++;
        }

            // 2. Calc weights
                // 2.1 Build Missed Table
        float b_grad = 0.0;
        float* missed_by = malloc(vector_rows*sizeof(float));
        i = 0;
        while(i < vector_rows){
            missed_by[i] = predicts[i] - (float) tags[i];
            b_grad += missed_by[i];
            // printf("my_pred: %.4f, target: %d\n", predicts[i], tags[i]);
            i++;
        }

        b_grad /= (float) i;

                // 2.2 Calc Grad - PER WEIGHT !!!
        float* grad = malloc((vector_cols+1)*sizeof(float));
        grad[0] = b_grad;

        // printf("b_grtad: %.4f\n", b_grad);

        int y = 0;
        while(y < vector_cols){
            int x = 0;
            float magic_num = 0.0;
            while(x < vector_rows){
                if(vector[x][y]!=0){
                    // printf("vector[%d][%d]: %.4f, missed_by[%d]: %.4f\n", x,y, vector[x][y] ,x, missed_by[x]);

                    // printf("VRHKA ENA\n");
                }
                magic_num += vector[x][y]*missed_by[x];
                    // printf("vector[%d][%d]: %.4f, missed_by[%d]: %.4f", x,y, vector[x][y] ,x, missed_by[x]);
                x++;
            }
            grad[1+y] = magic_num;
            y++;
        }

                // 2.3 Update Weights
        limit = weights_update(model->finalWeights, grad, vector_cols);
        limit = mean(missed_by, vector_rows);
        // printf("limit: %.4f\n", limit);

        model->trained_times++;

        // if(model->trained_times > 5)
            // limit = 0.0;
        

        // FREE MEM
        free(predicts);
        free(missed_by);
        free(grad);
    }
    printf("limit: %.4f\n", limit);
    // int i = 0;
    // while(i < vector_rows ){
        // if()
        // printf("my_pred: %.4f, target: %d\n", logistic_predict(model, vector[i], vector_cols), tags[i]);
        // i++;
    // }
    

}


float logistic_predict(logM* model, float* vector, int size){
    float pred = -1.000;

    if(model->finalWeights == NULL){
        printf("Error - Untrained model !!\n");
        return -1.000;
    }
    
    if(size == model->weights_count){
        pred = calc_s(model->finalWeights, vector);
    }
    else{   // case we give more info than weights inside
        float* newVec = malloc(model->weights_count*sizeof(float));
        int i = 0;
        while(i < model->weights_count){
            newVec[i] = vector[i];
            i++;
        }

        pred =  calc_s(model->finalWeights, newVec);
        free(newVec);
    }

    return pred;

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

weights* weights_create(){
    weights* newWeight = malloc(sizeof(weights));

    newWeight->entries = 0;
    newWeight->limit = STOP_LIMIT;
    newWeight->rate = LEARING_RATE;

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

float calc_s(weights* myWeights, float* values){
    float f = 0.0;
    f =  calc_f(myWeights, values);

    double fixed = 0.0; 
    fixed = -1.0000*( (double) f);

    float sum = 0.0;
    sum =  1.0000 / ( 1.0000+ (float)exp(fixed));

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

float mean(float* vec, int size){
    float m = 0.0;
    int i = 0;
    while(i < size){
        m += vec[i++];
    }
    return m / (float)size;
}