#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

// Estructura auxiliar para almacenar los resultados de la comparación
typedef struct {
    char name[20];
    metrics_t metrics;
    int total_time;
} algorithm_result_t;

// --- Prototipo de la función auxiliar ---
void run_all_algorithms(process_t *original_processes, int n, algorithm_result_t *results, int num_algorithms);

/**
 * @brief Genera un informe de rendimiento en formato Markdown.
 * @param filename Nombre del archivo de salida (ej: "report.md").
 * @param original_processes El conjunto de procesos utilizado como carga de trabajo.
 * @param n Número de procesos.
 */
void generate_report(const char *filename, process_t *original_processes, int n) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo de reporte");
        return;
    }

    // Array para almacenar los resultados de 5 algoritmos
    algorithm_result_t results[5];
    int num_algorithms = 5;
    
    // Configuración para MLFQ (Ejemplo)
    int mlfq_quantums[] = {2, 4, 8};
    mlfq_config_t mlfq_config = {
        .num_queues = 3,
        .quantums = mlfq_quantums,
        .boost_interval = 10
    };

    // 1. Ejecutar y recopilar resultados de todos los algoritmos
    run_all_algorithms(original_processes, n, results, num_algorithms);

    // 2. Encontrar el mejor algoritmo (basado en Avg TAT)
    double min_tat = 99999.0;
    const char *best_alg = "N/A";
    for (int i = 0; i < num_algorithms; i++) {
        if (results[i].metrics.avg_turnaround_time < min_tat) {
            min_tat = results[i].metrics.avg_turnaround_time;
            best_alg = results[i].name;
        }
    }

    // 3. Escribir el informe en Markdown

    // --- Título del Informe ---
    fprintf(file, "# 📊 Informe de Rendimiento del Planificador de CPU\n\n");
    
    // --- Sección de Procesos ---
    fprintf(file, "## Conjunto de Procesos Analizado\n");
    fprintf(file, "| PID | Arrival | Burst | Priority |\n");
    fprintf(file, "|-----|---------|-------|----------|\n");
    for (int i = 0; i < n; i++) {
        fprintf(file, "| %-3d | %-7d | %-5d | %-8d |\n",
                original_processes[i].pid, 
                original_processes[i].arrival_time, 
                original_processes[i].burst_time,
                original_processes[i].priority);
    }
    fprintf(file, "\n");

    // --- Sección de Comparación ---
    fprintf(file, "## Comparación de Algoritmos\n");
    fprintf(file, "| Algorithm | Avg TAT | Avg WT | Avg RT | Throughput | CPU Util | Fairness |\n");
    fprintf(file, "|-----------|---------|--------|--------|------------|----------|----------|\n");
    for (int i = 0; i < num_algorithms; i++) {
        fprintf(file, "| %-9s | %-7.2f | %-6.2f | %-6.2f | %-10.4f | %-8.2f | %-8.4f |\n",
                results[i].name,
                results[i].metrics.avg_turnaround_time,
                results[i].metrics.avg_waiting_time,
                results[i].metrics.avg_response_time,
                results[i].metrics.throughput,
                results[i].metrics.cpu_utilization,
                results[i].metrics.fairness_index);
    }
    fprintf(file, "\n");

    // --- Sección de Análisis ---
    fprintf(file, "## Análisis y Recomendaciones\n");
    fprintf(file, "El algoritmo con el **menor tiempo de retorno promedio (Avg TAT)** para esta carga de trabajo fue **%s** (%.2f unidades de tiempo).\n\n", best_alg, min_tat);
    
    fprintf(file, "### Conclusiones Clave\n");
    fprintf(file, "* **Para trabajos de Lote (Batch Jobs):** Algoritmos como **SJF** o **STCF** suelen ser óptimos para minimizar el TAT y el WT.\n");
    fprintf(file, "* **Para sistemas Interactivos:** **Round Robin (RR)** o **MLFQ** son preferibles debido a su bajo **Tiempo de Respuesta (Avg RT)**.\n");
    fprintf(file, "* **Equidad (Fairness Index):** RR y MLFQ suelen tener mejores índices de equidad al garantizar que ningún proceso espere indefinidamente (a menos que haya un problema de inanición).\n");

    fclose(file);
    printf("✅ Informe de rendimiento generado en: %s\n", filename);
}

/**
 * @brief Función auxiliar que ejecuta todos los planificadores para la comparación.
 */
void run_all_algorithms(process_t *original_processes, int n, algorithm_result_t *results, int num_algorithms) {
    
    // Arrays de trabajo para cada simulación
    process_t current_processes[MAX_PROCESSES];
    timeline_event_t timeline[MAX_TIMELINE_EVENTS];

    // Configuración MLFQ (copia local para los nombres de los quantums)
    int mlfq_quantums[] = {2, 4, 8};
    mlfq_config_t mlfq_config = {
        .num_queues = 3,
        .quantums = mlfq_quantums,
        .boost_interval = 10
    };
    
    // Definiciones de algoritmos a ejecutar
    struct {
        const char *name;
        void (*scheduler)(process_t*, int, ...); // Puntero de función genérico
        int param; // Quantum para RR
    } alg_defs[] = {
        {"FIFO", (void (*)(process_t*, int, ...))schedule_fifo, 0},
        // Nota: SJF usa el mismo prototipo que FIFO/STCF si el planificador lo maneja internamente
        {"SJF", (void (*)(process_t*, int, ...))schedule_sjf, 0}, 
        {"STCF", (void (*)(process_t*, int, ...))schedule_stcf, 0},
        {"RR (q=3)", (void (*)(process_t*, int, ...))schedule_rr, 3}, // Usar quantum=3
        {"MLFQ", (void (*)(process_t*, int, ...))schedule_mlfq, 0}
    };
    
    // La función `reset_processes` debe estar disponible (incluida/definida)
    extern void reset_processes(process_t *processes, int n, process_t *original);

    for (int i = 0; i < num_algorithms; i++) {
        // A. Resetear y cargar procesos
        reset_processes(current_processes, n, original_processes);
        memset(timeline, 0, sizeof(timeline));
        
        // B. Ejecutar el planificador
        if (strcmp(alg_defs[i].name, "FIFO") == 0 || strcmp(alg_defs[i].name, "SJF") == 0 || strcmp(alg_defs[i].name, "STCF") == 0) {
            alg_defs[i].scheduler(current_processes, n, timeline);
        } else if (strstr(alg_defs[i].name, "RR") != NULL) {
            schedule_rr(current_processes, n, alg_defs[i].param, timeline);
        } else if (strcmp(alg_defs[i].name, "MLFQ") == 0) {
            schedule_mlfq(current_processes, n, &mlfq_config, timeline);
        }
        
        // C. Calcular el tiempo total de simulación
        int total_time = 0;
        for (int j = 0; j < n; j++) {
            if (current_processes[j].completion_time > total_time) {
                total_time = current_processes[j].completion_time;
            }
        }

        // D. Calcular métricas
        metrics_t metrics;
        calculate_metrics(current_processes, n, total_time, &metrics);

        // E. Almacenar resultados
        strcpy(results[i].name, alg_defs[i].name);
        results[i].metrics = metrics;
        results[i].total_time = total_time;
    }
}
