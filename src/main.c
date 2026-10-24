#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_SIZE 64  /* max number of coefficients, i.e. max degree is MAX_SIZE - 1 */
#define TOLERANCE 1e-3
#define MAX_ITERATION 1023

static void printVector(const double *vec, int size) {
    for (int i = 0; i < size; i++) {
        printf("%g\n", vec[i]);
    }
}

static int compareDouble(const void *a, const void *b) {
    double x = *(const double *)a;
    double y = *(const double *)b;
    return (x > y) - (x < y);
}

static int graeffeRootsFinder(const double *coeff, int size, int max_iterations) {
    int n = size - 1;  // degree of polynomial
    max_iterations = max_iterations > MAX_ITERATION ? MAX_ITERATION : max_iterations;

    double bufA[MAX_SIZE];
    double bufB[MAX_SIZE];
    double bufC[MAX_SIZE];
    double bufD[MAX_SIZE];

    double *curent_coeff = bufA;
    double *next_coeff = bufB;

    double *estimated_roots = bufC;
    double *next_estimated_roots = bufD;

    int bad = 0;
    
    int actual_iterations = 0;

    // initialization
    for (int i = 0; i < size; i++) {
        curent_coeff[i] = coeff[i];
        estimated_roots[i] = curent_coeff[i];
    }

    // perform root squaring
    for (int iter = 0; iter < max_iterations; iter++) {
        int ok = 1;

        for (int j = 0; j <= n && ok; j++) {
            double sum = 0.0;
            int outer_sign = (j % 2 == 0) ? 1 : -1;

            for (int k = 1; k <= j; k++) {
                if (j - k >= 0 && j + k <= n) {
                    double inner_sign = (k % 2 == 1) ? -1 * outer_sign : outer_sign;
                    sum += inner_sign * 2.0 * curent_coeff[j - k] * curent_coeff[j + k];
                }
            }

            next_coeff[j] = (outer_sign * curent_coeff[j] * curent_coeff[j]) + sum;

            if (!isfinite(sum) || !isfinite(next_coeff[j])) ok = 0;
        }

        // if bad; coefficients might be exploding
        if (!ok) break;

        // calculate estimated roots and compare
        double m = pow(2.0, iter + 1);
        for (int i = 0; i < n && ok; i++) {
            if (next_coeff[i] == 0 || !isfinite(next_coeff[i + 1] / next_coeff[i])) {
                ok = 0;
                continue;
            }

            double ratio = fabs(next_coeff[i + 1] / next_coeff[i]);
            next_estimated_roots[i] = pow(ratio, 1.0 / m);
        }

        if (!ok) break; // if bad; coefficients underflowing

        double change = 0.0;
        for (int i = 0; i <= n; i++) {
            double scale = fmax(estimated_roots[i], next_estimated_roots[i]);
            if (scale > 0.0) {
                change = fmax(change, fabs(estimated_roots[i] - next_estimated_roots[i]) / scale);
            }
        }
        
        // swap buffers instead of copying
        double *tmp = curent_coeff;
        curent_coeff = next_coeff;
        next_coeff = tmp;
        
        double *tmp_roots = next_estimated_roots;
        next_estimated_roots = estimated_roots;
        estimated_roots = tmp_roots;

        if (change < TOLERANCE) break;  // converged
    }

    // sort the roots
    qsort(estimated_roots, n, sizeof(double), compareDouble);

    printf("Smallest Root: %g\n", estimated_roots[0]);
    printf("Largest Root: %g\n", estimated_roots[n - 1]);
    printf("Middle Root: %g\n", estimated_roots[n / 2]);

    return 0;
}

int main(void) {
    int degree;
    int iterations = 0;
    double coeff[MAX_SIZE];

    printf("Enter the polynomial degree:\n");
    if (scanf("%d", &degree) != 1 || degree < 1 || degree > MAX_SIZE - 1) {
        fprintf(stderr, "error: degree must be an integer between 1 and %d\n", MAX_SIZE - 1);
        return 1;
    }

    printf("Enter the respective coefficients: \n");
    for (int i = 0; i <= degree; i++) {
        if (scanf("%lf", &coeff[i]) != 1) {
            fprintf(stderr, "error: invalid coefficient\n");
            return 1;
        }
    }

    if (coeff[0] != 1) {
        printf("error: the coefficient of highest degree should be 1\n");
        return 1;
    }

    printf("Enter the number of iterations: \n");
    if (scanf("%d", &iterations) != 1 || iterations < 0) {
        fprintf(stderr, "error: iterations must be an integer >= 0\n");
        return 1;
    }

    // record the start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    graeffeRootsFinder(coeff, degree + 1, iterations);

    // record the end time
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration_ms = (long long)(end.tv_sec - start.tv_sec) * 1000LL
                          + (end.tv_nsec - start.tv_nsec) / 1000000LL;

    printf("Execution time: %lld ms\n", duration_ms);

    return 0;
}