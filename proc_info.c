#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#define MAX_LINE_LENGTH 256

// This Function reads and prints processor type
void print_processor_type() {
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file == NULL) {
        perror("Error opening /proc/cpuinfo");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "model name", 10) == 0) {
            printf("Processor type: %s", line + 13);
            break;
        }
    }

    fclose(file);
}

// This Function prints the kernel version
void print_kernel_version() {
    FILE *file = fopen("/proc/version", "r");
    if (file == NULL) {
        perror("Error opening /proc/version");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    if (fgets(line, sizeof(line), file)) {
        printf("Kernel version: %s", line);
    }

    fclose(file);
}

// This Function prints memory information
void print_memory_info() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("Error opening /proc/meminfo");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "MemTotal", 8) == 0) {
            printf("Memory configured: %s", line + 10);
        }
    }

    fclose(file);
}

// This Function prints system uptime
void print_system_uptime() {
    FILE *file = fopen("/proc/uptime", "r");
    if (file == NULL) {
        perror("Error opening /proc/uptime");
        exit(EXIT_FAILURE);
    }

    double uptime;
    if (fscanf(file, "%lf", &uptime) == 1) {
        int days = (int)(uptime / (3600 * 24));
        int hours = (int)((uptime - days * 3600 * 24) / 3600);
        int minutes = (int)((uptime - days * 3600 * 24 - hours * 3600) / 60);
        int seconds = (int)(uptime - days * 3600 * 24 - hours * 3600 - minutes * 60);

        printf("System uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
    }

    fclose(file);
}

// This Function calculates CPU usage
double calculate_cpu_usage(long long int prev_idle, long long int prev_total) {
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    long long int user, nice, system, idle, total;
    if (fscanf(file, "cpu %lld %lld %lld %lld", &user, &nice, &system, &idle) != 4) {
        fclose(file);
        return -1.0;
    }

    total = user + nice + system + idle;
    fclose(file);

    double cpu_usage = 100.0 * (1.0 - ((double)(idle - prev_idle) / (double)(total - prev_total)));
    return cpu_usage;
}

// This Function prints dynamic values of CPU Usage
void print_dynamic_values(int read_rate, int printout_rate) {
    // Variables that store previous values of CPU usage
    long long int prev_idle = 0, prev_total = 0;

    while (1) {
        // Calculates CPU usage
        double cpu_usage = calculate_cpu_usage(prev_idle, prev_total);

        // Updates previous values for next iteration
        FILE *stat_file = fopen("/proc/stat", "r");
        if (stat_file == NULL) {
            perror("Error opening /proc/stat");
            exit(EXIT_FAILURE);
        }

        long long int user, nice, system, idle, total;
        if (fscanf(stat_file, "cpu %lld %lld %lld %lld", &user, &nice, &system, &idle) != 4) {
            fclose(stat_file);
            exit(EXIT_FAILURE);
        }

        total = user + nice + system + idle;
        fclose(stat_file);

        prev_idle = idle;
        prev_total = total;

        // Prints dynamic values
        printf("CPU Usage: %.2f%%\n", cpu_usage);

        // Sleeps for specified read rate
        sleep(read_rate);
    }
}


int main(int argc, char *argv[]) {
    // Checks for command line arguments
    if (argc == 1) {
        // Default version: It prints static information once
        print_processor_type();
        print_kernel_version();
        print_memory_info();
        print_system_uptime();
    } else if (argc == 3) {
        // Dynamic version: It prints dynamic values continuously
        int read_rate = atoi(argv[1]);
        int printout_rate = atoi(argv[2]);
        print_dynamic_values(read_rate, printout_rate);
    } else {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  For default version: %s\n", argv[0]);
        fprintf(stderr, "  For dynamic version: %s <read_rate> <printout_rate>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return 0;
}
