#include <math.h>
#include <time.h>
#include <stdlib.h>


float sumSquared(const float arr[], int size) ;

float sumSineSquared( const float timestamps[], int size, float frequency) ;

float sumCosineSquared( const float timestamps[], int size, float frequency) ;

float sumMult(const float arr1[], const float arr2[], int size) ;

float sumSineMult(const float data[], const float timestamps[], int size, float frequency);

float sumCosineMult(const float data[], const float timestamps[], int size, float frequency);

float avg(const float arr1[], int size) ;

double avg2(const double arr1[], const double arr2[], int size);

void SINE_least_squares_regression(const float data[], const float timestamps[], int n_samples,
								   float frequency, float* bestAmplitude, float* bestPhase, float* bestOffset);
