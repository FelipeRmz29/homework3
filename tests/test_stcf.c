#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h> // Para fabs
#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

// --- Workload de Prueba (Workload 2: Prueba de Preempción) ---
// T=0: Solo P1 llega (Burst=8). P1 ejecuta 1s.
// T=1: P2 llega (Burst=4). Remaining_P1=7. Se elige P2. P2 ejecuta 4s.
// T=5: P2 termina. P3 llega (Burst=9). Remaining_P1=7. Se elige P1. P1 ejecuta 7s.
// T=12: P1 termina. P3 ejecuta 9s.
// T=21: P3 termina.

process_t test_processes[] = {
    // PID | Arrival | Burst | Priority | Rem | Start | Comp | TAT | WT | RT
    {1, 0, 8, 1, 0, -1, 0, 0, 0, 0, 0, 0}, 
    {2, 1, 4, 1, 0, -1, 0, 0, 0, 0, 0, 0}, 
    {3, 5, 9, 1, 0, -1, 0, 0, 0, 0, 0, 0}  
};
const int NUM_TEST_PROCESSES = 3;
const int EXPECTED_TOTAL_TIME = 21; // 8 + 4 + 9 = 21

// Prototipo de la función auxiliar para resetear procesos (si no está disponible externamente)
extern void reset_processes(process_t *processes, int n, process_t *original);

/**
 * @brief Función principal de prueba para el algoritmo STCF (Preemptive).
 */
void test_schedule_stcf() {
    printf("--- Ejecutando test_schedule_stcf (Preemptive) ---\n");

    // 1. Inicialización de datos
    process_t processes[MAX_PROCESSES];
    timeline_event_t timeline[MAX_TIMELINE_EVENTS];
    metrics_t metrics;
    
    // Usamos reset_processes para cargar la copia limpia del workload
    reset_processes(processes, NUM_TEST_PROCESSES, test_processes);
    
    // 2. Ejecutar el algoritmo STCF
    schedule_stcf(processes, NUM_TEST_PROCESSES, timeline);
    
    // 3. Verificación de Tiempos Finales (Valores Esperados)
    
    // P1 (PID 1, Burst 8, Arrivo 0): Ejecuta [0-1] y [5-12]
    // Start=0, Completion=12. TAT=12. WT = 12-8 = 4. RT=0.
    assert(processes[0].start_time == 0);
    assert(processes[0].completion_time == 12);
    
    // P2 (PID 2, Burst 4, Arrivo 1): Ejecuta [1-5]
    // Start=1, Completion=5. TAT=4. WT = 4-4 = 0. RT=0.
    assert(processes[1].start_time == 1);
    assert(processes[1].completion_time == 5);
    
    // P3 (PID 3, Burst 9, Arrivo 5): Ejecuta [12-21]
    // Start=12, Completion=21. TAT=16. WT = 9+2=11. RT=7. (12-5=7)
    // Nota: Aunque P3 llegó en T=5, tuvo que esperar hasta T=12.
    assert(processes[2].start_time == 12);
    assert(processes[2].completion_time == 21);

    printf("  ✅ Verificación de Tiempos de Completación (Start/Completion) OK.\n");

    // 4. Calcular métricas
    int total_time = EXPECTED_TOTAL_TIME;
    calculate_metrics(processes, NUM_TEST_PROCESSES, total_time, &metrics);

    // 5. Verificación de Métricas (Valores Esperados)
    
    // TATs: P1=12, P2=4, P3=16. Total TAT = 32. Avg TAT = 32 / 3 ≈ 10.666...
    assert(fabs(metrics.avg_turnaround_time - 10.67) < 0.01);

    // WTs: P1=4, P2=0, P3=12. Total WT = 16. Avg WT = 16 / 3 ≈ 5.333...
    assert(fabs(metrics.avg_waiting_time - 5.33) < 0.01);
    
    // RTs: P1=0, P2=0, P3=7. Total RT = 7. Avg RT = 7 / 3 ≈ 2.333...
    assert(fabs(metrics.avg_response_time - 2.33) < 0.01);
    
    // CPU Utilization: Total Burst (21) / Total Time (21) = 100.0%
    assert(fabs(metrics.cpu_utilization - 100.0) < 0.01);

    printf("  ✅ Verificación de Métricas Promedio OK.\n");
    printf("--- test_schedule_stcf PASSED ---\n");
}

int main() {
    test_schedule_stcf();
    return 0;
}
