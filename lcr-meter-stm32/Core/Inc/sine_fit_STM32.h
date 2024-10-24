

float calculateError(float data[], float timestamps[], int dataLength,
                      float amplitude, float phase, float offset, float frequency);


void fitSineWave(const float data[], const float timestamps[], int dataLength, float frequency,
				float* bestAmplitude, float* bestPhase, float* bestOffset);
