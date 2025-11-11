#include <stdio.h>
#include <math.h>
#include "../include/scheduler.h"
#include "../include/metrics.h"

// Nota: Asume que las estructuras process_t y metrics_t están definidas en scheduler.h

/**
 * @brief Calcula todas las métricas de rendimiento para un conjunto de procesos
 * después de una simulación.
 * * @param processes Array de procesos con resultados (start_time, completion_time, etc.).
 * @param n Número de procesos en el array.
 * @param total_time Tiempo total de simulación (completion_time del último proceso).
 * @param metrics Puntero a la estructura metrics_t donde se almacenarán los resultados.
 */
void calculate_metrics(process_t *processes, int n, int total_time, metrics_t *metrics) {
    if (n == 0 || total_time == 0) {
        // Inicializar a 0 si no hay procesos o el tiempo total es 0
        metrics->avg_turnaround_time = 0.0;
        metrics->avg_waiting_time = 0.0;
        metrics->avg_response_time = 0.0;
        metrics->cpu_utilization = 0.0;
        metrics->throughput = 0.0;
        metrics->fairness_index = 0.0;
        return;
    }

    double total_tat = 0.0;
    double total_wt = 0.0;
    double total_rt = 0.0;
    double total_burst = 0.0;
    
    // Para Jain's Fairness Index
    double sum_xi = 0.0;            // Suma de xi (Turnaround Time)
    double sum_xi_squared = 0.0;    // Suma de xi^2
    int completed_processes = 0;

    // 1. Iterar sobre todos los procesos para calcular métricas individuales y sumas
    for (int i = 0; i < n; i++) {
        // Solo considerar procesos que realmente se completaron (completion_time > 0)
        if (processes[i].completion_time > 0) {
            
            // a. Turnaround Time (TAT) = Completion Time - Arrival Time
            processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
            
            // b. Waiting Time (WT) = Turnaround Time - Burst Time
            processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
            
            // c. Response Time (RT) = Start Time - Arrival Time
            // Se asume que start_time >= 0, ya que se inicializa a -1
            processes[i].response_time = processes[i].start_time - processes[i].arrival_time;
            
            // d. Sumas para promedios
            total_tat += processes[i].turnaround_time;
            total_wt += processes[i].waiting_time;
            total_rt += processes[i].response_time;
            
            // e. Sumas para CPU Utilization y contador
            total_burst += processes[i].burst_time;
            completed_processes++;

            // f. Sumas para Jain's Fairness Index (usando TAT como xi)
            double tat = (double)processes[i].turnaround_time;
            sum_xi += tat;
            sum_xi_squared += tat * tat;
        }
    }

    // Si no se completó ningún proceso (simulación incompleta, aunque no debería pasar si total_time > 0)
    if (completed_processes == 0) {
        metrics->avg_turnaround_time = 0.0;
        // ... (el resto también a 0)
        return;
    }

    // 2. Calcular métricas promedio
    metrics->avg_turnaround_time = total_tat / completed_processes;
    metrics->avg_waiting_time = total_wt / completed_processes;
    metrics->avg_response_time = total_rt / completed_processes;
    
    // 3. CPU Utilization (Utilización de CPU)
    // CPU Utilization = (Busy Time / Total Time) × 100
    // Busy Time = total_burst (suma de todos los tiempos de ráfaga completados)
    metrics->cpu_utilization = (total_burst / total_time) * 100.0;
    
    // 4. Throughput (Rendimiento)
    // Throughput = Procesos Completados / Total Time
    metrics->throughput = (double)completed_processes / (double)total_time;
    
    // 5. Jain's Fairness Index (Índice de Equidad de Jain)
    // Fórmula: $J = (\sum x_i)^2 / (n \times \sum x_i^2)$
    // donde $x_i$ es el Turnaround Time y $n$ es el número de procesos completados.
    if (sum_xi_squared > 0) {
        metrics->fairness_index = (sum_xi * sum_xi) / (completed_processes * sum_xi_squared);
    } else {
        metrics->fairness_index = 0.0; // Si sum_xi_squared es 0, no hay equidad o no hay procesos.
    }
}
