#include <math.h>
#include <float.h>
#include <stdbool.h>

// Function to calculate the error between the input data and a sine wave
float calculateError(float data[], float timestamps[], int dataLength,
                      float amplitude, float phase, float offset, float frequency) {
    double error = 0.0;
    for (int i = 0; i < dataLength; i++) {
        float sineValue = amplitude * sin(2 * M_PI * frequency * timestamps[i] + phase) + offset;
        float difference = sineValue - data[i];
        error += difference * difference; // Squared error
    }
    return error;
}


float max(float data[], int dataLength) {
	float max = 0.0f;
	for (int i = 0; i < dataLength; i++) {
		if (data[i] > max) {
			max = data[i];
		}
	}
	return max;
}


float min(float data[], int dataLength) {
	float min = data[0];
	for (int i = 0; i < dataLength; i++) {
		if (data[i] < min) {
			min = data[i];
		}
	}
	return min;
}

float find_zero_crossing(float data[], int dataLength, float offset) {
	float initial_val = data[0];
	if (initial_val > offset) {
		for (int i = 0; i < dataLength; i++) {
			if (data[i] < offset) {
				// Falling side, phase difference, this point is pi offset from the phase.
				// Just pass this value along to the next part.
				initial_val = data[i];
			} else if (data[i] > offset) {
				// This is the point that should be used as a reference for the phase.
				return (data[i] + data[i-1]) / 2;
			}
		}
	} else {
		for (int i = 0; i < dataLength; i++) {
			if (data[i] > offset) {
				// Rising edge, no need to continue.
				return (data[i] + data[i-1]) / 2;
			}
		}
	}
}

float find_offset(float data[], int dataLength) {
	float total = 0;
	for (int i = 0; i < dataLength; i++) {
		total += data[i];
	}
	total = total / dataLength;
	return total;
}

// Function to fit sine wave to the data by sweeping amplitude, phase, and offset
void fitSineWave(const float data[], const float timestamps[], int dataLength, float frequency,
				float* bestAmplitude, float* bestPhase, float* bestOffset) {

    float amplitudeMin = 0.01, amplitudeMax = 1.0, amplitudeStep = 0.01;
    float phaseMin = - M_PI, phaseMax = M_PI, phaseStepLarge = 1;
    float offsetMin = 0.5, offsetMax = 2.0, offsetStep = 0.01;

    int cycleSteps = 3;
    // SImplest (worst) method for sine fitting.
    // Bad guesses for amplitude and offset, then recursively find the phase angle.
    float A = (max(data, dataLength) - min(data, dataLength)) / 2;
//    float O = (max(data, dataLength) + min(data, dataLength)) / 2;
    float O = find_offset(data, dataLength);


    float minError = DBL_MAX; // Initialize with the largest possible number

    // Start by finding a quick estimate
    for (float phase = phaseMin; phase <= phaseMax; phase += phaseStepLarge / cycleSteps) {
		// Calculate the error for the current combination of parameters
		float error = calculateError(data, timestamps, dataLength, A, phase, O, frequency);

		// If this combination gives a smaller error, update the best parameters
		if (error < minError) {
			minError = error;
			*bestPhase = phase;
			*bestAmplitude = A;
			*bestOffset = O;
		}
	}

    // from here on out, recursively sweep to find a more and more ideal phase estimation.
    for(int i = 2; i <= cycleSteps; i++ ) {
		// At this point it should be an alright value of the phase. Get closer to the real one.
		for (float phase = *bestPhase - phaseStepLarge / (10 * (i - 1));
				phase <= *bestPhase + phaseStepLarge / (10 * (i - 1));
				phase += phaseStepLarge / (10 * i)) {
			// Calculate the error for the current combination of parameters
			float error = calculateError(data, timestamps, dataLength, A, phase, O, frequency);

			// If this combination gives a smaller error, update the best parameters
			if (error < minError) {
				minError = error;
				*bestPhase = phase;
				*bestAmplitude = A;
				*bestOffset = O;
			}
		}
    }



//    float minError = DBL_MAX; // Initialize with the largest possible number
//	for (float phase = phaseMin; phase <= phaseMax; phase += phaseStep) {
//		// Calculate the error for the current combination of parameters
//		float error = calculateError(data, timestamps, dataLength, 1, phase, 0, frequency);
//
//		// If this combination gives a smaller error, update the best parameters
//		if (error < minError) {
//			minError = error;
//			*bestPhase = phase;
//		}
//	}
//
//	minError = DBL_MAX;
//
//	for (double amplitude = amplitudeMin; amplitude <= amplitudeMax; amplitude += amplitudeStep) {
//			// Calculate the error for the current combination of parameters
//		double error = calculateError(data, timestamps, dataLength, amplitude, 0, 0, frequency);
//
//		// If this combination gives a smaller error, update the best parameters
//		if (error < minError) {
//			minError = error;
//			*bestAmplitude = amplitude;
//		}
//	}
//
//	minError = DBL_MAX;
//
//    for (float offset = offsetMin; offset <= offsetMax; offset += offsetStep) {
//		// Calculate the error for the current combination of parameters
//		float error = calculateError(data, timestamps, dataLength, *bestAmplitude, *bestPhase, offset, frequency);
//
//		// If this combination gives a smaller error, update the best parameters
//		if (error < minError) {
//			minError = error;
//			*bestOffset = offset;
//		}
//	}


//	 Nested loops to sweep amplitude, phase, and offset
//    float minError = DBL_MAX;
//	for (double amplitude = amplitudeMin; amplitude <= amplitudeMax; amplitude += amplitudeStep) {
//		for (double phase = phaseMin; phase <= phaseMax; phase += phaseStep) {
//			for (float offset = offsetMin; offset <= offsetMax; offset += offsetStep) {
//					// Calculate the error for the current combination of parameters
//				float error = calculateError(data, timestamps, dataLength, *bestAmplitude, *bestPhase, offset, frequency);
//
//				// If this combination gives a smaller error, update the best parameters
//				if (error < minError) {
//					minError = error;
//					*bestAmplitude = amplitude;
//					*bestPhase = phase;
//					*bestOffset = offset;
//				}
//			}
//		}
//    }
}
