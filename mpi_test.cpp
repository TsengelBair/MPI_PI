#include <iostream>
#include <random>
#include <mpi.h>

double calculatePi(long long num_samples, unsigned int seed) {
    long long num_points_in_circle = 0;

    // Для рандома
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    for (long long i = 0; i < num_samples; i++) {
        double x = dis(gen);
        double y = dis(gen);
        double distance_squared = x * x + y * y;
        if (distance_squared <= 1) {
            num_points_in_circle++;
        }
    }

    return 4.0 * num_points_in_circle / num_samples;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long total_samples = 100000000; // значение по умолчанию
    if (argc > 1) {
        total_samples = std::stoll(argv[1]);
    }
    long long samples_per_process = total_samples / size;
    if (rank == size - 1) {
    samples_per_process = total_samples - (size - 1) * samples_per_process; // если total_samples не делится нацело на количество процессов
    }
    
    unsigned int seed = time(NULL) + rank; // Генерация seed для каждого процесса

    double start_time = MPI_Wtime();
    double local_pi = calculatePi(samples_per_process, seed);
    double end_time = MPI_Wtime();
    double local_elapsed_time = end_time - start_time;

    double global_pi;
    double global_elapsed_time;

    MPI_Reduce(&local_pi, &global_pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_elapsed_time, &global_elapsed_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        global_pi /= size;
        std::cout << "ПИ: " << global_pi << "   Время выполнения: " << global_elapsed_time << " секунд" << std::endl;
    } else {
        MPI_Reduce(&local_pi, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_elapsed_time, NULL, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    return 0;
}
