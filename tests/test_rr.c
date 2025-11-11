#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h> // Para fabs
#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

// --- Workload de Prueba (Workload 1) ---
// PID 1: Arrival=0, Burst=5
// PID 2: Arrival=1, Burst=3
// PID 3: Arrival=2, Burst=8
process_t test_processes[] = {
    // PID | Arrival | Burst | Priority | Rem | Start | Comp | TAT | WT | RT
    {1, 0, 5, 1, 0, -1, 0, 0, 0, 0, 0, 0}, 
    {2, 1, 3, 2, 0, -1, 0, 0, 0, 0, 0, 0}, 
    {3, 2, 8, 1, 0, -1, 0, 0, 0, 0, 0, 0}  
};
const int NUM_TEST_PROCESSES = 3;
const int TEST_QUANTUM = 3;

// Prototipo de la función auxiliar para resetear procesos (si no está disponible externamente)
extern void reset_processes(process_t *processes, int n, process_t *original);

/**
 * @brief Función principal de prueba para el algoritmo Round Robin (q=3).
 */
void test_schedule_rr() {
    printf("--- Ejecutando test_schedule_rr (Quantum=%d) ---\n", TEST_QUANTUM);

    // 1. Inicialización de datos
    process_t processes[MAX_PROCESSES];
    timeline_event_t timeline[MAX_TIMELINE_EVENTS];
    metrics_t metrics;
    
    // Usamos reset_processes para cargar la copia limpia del workload
    reset_processes(processes, NUM_TEST_PROCESSES, test_processes);
    
    // 2. Ejecutar el algoritmo Round Robin
    schedule_rr(processes, NUM_TEST_PROCESSES, TEST_QUANTUM, timeline);
    
    // 3. Verificación de Tiempos Finales (Valores Esperados para RR q=3)
    
    // Ejecución esperada (simulación detallada):
    // T=0: P1 llega. [P1(3), Rem=2]. Ready Queue: [P2, P3]
    // T=3: P1 preempta. [P2(3), Rem=0]. P2 termina. Ready Queue: [P3, P1]
    // T=6: P2 termina. [P3(3), Rem=5]. Ready Queue: [P1]
    // T=9: P3 preempta. [P1(2), Rem=0]. P1 termina. Ready Queue: [P3]
    // T=11: P1 termina. [P3(3), Rem=2]. Ready Queue: [P3]
    // T=14: P3 preempta. [P3(2), Rem=0]. P3 termina.
    // T=16: P3 termina. Total Time = 16.
    
    // P1 (PID 1, Burst 5, Arrivo 0):
    // Start=0, Completion=11. TAT=11. WT = 11-5 = 6. RT=0.
    assert(processes[0].start_time == 0);
    assert(processes[0].completion_time == 11);
    
    // P2 (PID 2, Burst 3, Arrivo 1):
    // Start=3, Completion=6. TAT=5. WT = 5-3 = 2. RT=2. (3-1=2)
    assert(processes[1].start_time == 3);
    assert(processes[1].completion_time == 6);
    
    // P3 (PID 3, Burst 8, Arrivo 2):
    // Start=6, Completion=16. TAT=14. WT = 14-8 = 6. RT=4. (6-2=4)
    assert(processes[2].start_time == 6);
    assert(processes[2].completion_time == 16);

    printf("  ✅ Verificación de Tiempos de Completación (Start/Completion) OK.\n");

    // 4. Calcular métricas
    int total_time = 16;
    calculate_metrics(processes, NUM_TEST_PROCESSES, total_time, &metrics);

    // 5. Verificación de Métricas (Valores Esperados para RR q=3)
    
    // TATs: P1=11, P2=5, P3=14. Total TAT = 30. Avg TAT = 30 / 3 = 10.0
    assert(fabs(metrics.avg_turnaround_time - 10.0) < 0.01);

    // WTs: P1=6, P2=2, P3=6. Total WT = 14. Avg WT = 14 / 3 ≈ 4.666...
    assert(fabs(metrics.avg_waiting_time - 4.67) < 0.01);
    
    // RTs: P1=0, P2=2, P3=4. Total RT = 6. Avg RT = 6 / 3 = 2.0
    assert(fabs(metrics.avg_response_time - 2.0) < 0.01);
    
    // CPU Utilization: Total Burst (16) / Total Time (16) = 100.0%
    assert(fabs(metrics.cpu_utilization - 100.0) < 0.01);

    printf("  ✅ Verificación de Métricas Promedio OK.\n");
    printf("--- test_schedule_rr PASSED ---\n");
}

int main() {
    test_schedule_rr();
    return 0;
}
