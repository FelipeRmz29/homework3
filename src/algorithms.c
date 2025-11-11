#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/algorithms.h" // Se asume que este .h incluye los prototipos de las funciones schedule_*

// --- Funciones de Utilidad (Necesarias para qsort en C) ---

/**
 * @brief Función de comparación para qsort: ordena por arrival_time.
 * Usada principalmente en FIFO.
 */
int compare_arrival_time(const void *a, const void *b) {
    return ((process_t*)a)->arrival_time - ((process_t*)b)->arrival_time;
}

// --- Algoritmo 1: FIFO (First In First Out) ---

void schedule_fifo(process_t *processes, int n, timeline_event_t *timeline) {
    int current_time = 0;
    int timeline_idx = 0;
    
    // 1. Clonar y ordenar los procesos por tiempo de llegada (arrival_time)
    process_t *sorted_processes = malloc(n * sizeof(process_t));
    if (!sorted_processes) {
        perror("Fallo en la asignación de memoria para FIFO");
        return;
    }
    memcpy(sorted_processes, processes, n * sizeof(process_t));
    qsort(sorted_processes, n, sizeof(process_t), compare_arrival_time);
    
    // 2. Simulación
    for (int i = 0; i < n; i++) {
        process_t *p = &sorted_processes[i];

        // Manejar el tiempo de inactividad (IDLE) si el proceso no ha llegado
        if (current_time < p->arrival_time) {
            // Registrar tiempo de inactividad
            timeline[timeline_idx].time = current_time;
            timeline[timeline_idx].pid = -1; // IDLE
            timeline[timeline_idx].duration = p->arrival_time - current_time;
            timeline_idx++;
            current_time = p->arrival_time;
        }

        // 3. Ejecutar el proceso (No preemptivo)
        p->start_time = current_time;
        p->completion_time = current_time + p->burst_time;
        current_time = p->completion_time;

        // 4. Registrar evento en la línea de tiempo
        timeline[timeline_idx].time = p->start_time;
        timeline[timeline_idx].pid = p->pid;
        timeline[timeline_idx].duration = p->burst_time;
        timeline_idx++;
        
        // 5. Actualizar los resultados en el array original
        for (int j = 0; j < n; j++) {
            if (processes[j].pid == p->pid) {
                processes[j].start_time = p->start_time;
                processes[j].completion_time = p->completion_time;
                break;
            }
        }
    }
    
    // 6. Finalizar la línea de tiempo
    timeline[timeline_idx].time = current_time;
    timeline[timeline_idx].pid = 0; // Marca de fin
    timeline[timeline_idx].duration = 0;

    free(sorted_processes);
}

// -----------------------------------------------------------------

/**
 * @brief Función de utilidad para encontrar el proceso elegible más corto.
 * Usada por SJF (solo entre arribados) y STCF (preemptivo).
 */
process_t *find_shortest_remaining(process_t *processes, int n, int current_time) {
    process_t *shortest = NULL;
    int min_remaining = 2147483647; // MAX_INT

    for (int i = 0; i < n; i++) {
        // Criterio de elegibilidad: ha llegado y no ha terminado
        if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0) {
            if (processes[i].remaining_time < min_remaining) {
                min_remaining = processes[i].remaining_time;
                shortest = &processes[i];
            }
            // Criterio de desempate: Si el tiempo restante es igual, elegir el de menor arrival_time
            else if (processes[i].remaining_time == min_remaining && 
                     processes[i].arrival_time < shortest->arrival_time) {
                shortest = &processes[i];
            }
        }
    }
    return shortest;
}

// --- Algoritmo 2: SJF (Shortest Job First) ---

void schedule_sjf(process_t *processes, int n, timeline_event_t *timeline) {
    // SJF es no preemptivo. Se puede implementar de forma similar a FIFO, 
    // pero en el punto de selección, se elige el trabajo más corto de entre los *ya llegados*.
    // Pista: Usa la lógica de find_shortest_remaining y una variable para 
    // marcar los procesos completados, ya que no se pueden ordenar de antemano.
    
    printf("SJF no implementado. Usar find_shortest_remaining y simulación basada en tiempo.\n");
    // Implementación requerida...
}

// --- Algoritmo 3: STCF (Shortest Time to Completion First) ---

void schedule_stcf(process_t *processes, int n, timeline_event_t *timeline) {
    int current_time = 0;
    int completed_processes = 0;
    int timeline_idx = 0;
    process_t *current_process = NULL;
    int total_burst = 0;

    for (int i = 0; i < n; i++) {
        total_burst += processes[i].burst_time;
    }

    // Usar una marca de tiempo que garantice la finalización si no hay más procesos
    int max_simulation_time = total_burst + 100; 

    while (completed_processes < n && current_time < max_simulation_time) {
        // 1. Encontrar el proceso elegible con el menor tiempo restante
        process_t *next_process = find_shortest_remaining(processes, n, current_time);

        // 2. Manejo de IDLE o Fin de Simulación
        if (!next_process) {
            if (completed_processes == n) break; // Terminar
            
            // Avanzar al tiempo de llegada del siguiente proceso
            int next_arrival = max_simulation_time;
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > 0 && processes[i].arrival_time < next_arrival) {
                    next_arrival = processes[i].arrival_time;
                }
            }
            if (next_arrival <= max_simulation_time && next_arrival > current_time) {
                // Registrar IDLE
                timeline[timeline_idx].time = current_time;
                timeline[timeline_idx].pid = -1; // IDLE
                timeline[timeline_idx].duration = next_arrival - current_time;
                timeline_idx++;
                current_time = next_arrival;
                continue;
            } else {
                 current_time++; // Incrementar tiempo si no hay nada
                 continue;
            }
        }
        
        // 3. Manejo de Preempción / Inicio de Ejecución
        if (next_process != current_process) {
            // Iniciar un nuevo segmento en el Gráfico de Gantt
            timeline[timeline_idx].time = current_time;
            timeline[timeline_idx].pid = next_process->pid;
            timeline[timeline_idx].duration = 0; // Se actualizará en el paso de tiempo
            timeline_idx++;

            if (next_process->start_time == -1) {
                next_process->start_time = current_time;
            }
        }

        current_process = next_process;

        // 4. Ejecutar por una unidad de tiempo
        current_process->remaining_time--;
        timeline[timeline_idx - 1].duration++; // Extender el segmento actual
        current_time++;

        // 5. Manejar la finalización del proceso
        if (current_process->remaining_time == 0) {
            current_process->completion_time = current_time;
            completed_processes++;
            current_process = NULL; // Forzar la re-evaluación del planificador
        }
    }

    // 6. Finalizar la línea de tiempo
    timeline[timeline_idx].time = current_time;
    timeline[timeline_idx].pid = 0;
    timeline[timeline_idx].duration = 0;
}

// --- Algoritmo 4: Round Robin (RR) ---

void schedule_rr(process_t *processes, int n, int quantum, timeline_event_t *timeline) {
    // Pista: Implementar una cola circular (o un array/lista simple) para la Ready Queue.
    // Usar el campo remaining_time y preempción cuando el proceso usa el 'quantum' completo.
    
    printf("Round Robin no implementado. Se requiere gestionar una cola Ready y el quantum.\n");
    // Implementación requerida...
}

// --- Algoritmo 5: MLFQ (Multi-Level Feedback Queue) ---

void schedule_mlfq(process_t *processes, int n, mlfq_config_t *config, timeline_event_t *timeline) {
    // Pista: Requiere múltiples colas (ej: una lista de listas), la lógica de 
    // degradación (al usar el quantum completo) y el boost periódico (cada boost_interval).
    
    printf("MLFQ no implementado. Se requiere múltiples colas, lógica de boost y degradación.\n");
    // Implementación requerida...
}
