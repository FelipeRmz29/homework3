#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/algorithms.h" // Prototipos de schedule_fifo, schedule_stcf, etc.
#include "../include/metrics.h"    // Prototipo de calculate_metrics

// --- Prototipos locales ---
void print_results(const char *alg_name, process_t *processes, int n, const metrics_t *metrics);
void print_timeline(timeline_event_t *timeline);
void reset_processes(process_t *processes, int n, process_t *original);

// Workload 1: Simple (3 procesos) para ejemplo inicial
process_t workload_1[] = {
    {1, 0, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0}, // PID 1, Arrivo 0, Burst 5, Prioridad 1
    {2, 1, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0}, // PID 2, Arrivo 1, Burst 3, Prioridad 2
    {3, 2, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0}  // PID 3, Arrivo 2, Burst 8, Prioridad 1
};
int num_processes = 3;


int main() {
    printf("==========================================\n");
    printf("  CPU Scheduler Simulator (CLI Version) \n");
    printf("==========================================\n");

    // Copia de los procesos originales para que cada simulación use datos limpios
    process_t original_processes[MAX_PROCESSES];
    memcpy(original_processes, workload_1, num_processes * sizeof(process_t));

    // Array para almacenar los procesos durante la simulación y sus resultados
    process_t current_processes[MAX_PROCESSES];
    // Array para almacenar la línea de tiempo (Gráfico de Gantt)
    timeline_event_t timeline[MAX_TIMELINE_EVENTS];
    // Estructura para almacenar las métricas
    metrics_t metrics;
    int total_time = 0; // Tiempo total de la simulación

    // --- Configuración MLFQ de ejemplo ---
    int mlfq_quantums[] = {2, 4, 8}; // Quantum para Q0, Q1, Q2
    mlfq_config_t mlfq_config = {
        .num_queues = 3,
        .quantums = mlfq_quantums,
        .boost_interval = 10
    };

    // ------------------------------------
    // 1. Simular FIFO
    // ------------------------------------
    printf("\n--- Simulación: FIFO ---\n");
    reset_processes(current_processes, num_processes, original_processes);
    schedule_fifo(current_processes, num_processes, timeline);

    // Calcular el tiempo total de simulación
    // Se asume que el tiempo total es la finalización del último proceso
    total_time = 0;
    for (int i = 0; i < num_processes; i++) {
        if (current_processes[i].completion_time > total_time) {
            total_time = current_processes[i].completion_time;
        }
    }
    
    // Si la simulación terminó en IDLE, buscar el último evento
    if (total_time == 0 && timeline[0].pid != 0) {
        for (int i = 0; timeline[i].pid != 0; i++) {
            total_time = timeline[i].time + timeline[i].duration;
        }
    }

    calculate_metrics(current_processes, num_processes, total_time, &metrics);
    print_results("FIFO", current_processes, num_processes, &metrics);
    print_timeline(timeline);


    // ------------------------------------
    // 2. Simular STCF
    // ------------------------------------
    printf("\n--- Simulación: STCF ---\n");
    reset_processes(current_processes, num_processes, original_processes);
    // Nota: Necesitas la implementación de schedule_stcf en algorithms.c
    schedule_stcf(current_processes, num_processes, timeline);

    total_time = 0; // Recalcular total_time para STCF (puede ser diferente)
    for (int i = 0; i < num_processes; i++) {
        if (current_processes[i].completion_time > total_time) {
            total_time = current_processes[i].completion_time;
        }
    }

    calculate_metrics(current_processes, num_processes, total_time, &metrics);
    print_results("STCF", current_processes, num_processes, &metrics);
    print_timeline(timeline);


    // Aquí irían las llamadas a SJF, Round Robin y MLFQ...

    return 0;
}

/**
 * @brief Restablece los procesos a su estado inicial para una nueva simulación.
 * @param processes Array de trabajo para la simulación.
 * @param n Número de procesos.
 * @param original Array con el estado inicial.
 */
void reset_processes(process_t *processes, int n, process_t *original) {
    for (int i = 0; i < n; i++) {
        // Copiar todos los campos base
        processes[i] = original[i]; 
        // Inicializar campos de simulación/resultado
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1; // Usar -1 para indicar que no ha comenzado
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = 0;
        processes[i].current_queue = 0;
        processes[i].time_in_current_quantum = 0;
    }
}

/**
 * @brief Imprime la línea de tiempo (Gantt) en la consola.
 */
void print_timeline(timeline_event_t *timeline) {
    printf("  Gantt Chart (Time: [Duration] PID): \n");
    printf("  ");
    
    int current_time = 0;
    for (int i = 0; timeline[i].pid != 0 && i < MAX_TIMELINE_EVENTS; i++) {
        // Omitir eventos con duración 0
        if (timeline[i].duration <= 0) continue; 
        
        char pid_str[10];
        if (timeline[i].pid == -1) {
            strcpy(pid_str, "IDLE");
        } else {
            sprintf(pid_str, "P%d", timeline[i].pid);
        }

        printf("%d: [%d] %s | ", timeline[i].time, timeline[i].duration, pid_str);
        current_time = timeline[i].time + timeline[i].duration;
    }
    printf("END (%d)\n", current_time);
}

/**
 * @brief Imprime la tabla de resultados y las métricas.
 */
void print_results(const char *alg_name, process_t *processes, int n, const metrics_t *metrics) {
    printf("\n  Resultados detallados (%s):\n", alg_name);
    printf("  +-----+---------+--------+------+-----+-----+-----+-----+-----+\n");
    printf("  | PID | Arrival | Burst  | Start| Comp| TAT | WT  | RT  | Prio|\n");
    printf("  +-----+---------+--------+------+-----+-----+-----+-----+-----+\n");
    for (int i = 0; i < n; i++) {
        printf("  | %-3d | %-7d | %-6d | %-4d | %-3d | %-3d | %-3d | %-3d | %-3d |\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].start_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].response_time,
               processes[i].priority);
    }
    printf("  +-----+---------+--------+------+-----+-----+-----+-----+-----+\n");

    printf("\n  Métricas de Rendimiento:\n");
    printf("  - Avg Turnaround Time: %.2f\n", metrics->avg_turnaround_time);
    printf("  - Avg Waiting Time:    %.2f\n", metrics->avg_waiting_time);
    printf("  - Avg Response Time:   %.2f\n", metrics->avg_response_time);
    printf("  - CPU Utilization:     %.2f%%\n", metrics->cpu_utilization);
    printf("  - Throughput:          %.4f (Proc/Unit Time)\n", metrics->throughput);
    printf("  - Jain's Fairness Index: %.4f\n", metrics->fairness_index);
}
