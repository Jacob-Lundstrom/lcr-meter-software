#include <math.h>
#include <time.h>
#include <stdlib.h>


float sumSquared(const float arr[], int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] * arr[i];
    }
    return sum;
}

float sumSineSquared( const float timestamps[], int size, float frequency) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += sin(2 * M_PI * frequency * timestamps[i]) * sin(2 * M_PI * frequency * timestamps[i]);
    }
    return sum;
}

float sumCosineSquared( const float timestamps[], int size, float frequency) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += cos(2 * M_PI * frequency * timestamps[i]) * cos(2 * M_PI * frequency * timestamps[i]);
    }
    return sum;
}

float sumMult(const float arr1[], const float arr2[], int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr1[i] * arr2[i];
    }
    return sum;
}

float sumSineMult(const float data[], const float timestamps[], int size, float frequency) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * sin(2 * M_PI * frequency * timestamps[i]);
    }
    return sum;
}

float sumCosineMult(const float data[], const float timestamps[], int size, float frequency) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i] * cos(2 * M_PI * frequency * timestamps[i]);;
    }
    return sum;
}

float avg(const float arr1[], int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr1[i];
    }
    return sum / size;
}

double avg2(const double arr1[], const double arr2[], int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr1[i] * arr2[i];
    }
    return sum / size;
}

void SINE_least_squares_regression(const float data[], const float timestamps[], int n_samples, float frequency, float* bestAmplitude, float* bestPhase, float* bestOffset) {

    float result[2] = { sumSineMult(data, timestamps, n_samples , frequency) / sumSineSquared(timestamps, n_samples , frequency),
                        sumCosineMult(data, timestamps, n_samples , frequency) / sumCosineSquared(timestamps, n_samples , frequency) };

//    float result[2] = { 2 * sumSineMult(data, timestamps, n_samples , frequency) / n_samples,
//                        2 * sumCosineMult(data, timestamps, n_samples , frequency) / n_samples};

    float A = sqrt(result[0] * result[0] + result[1] * result[1]);
    float P = atan(result[1] / result[0]);
    float O = avg(data, n_samples);

    *bestAmplitude = A;
    *bestPhase = P;
    *bestOffset = O;
}
