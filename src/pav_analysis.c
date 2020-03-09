#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    
    int i = 0;
    float P = 0;
    for(i = 0; i < N; i++){

        P = P + x[i]*x[i];
    }
    return 10*log10(P/N);
}

float compute_am(const float *x, unsigned int N) {

    int i = 0;
    float A = 0;
    for(i = 0; i < N; i++){

        A = A + fabs(x[i]);
    }

    return A/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {

    int i = 0;
    float ZCR = 0;
    for(i = 1; i < N; i++){
        
        if(x[i - 1] * x[i] < 0){

            ZCR++;
        }
    }

    return ZCR * (fm/(2*(N-1)));
}

float hamming_window(int n, int N){

    return (0.54 - 0.46 * cos(2 * n * N * M_PI));
}

float compute_power_hamming(const float *x, unsigned int N) {
    
    int i = 0;
    float numerador = 0;
    float denominador = 0;
    float P = 0;
    for(i = 0; i < N; i++){

        numerador = numerador + pow(x[i] * hamming_window(i, N), 2);
        denominador = denominador + pow(hamming_window(i, N), 2);

    }


    return 10*log10(numerador/denominador);
}