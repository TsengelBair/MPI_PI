#include <iostream>
#include <random>
#include <mpi.h>
#include <chrono>

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
    auto start_time = std::chrono::high_resolution_clock::now(); // Начало отсчета времени

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

    auto parallel_start_time = std::chrono::high_resolution_clock::now(); // Начало отсчета времени параллельной части

    double local_pi = calculatePi(samples_per_process, seed);

    auto parallel_end_time = std::chrono::high_resolution_clock::now(); // Конец отсчета времени параллельной части
    auto parallel_duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(parallel_end_time - parallel_start_time).count();

    double global_pi;

    MPI_Reduce(&local_pi, &global_pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        global_pi /= size;
        auto end_time = std::chrono::high_resolution_clock::now(); // Конец отсчета времени
        auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_time - start_time).count();
        std::cout << "ПИ: " << global_pi << std::endl;
        std::cout << "Время выполнения: " << duration << " мс" << std::endl;
        std::cout << "Время выполнения параллельной части: " << parallel_duration << " мс" << std::endl;
        std::cout << "Доля последовательной части: " << (duration - parallel_duration)/duration << std::endl;
        std::cout << "Доля параллельной части: " << parallel_duration/duration << std::endl;
    } else {
        MPI_Reduce(&local_pi, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
