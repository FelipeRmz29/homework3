#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

// --- Constantes y Definiciones de Ventanas ---
#define MAX_ROWS 30
#define MAX_COLS 100

// Alturas y anchos de las subventanas
#define HEIGHT_PROCESSES 10
#define HEIGHT_CONTROLS 3
#define HEIGHT_GANTT 5
#define HEIGHT_METRICS 8

// --- Variables Globales de Estado del Simulador ---
process_t global_processes[MAX_PROCESSES];
int global_num_processes = 3; 
timeline_event_t global_timeline[MAX_TIMELINE_EVENTS];
metrics_t global_metrics;
int global_total_time = 0; 
char current_algorithm_name[20] = "FIFO";
int current_quantum = 4;

// Ventanas de ncurses
WINDOW *win_main;
WINDOW *win_processes;
WINDOW *win_controls;
WINDOW *win_gantt;
WINDOW *win_metrics;
WINDOW *win_footer;

// --- Funciones de Utilidad (Simulación) ---

// Carga el Workload 1 de ejemplo
void load_workload_1() {
    process_t workload_1[] = {
        {1, 0, 5, 1, 0, -1, 0, 0, 0, 0, 0, 0}, 
        {2, 1, 3, 2, 0, -1, 0, 0, 0, 0, 0, 0}, 
        {3, 2, 8, 1, 0, -1, 0, 0, 0, 0, 0, 0}
    };
    global_num_processes = 3;
    memcpy(global_processes, workload_1, global_num_processes * sizeof(process_t));
}

// Llama al planificador y calcula métricas (Misma lógica que en gui_gtk.c)
void run_simulation() {
    // Nota: Aquí se debería llamar a reset_processes para limpiar los datos
    memset(global_timeline, 0, sizeof(global_timeline)); 
    
    // Ejecutar el algoritmo seleccionado
    if (strcmp(current_algorithm_name, "FIFO") == 0) {
        schedule_fifo(global_processes, global_num_processes, global_timeline);
    } else if (strcmp(current_algorithm_name, "STCF") == 0) {
        schedule_stcf(global_processes, global_num_processes, global_timeline);
    } 
    // ... Más llamadas a algoritmos ...
    
    // Calcular tiempo total y métricas
    global_total_time = 0;
    for (int i = 0; i < global_num_processes; i++) {
        if (global_processes[i].completion_time > global_total_time) {
            global_total_time = global_processes[i].completion_time;
        }
    }
    calculate_metrics(global_processes, global_num_processes, global_total_time, &global_metrics);
}

// --- Funciones de Dibujo (ncurses) ---

/**
 * @brief Dibuja la tabla de procesos.
 */
void draw_processes_window() {
    wclear(win_processes);
    box(win_processes, 0, 0);
    mvwprintw(win_processes, 0, 2, " Procesos ");
    
    mvwprintw(win_processes, 1, 1, " PID | Arrival | Burst | Priority | Start | Comp | TAT | WT | RT ");
    mvwprintw(win_processes, 2, 1, "-----+---------+-------+----------+-------+------+-----+----+----");

    for (int i = 0; i < global_num_processes; i++) {
        mvwprintw(win_processes, 3 + i, 1, 
                  " %-3d | %-7d | %-5d | %-8d | %-5d | %-4d | %-3d | %-2d | %-2d ",
                  global_processes[i].pid, 
                  global_processes[i].arrival_time, 
                  global_processes[i].burst_time,
                  global_processes[i].priority,
                  global_processes[i].start_time > 0 ? global_processes[i].start_time : -1,
                  global_processes[i].completion_time,
                  global_processes[i].turnaround_time,
                  global_processes[i].waiting_time,
                  global_processes[i].response_time);
    }
    wrefresh(win_processes);
}

/**
 * @brief Dibuja los controles de selección de algoritmo y parámetros.
 */
void draw_controls_window() {
    wclear(win_controls);
    box(win_controls, 0, 0);
    
    mvwprintw(win_controls, 1, 2, "Algorithm: [%s] Quantum: [%d] (Use 'A' to change)", 
              current_algorithm_name, current_quantum);
    
    wrefresh(win_controls);
}

/**
 * @brief Dibuja el Gráfico de Gantt en modo texto.
 */
void draw_gantt_window() {
    wclear(win_gantt);
    box(win_gantt, 0, 0);
    mvwprintw(win_gantt, 0, 2, " Gantt Chart ");
    
    // Fila de ejecución
    int row = 1;
    int col = 1;
    
    // Mapeo simple de tiempo a caracteres (1 unidad de tiempo = 1 carácter)
    int max_gantt_width = getmaxx(win_gantt) - 2; 

    for (int i = 0; global_timeline[i].pid != 0 && col < max_gantt_width; i++) {
        char label;
        int pid = global_timeline[i].pid;
        int duration = global_timeline[i].duration;

        if (pid == -1) {
            label = '-'; // IDLE
        } else if (pid > 0) {
            label = '0' + (pid % 10); // Usar el último dígito del PID como marcador
        } else {
            label = ' '; // Espacio vacío
        }
        
        for (int d = 0; d < duration && col < max_gantt_width; d++) {
            mvwaddch(win_gantt, row, col++, label);
        }
    }
    
    // Fila de escala de tiempo (cada 5 unidades)
    for (int t = 0; t <= global_total_time && t < max_gantt_width; t += 5) {
        mvwprintw(win_gantt, row + 1, t + 1, "%d", t);
    }
    
    wrefresh(win_gantt);
}

/**
 * @brief Dibuja la sección de métricas.
 */
void draw_metrics_window() {
    wclear(win_metrics);
    box(win_metrics, 0, 0);
    mvwprintw(win_metrics, 0, 2, " Metrics ");

    mvwprintw(win_metrics, 1, 2, "Avg Turnaround Time: %.2f", global_metrics.avg_turnaround_time);
    mvwprintw(win_metrics, 2, 2, "Avg Waiting Time:    %.2f", global_metrics.avg_waiting_time);
    mvwprintw(win_metrics, 3, 2, "Avg Response Time:   %.2f", global_metrics.avg_response_time);
    mvwprintw(win_metrics, 4, 2, "CPU Utilization:     %.2f%%", global_metrics.cpu_utilization);
    mvwprintw(win_metrics, 5, 2, "Fairness Index:      %.4f", global_metrics.fairness_index);
    
    wrefresh(win_metrics);
}

/**
 * @brief Dibuja el pie de página con las opciones de comando.
 */
void draw_footer_window() {
    wclear(win_footer);
    box(win_footer, 0, 0);
    mvwprintw(win_footer, 1, 2, 
              "[R]un | [A]lgorithm | [E]dit Processes | [Q]uit");
    wrefresh(win_footer);
}

/**
 * @brief Dibuja todas las ventanas.
 */
void redraw_all() {
    draw_processes_window();
    draw_controls_window();
    draw_gantt_window();
    draw_metrics_window();
    draw_footer_window();
    doupdate();
}

// --- Lógica Principal ---

void setup_ncurses() {
    // Inicializar ncurses
    initscr();
    cbreak();           // Capturar Ctrl+C (interrupción)
    noecho();           // No mostrar las teclas presionadas
    keypad(stdscr, TRUE); // Habilitar la captura de teclas especiales
    
    // Verificar el tamaño de la terminal
    if (LINES < MAX_ROWS || COLS < MAX_COLS) {
        endwin();
        fprintf(stderr, "Error: Terminal must be at least %dx%d\n", MAX_COLS, MAX_ROWS);
        exit(1);
    }
    
    // Crear las ventanas (subventanas dentro de stdscr)
    int start_row = 0;
    
    win_processes = newwin(HEIGHT_PROCESSES, MAX_COLS, start_row, 0);
    start_row += HEIGHT_PROCESSES;
    
    win_controls = newwin(HEIGHT_CONTROLS, MAX_COLS, start_row, 0);
    start_row += HEIGHT_CONTROLS;
    
    win_gantt = newwin(HEIGHT_GANTT, MAX_COLS, start_row, 0);
    start_row += HEIGHT_GANTT;
    
    win_metrics = newwin(HEIGHT_METRICS, MAX_COLS, start_row, 0);
    start_row += HEIGHT_METRICS;
    
    win_footer = newwin(3, MAX_COLS, start_row, 0);
}

void cleanup_ncurses() {
    delwin(win_processes);
    delwin(win_controls);
    delwin(win_gantt);
    delwin(win_metrics);
    delwin(win_footer);
    endwin(); // Finalizar ncurses
}

void main_loop() {
    int ch;
    
    // Simulación inicial al cargar
    run_simulation(); 
    redraw_all();

    while ((ch = getch()) != 'q' && ch != 'Q') {
        switch (ch) {
            case 'r':
            case 'R':
                // Ejecutar la simulación
                run_simulation();
                redraw_all();
                break;
            case 'a':
            case 'A':
                // Lógica para cambiar el algoritmo (requiere sub-rutina de entrada)
                // Aquí solo se simula el cambio
                if (strcmp(current_algorithm_name, "FIFO") == 0) {
                    strcpy(current_algorithm_name, "STCF");
                } else {
                    strcpy(current_algorithm_name, "FIFO");
                }
                draw_controls_window();
                break;
            case 'e':
            case 'E':
                // Lógica para editar procesos (requiere sub-rutina de entrada/edición)
                mvwprintw(win_footer, 1, 50, "Editing mode...");
                wrefresh(win_footer);
                break;
            default:
                // Mostrar mensaje de ayuda temporal
                mvwprintw(win_footer, 1, 50, "Unknown command: %c", ch);
                wrefresh(win_footer);
                break;
        }
    }
}

int main() {
    load_workload_1();

    setup_ncurses();
    
    // Dibujar el título en stdscr
    mvprintw(0, (MAX_COLS / 2) - 17, "CPU Scheduler Simulator v1.0 (ncurses)");
    refresh();

    main_loop();

    cleanup_ncurses();

    return 0;
}
