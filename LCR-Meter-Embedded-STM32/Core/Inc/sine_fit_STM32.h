

float calculateError(const float data[], int dataLength,
                      float amplitude, float phase, float offset, float frequency);


void fitSineWave(const float data[], int dataLength, float frequency,
				float* bestAmplitude, float* bestPhase, float* bestOffset);
