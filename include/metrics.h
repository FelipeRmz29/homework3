#ifndef METRICS_H
#define METRICS_H

#include "scheduler.h" // Necesario para usar las estructuras process_t y metrics_t

// --- Prototipo de la Función de Cálculo de Métricas ---

/**
 * @brief Calcula las métricas de rendimiento globales (TAT, WT, RT promedio,
 * utilización de CPU, rendimiento y equidad) después de la simulación.
 * * @param processes Array de procesos con resultados de simulación.
 * @param n Número de procesos.
 * @param total_time Tiempo total que duró la simulación.
 * @param metrics Puntero a la estructura donde se guardarán los resultados.
 */
void calculate_metrics(process_t *processes, int n, int total_time,
                       metrics_t *metrics);

#endif // METRICS_H
