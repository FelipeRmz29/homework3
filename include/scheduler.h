#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 100           // Máximo número de procesos soportados
#define MAX_TIMELINE_EVENTS 1000    // Máximo número de eventos para el Gráfico de Gantt
#define MAX_QUEUES 5                // Máximo número de colas para MLFQ

// --- Estructuras de Datos Principales ---

/**
 * @brief Estructura que representa un proceso en el sistema.
 */
typedef struct {
    int pid;                    // ID del Proceso
    int arrival_time;           // Tiempo de llegada (T_a)
    int burst_time;             // Tiempo total de CPU requerido (T_b)
    int priority;               // Prioridad estática (menor valor = mayor prioridad)
    
    // Tiempos de Simulación y Control
    int remaining_time;         // Tiempo restante de CPU (para algoritmos preemptivos)
    int start_time;             // Primer tiempo en que se programa (para Response Time). Inicializar a -1.
    int completion_time;        // Tiempo en que finaliza (T_c)
    
    // Métricas calculadas
    int turnaround_time;        // T_c - T_a
    int waiting_time;           // TAT - T_b
    int response_time;          // Start_time - T_a
    
    // Campos Específicos para MLFQ
    int current_queue;          // Cola actual de prioridad en MLFQ
    int time_in_current_quantum; // Tiempo usado en el quantum actual de su cola
} process_t;

/**
 * @brief Estructura que registra un segmento de ejecución para el Gráfico de Gantt.
 */
typedef struct {
    int time;                   // Tiempo de inicio del segmento
    int pid;                    // PID del proceso ejecutándose (-1 para IDLE, 0 para marca de fin)
    int duration;               // Duración del segmento
} timeline_event_t;

/**
 * @brief Estructura para la configuración del algoritmo Multi-Level Feedback Queue (MLFQ).
 */
typedef struct {
    int num_queues;
    int quantums[MAX_QUEUES];   // Array de quantums para cada cola (Q0, Q1, ...)
    int boost_interval;         // Intervalo de tiempo para el "Priority Boost"
} mlfq_config_t;

/**
 * @brief Estructura para almacenar las métricas de rendimiento globales.
 */
typedef struct {
    double avg_turnaround_time;
    double avg_waiting_time;
    double avg_response_time;
    double cpu_utilization;
    double throughput;
    double fairness_index;      // Índice de equidad de Jain
} metrics_t;

#endif // SCHEDULER_H
