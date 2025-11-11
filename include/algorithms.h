#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "scheduler.h" // Incluye las estructuras process_t, mlfq_config_t, timeline_event_t, etc.

// --- Prototipos de las Funciones de Planificación ---

/**
 * @brief Implementa el algoritmo FIFO (First In First Out).
 * No preemptivo. Ordena por arrival_time.
 */
void schedule_fifo(process_t *processes, int n, timeline_event_t *timeline);

/**
 * @brief Implementa el algoritmo SJF (Shortest Job First).
 * No preemptivo. Selecciona el trabajo con el menor burst_time entre los procesos arribados.
 */
void schedule_sjf(process_t *processes, int n, timeline_event_t *timeline);

/**
 * @brief Implementa el algoritmo STCF (Shortest Time to Completion First).
 * Preemptivo. Selecciona el proceso con el menor remaining_time entre los procesos arribados.
 */
void schedule_stcf(process_t *processes, int n, timeline_event_t *timeline);

/**
 * @brief Implementa el algoritmo Round Robin.
 * Preemptivo, basado en un quantum de tiempo.
 */
void schedule_rr(process_t *processes, int n, int quantum, 
                 timeline_event_t *timeline);

/**
 * @brief Implementa el algoritmo Multi-Level Feedback Queue (MLFQ).
 * Preemptivo, usa múltiples colas con diferentes quantums y boosting periódico.
 */
void schedule_mlfq(process_t *processes, int n, mlfq_config_t *config,
                   timeline_event_t *timeline);

#endif // ALGORITHMS_H
