#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* gcc -o run_stress_ng run_stress_ng.c -lm --static */

#define BUFFER_SIZE 1024

typedef struct {
    double ops_per_sec;
} Metrics;

void run_stress_ng(Metrics *metrics, int num_runs) {
    char buffer[BUFFER_SIZE];
    char command[] = "./stress-ng --times --verify --metrics --no-rand-seed --timeout 60 --pagemove 64";
    FILE *pipe;

    for (int i = 0; i < num_runs; i++) {
        printf("Running stress-ng pagemove test (%d/%d)...\n", i + 1, num_runs);

        // Open the pipe to run the command and capture the output
        pipe = popen(command, "r");
        if (!pipe) {
            perror("popen failed");
            exit(EXIT_FAILURE);
        }

        // Initialize the metrics for this run
        metrics[i].ops_per_sec = 0.0;

        // Read the command output line by line
        while (fgets(buffer, BUFFER_SIZE, pipe)) {
	    //stress-ng: metrc: [4205] pagemove           40418.14 page remaps per sec (harmonic mean of 64 instances)
	   if (strstr(buffer, "page remaps per sec") == NULL)
	       continue;

            if (sscanf(buffer, "%*[^p]pagemove%*[^0-9]%lf [^p]page remaps per sec", &metrics[i].ops_per_sec) == 1) {
		printf("%lf opt per sec", metrics[i].ops_per_sec);
                continue;
            }
        }

        pclose(pipe);
    }
}

void calculate_statistics(Metrics *metrics, int num_runs) {
    double ops_per_sec_sum = 0.0, ops_per_sec_mean = 0.0, ops_per_sec_std_dev = 0.0, ops_per_sec_std_err = 0.0;

    // Calculate the mean
    for (int i = 0; i < num_runs; i++) {
        ops_per_sec_sum += metrics[i].ops_per_sec;
    }
    ops_per_sec_mean = ops_per_sec_sum / num_runs;

    // Calculate the standard deviation
    for (int i = 0; i < num_runs; i++) {
        ops_per_sec_std_dev += pow(metrics[i].ops_per_sec - ops_per_sec_mean, 2);
    }
    ops_per_sec_std_dev = sqrt(ops_per_sec_std_dev / num_runs);

    // Calculate the standard error
    ops_per_sec_std_err = ops_per_sec_std_dev / sqrt(num_runs);

    // Print the results
    printf("\nStatistics:\n");

    printf("Ops/sec:\n");
    printf("  Mean     : %.2f\n", ops_per_sec_mean);
    printf("  Std Dev  : %.2f (%.2f%% of Mean)\n", ops_per_sec_std_dev, (ops_per_sec_std_dev / ops_per_sec_mean) * 100);
    printf("  Std Err  : %.2f (%.2f%% of Mean)\n", ops_per_sec_std_err, (ops_per_sec_std_err / ops_per_sec_mean) * 100);
}

int main(int argc, char *argv[]) {
    int num_runs;  // Number of runs
    Metrics *metrics;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_runs>\n", argv[0]);
	return EXIT_FAILURE;
    }

    num_runs = atoi(argv[1]);

    metrics=malloc(sizeof(Metrics) * num_runs);
    
    int num_run = atoi(argv[1]);

    // Run stress-ng and collect metrics
    run_stress_ng(metrics, num_runs);

    // Calculate and print statistics
    calculate_statistics(metrics, num_runs);

    return 0;
}
