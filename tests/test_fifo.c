#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h> // Para realizar las aserciones
#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

// --- Workload de Prueba (Workload 1) ---
process_t test_processes[] = {
    // PID | Arrival | Burst | Priority
    {1, 0, 5, 1, 0, -1, 0, 0, 0, 0, 0, 0}, 
    {2, 1, 3, 2, 0, -1, 0, 0, 0, 0, 0, 0}, 
    {3, 2, 8, 1, 0, -1, 0, 0, 0, 0, 0, 0}  
};
const int NUM_TEST_PROCESSES = 3;

// --- Prototipo de la función auxiliar para resetear procesos (si no está disponible externamente) ---
// Asume que la implementación está en scheduler.c o es accesible.
extern void reset_processes(process_t *processes, int n, process_t *original);

/**
 * @brief Función principal de prueba para el algoritmo FIFO.
 */
void test_schedule_fifo() {
    printf("--- Ejecutando test_schedule_fifo ---\n");

    // 1. Inicialización de datos
    process_t processes[MAX_PROCESSES];
    timeline_event_t timeline[MAX_TIMELINE_EVENTS];
    metrics_t metrics;
    
    // Usamos reset_processes para cargar la copia limpia del workload
    reset_processes(processes, NUM_TEST_PROCESSES, test_processes);
    
    // 2. Ejecutar el algoritmo FIFO
    schedule_fifo(processes, NUM_TEST_PROCESSES, timeline);
    
    // 3. Cálculos y Verificación de Tiempos Finales (Valores Esperados para FIFO)
    // El orden de ejecución es P1 (5s), P2 (3s), P3 (8s)
    
    // P1 (Arrival=0, Burst=5):
    // Start=0, Completion=5
    assert(processes[0].start_time == 0);
    assert(processes[0].completion_time == 5);
    
    // P2 (Arrival=1, Burst=3):
    // Start=5, Completion=5+3=8
    assert(processes[1].start_time == 5);
    assert(processes[1].completion_time == 8);
    
    // P3 (Arrival=2, Burst=8):
    // Start=8, Completion=8+8=16
    assert(processes[2].start_time == 8);
    assert(processes[2].completion_time == 16);

    printf("  ✅ Verificación de Tiempos de Completación (Start/Completion) OK.\n");

    // 4. Calcular métricas
    int total_time = processes[2].completion_time; // El tiempo total es 16
    calculate_metrics(processes, NUM_TEST_PROCESSES, total_time, &metrics);

    // 5. Verificación de Métricas (Valores Esperados para FIFO)
    
    // P1: TAT=5-0=5, WT=5-5=0, RT=0-0=0
    // P2: TAT=8-1=7, WT=7-3=4, RT=5-1=4
    // P3: TAT=16-2=14, WT=14-8=6, RT=8-2=6
    // Total TAT = 5 + 7 + 14 = 26
    // Avg TAT = 26 / 3 = 8.666...
    assert(fabs(metrics.avg_turnaround_time - 8.67) < 0.01);

    // Total WT = 0 + 4 + 6 = 10
    // Avg WT = 10 / 3 = 3.333...
    assert(fabs(metrics.avg_waiting_time - 3.33) < 0.01);
    
    // Total RT = 0 + 4 + 6 = 10
    // Avg RT = 10 / 3 = 3.333...
    assert(fabs(metrics.avg_response_time - 3.33) < 0.01);
    
    // CPU Utilization: Total Burst (5+3+8=16) / Total Time (16) = 100.0%
    assert(fabs(metrics.cpu_utilization - 100.0) < 0.01);

    printf("  ✅ Verificación de Métricas Promedio OK.\n");
    printf("--- test_schedule_fifo PASSED ---\n");
}

int main() {
    test_schedule_fifo();
    return 0;
}
