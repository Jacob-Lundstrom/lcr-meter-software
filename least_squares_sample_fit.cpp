#include <iostream>
#include <cmath>
#include <string>

#include <time.h>
#include <stdlib.h>


float sumSquared(const float arr[], int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] * arr[i];
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
    
    float basisT[2][n_samples]; // Transposed basis matrix. Transposed because it makes the functions more readable
    for(int i = 0; i < n_samples; i++) {

        basisT[0][i] = sin(2 * M_PI * frequency * timestamps[i]);
        basisT[1][i] = cos(2 * M_PI * frequency * timestamps[i]);

    }
    
    float result[2] = { sumMult(basisT[0], data, n_samples) / sumSquared(basisT[0], n_samples) , 
                         sumMult(basisT[1], data, n_samples) / sumSquared(basisT[1], n_samples) };

    float A = sqrt(result[0] * result[0] + result[1] * result[1]);
    float P = atan(result[1] / result[0]);
    float O = avg(data, n_samples);

    *bestAmplitude = A;
    *bestPhase = P;
    *bestOffset = O;
}

int main() {
    
    srand(time(NULL));   // Initialization, should only be called once.

    const int n_samples = 1001;
    float frequency = 10; // Hz
    double max_sample_deviation = 0.001; // seconds

    float data[n_samples];
    float time[n_samples];

    for(int i = 0; i < n_samples; i++) {

        double r =  2 * max_sample_deviation * (double)rand() / (RAND_MAX); // 0 to 10 us
        
        time[i] = ((double)i / ((double)n_samples * frequency)) + (r - max_sample_deviation); // Sample spacing randomness

        data[i] = sin(2 * M_PI * frequency * time[i] + M_PI / 6);

    }

    float amplitude, phase, offset;
    SINE_least_squares_regression(data, time, n_samples, frequency, &amplitude, &phase, &offset);

    std::cout << "Amplitude: " << std::endl;
    std::cout << amplitude << std::endl;
    std::cout << "Phase: " << std::endl;
    std::cout << phase << std::endl;
    std::cout << M_PI / 6  << std::endl;
    std::cout << "Offset" << std::endl;
    std::cout << offset << std::endl;

}