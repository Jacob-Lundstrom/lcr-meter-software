// This file will be used to fit a sine wave to collected data.
#include <math.h>
#include <main.h>

double __row_col_mult_sum_1__(int N, double A1[3][N], int row1, double A2[N][3], int col2){
	// Sums the multiple of row row1 in A1 with the corresponding values in column col2 of A2
	double sum = 0;
	for (int i = 0; i < N; i++) {
	    sum += A1[row1][i] * A2[i][col2];
	}
	return sum;
}

double __row_col_mult_sum_2__(int N, double A[3][N], int row1, double B[]) {

    double sum = 0;
    for (int i = 0; i < N; i++) {
        sum += A[row1][i] * B[i];
    }
    return sum;
}


/* This function takes the input Nx3 matrix, then multiplies it by its transpose
* O = (A Transpose) * A
* The result of this will be a 3x3 matrix. The data will be stored in a two-dimensional
* array that is passed to the function.
*/
void ATA_Nx3(const int N, double A[N][3], double O[3][3]) {
    // I'm not using input validation. Make sure that the input is correct.
    double AT[3][N];
    // correct i think
    for (int i = 0; i < N; i++) {
        AT[0][i] = A[i][0];
        AT[1][i] = A[i][1];
        AT[2][i] = A[i][2];
    }

    for (int i = 0; i < 3; i++) {
        O[0][i] = __row_col_mult_sum_1__(N, AT, 0, A, i);
        O[1][i] = __row_col_mult_sum_1__(N, AT, 1, A, i);
        O[2][i] = __row_col_mult_sum_1__(N, AT, 2, A, i);
    }
    return;
}

void ATB_Nx3_Nx1(int N, double A[N][3], double B[], double O[3][1]) {
    // Here, B is treated as a column vecter, though the input should be in row vector form.

    // I'm not using input validation. Make sure that the input is correct.
    double AT[3][N];
    for (int i = 0; i < N; i++) {
        AT[0][i] = A[i][0];
        AT[1][i] = A[i][1];
        AT[2][i] = A[i][2];
    }
    O[0][0] = __row_col_mult_sum_2__(N, AT, 0, B);
    O[1][0] = __row_col_mult_sum_2__(N, AT, 1, B);
    O[2][0] = __row_col_mult_sum_2__(N, AT, 2, B);

    return;
}

// Function is verified to work
void invert_3x3(double A[3][3], double O[3][3]) {
    // Will take the contents of matrix A and place the matrix inverse in O.

    // Find the cofactor matrix of A, place contents in O
    O[0][0] = A[1][1] * A[2][2] - A[1][2] * A[2][1]; // X
    O[0][1] = -(A[1][0] * A[2][2] - A[1][2] * A[2][0]); // X
    O[0][2] = A[1][0] * A[2][1] - A[1][1] * A[2][0]; // X

    O[1][0] = -(A[0][1] * A[2][2] - A[0][2] * A[2][1]); // X
    O[1][1] = A[0][0] * A[2][2] - A[0][2] * A[2][0]; // X
    O[1][2] = -(A[0][0] * A[2][1] - A[0][1] * A[2][0]); // X

    O[2][0] = A[0][1] * A[1][2] - A[0][2] * A[1][1]; // X
    O[2][1] = -(A[0][0] * A[1][2] - A[0][2] * A[1][0]); // X
    O[2][2] = A[0][0] * A[1][1] - A[0][1] * A[1][0]; // X

    // Find the Adjoint matrix ( Transpose the cofactor matrix )
    double OT[3][3];
    for (int i = 0; i < 3; i++) {
        OT[0][i] = O[i][0];
        OT[1][i] = O[i][1];
        OT[2][i] = O[i][2];
    }

    // Replace contents in O
    for (int i = 0; i < 3; i++) {
        O[i][0] = OT[i][0];
        O[i][1] = OT[i][1];
        O[i][2] = OT[i][2];
    }

    // Find determinant of A, divide contents of O and replace
    double d1 = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]);
    double d2 = -A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]);
    double d3 = A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
    double determinant = d1 + d2 + d3;
    // Technically I don't have to do the additional calculations of
    // the cofactors but I'm leaving it for clarity

    if (determinant == 0.0) {
        return;
    }

    for (int i = 0; i < 3; i++) {
        O[0][i] /= determinant;
        O[1][i] /= determinant;
        O[2][i] /= determinant;
    }
}


void Matrix_Multiply_3x3_3x1(double A[3][3], double B[3][1], double O[3][1]) {
    for (int i = 0; i < 3; i++) {
        O[i][0] = 0; // Initialize the output element to zero
        for (int j = 0; j < 3; j++) {
            O[i][0] += A[i][j] * B[j][0];
        }
    }
}

void least_squares_sine(int N, double injection_frequency, double sampling_rate, double offset_us, double data[], double* bestAmplitude, double* bestPhase, double* bestOffset) {

    double basis[N][3];
    for (int i = 0; i < N; i++) {
    	double t = (((double)i / sampling_rate)+ offset_us / 1e6);
        basis[i][0] = sin(2 * M_PI * injection_frequency * t);
        basis[i][1] = cos(2 * M_PI * injection_frequency * t);
        basis[i][2] = 1;
    }

    double ATA[3][3];
    ATA_Nx3(N, basis, ATA); // Same

    double ATAI[3][3];
    invert_3x3(ATA, ATAI); // Same

    double ATB[3][1];
    ATB_Nx3_Nx1(N, basis, data, ATB); // same

    double X[3][1];
    Matrix_Multiply_3x3_3x1(ATAI, ATB, X); // Different

    *bestAmplitude = sqrt(X[0][0] * X[0][0] + X[1][0] * X[1][0]);
    *bestPhase = atan2(X[1][0], X[0][0]);
    *bestOffset = X[2][0];
}
