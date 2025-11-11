#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/scheduler.h"
#include "../include/algorithms.h"
#include "../include/metrics.h"

// --- Variables Globales de Estado del Simulador (Simplificadas) ---
// En una aplicación real, se usaría una estructura de datos para el estado.
process_t global_processes[MAX_PROCESSES];
int global_num_processes = 3; // Usaremos el Workload 1 de ejemplo
timeline_event_t global_timeline[MAX_TIMELINE_EVENTS];
metrics_t global_metrics;
int global_total_time = 15; // Tiempo máximo de simulación para escala

// Configuración de MLFQ (ejemplo, para inicializar el widget de parámetros)
mlfq_config_t mlfq_config = {
    .num_queues = 3,
    .quantums = (int[]){2, 4, 8}, // Nota: Esto es peligroso en C, mejor usar malloc/free
    .boost_interval = 10
};

// Variable de control del algoritmo actual
char *current_algorithm = "FIFO";
int current_quantum = 4; // Para Round Robin


// --- Funciones de Utilidad (Carga de Datos de Ejemplo) ---

// Carga el Workload 1 en la variable global
void load_workload_1() {
    process_t workload_1[] = {
        {1, 0, 5, 1, 0, -1, 0, 0, 0, 0, 0, 0}, 
        {2, 1, 3, 2, 0, -1, 0, 0, 0, 0, 0, 0}, 
        {3, 2, 8, 1, 0, -1, 0, 0, 0, 0, 0, 0}
    };
    global_num_processes = 3;
    memcpy(global_processes, workload_1, global_num_processes * sizeof(process_t));
}

// Llama al planificador basado en la selección actual
void run_simulation() {
    load_workload_1(); // Recargar el estado inicial de los procesos
    
    // Asumimos que tienes una función reset_processes para limpiar el estado
    // (debería estar en scheduler.c o ser incluida aquí)
    
    memset(global_timeline, 0, sizeof(global_timeline)); // Limpiar la línea de tiempo

    if (strcmp(current_algorithm, "FIFO") == 0) {
        schedule_fifo(global_processes, global_num_processes, global_timeline);
    } else if (strcmp(current_algorithm, "STCF") == 0) {
        schedule_stcf(global_processes, global_num_processes, global_timeline);
    } 
    // ... Implementar llamadas a SJF, RR, MLFQ
    
    // Calcular tiempo total y métricas
    global_total_time = 0;
    for (int i = 0; i < global_num_processes; i++) {
        if (global_processes[i].completion_time > global_total_time) {
            global_total_time = global_processes[i].completion_time;
        }
    }
    calculate_metrics(global_processes, global_num_processes, global_total_time, &global_metrics);
}


// --- 1. Dibujo del Gráfico de Gantt (Cairo) ---

// Función para asignar un color basado en el PID
void assign_color(cairo_t *cr, int pid) {
    // Usar colores simples basados en PID % 5
    switch (pid % 5) {
        case 1: cairo_set_source_rgb(cr, 0.2, 0.4, 0.8); break; // Azul
        case 2: cairo_set_source_rgb(cr, 0.8, 0.2, 0.2); break; // Rojo
        case 3: cairo_set_source_rgb(cr, 0.2, 0.8, 0.4); break; // Verde
        case 4: cairo_set_source_rgb(cr, 0.8, 0.6, 0.2); break; // Naranja
        default: cairo_set_source_rgb(cr, 0.5, 0.5, 0.5); break; // Gris
    }
}

/**
 * @brief Función callback de GTK para dibujar el Gráfico de Gantt.
 */
static gboolean draw_gantt_chart(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    if (global_total_time <= 0) return FALSE; // No dibujar si no hay simulación

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    double width = (double)allocation.width;
    double height = (double)allocation.height;
    
    // Espacio para la escala de tiempo
    double chart_height = height * 0.75;
    double time_scale_y = height * 0.85;

    // Escala de píxeles por unidad de tiempo
    double pixels_per_unit = width / (double)global_total_time;

    // 1. Dibujar el fondo
    cairo_set_source_rgb(cr, 0.95, 0.95, 0.95);
    cairo_rectangle(cr, 0, 0, width, chart_height);
    cairo_fill(cr);

    // 2. Dibujar la línea de tiempo
    double bar_height = chart_height * 0.4;
    double y_start = (chart_height - bar_height) / 2.0;

    for (int i = 0; global_timeline[i].pid != 0; i++) {
        int pid = global_timeline[i].pid;
        int duration = global_timeline[i].duration;
        
        double x = global_timeline[i].time * pixels_per_unit;
        double w = duration * pixels_per_unit;

        if (pid > 0) { // Proceso
            assign_color(cr, pid);
            cairo_rectangle(cr, x, y_start, w, bar_height);
            cairo_fill(cr);

            // Texto (PID)
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            char label[10];
            sprintf(label, "P%d", pid);
            cairo_move_to(cr, x + (w / 2.0) - 5.0, y_start + bar_height / 2.0 + 5.0);
            cairo_show_text(cr, label);
            
        } else if (pid == -1) { // IDLE
            cairo_set_source_rgb(cr, 0.7, 0.7, 0.7); // Gris claro
            cairo_rectangle(cr, x, y_start, w, bar_height);
            cairo_fill(cr);
        }
    }
    
    // 3. Dibujar la escala de tiempo (ejes X)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, 0, time_scale_y);
    cairo_line_to(cr, width, time_scale_y);
    cairo_stroke(cr);
    
    // Marcas de tiempo (cada 5 unidades de tiempo o menos si el total es pequeño)
    int interval = (global_total_time > 10) ? 5 : 1;
    for (int t = 0; t <= global_total_time; t += interval) {
        double x = t * pixels_per_unit;
        cairo_move_to(cr, x, time_scale_y);
        cairo_line_to(cr, x, time_scale_y + 5);
        cairo_stroke(cr);

        // Número de tiempo
        char time_label[10];
        sprintf(time_label, "%d", t);
        cairo_move_to(cr, x - 5, time_scale_y + 15);
        cairo_show_text(cr, time_label);
    }
    
    return FALSE;
}

// --- 2. Funciones de Callback y UI Update ---

GtkLabel *lbl_avg_tat;
GtkLabel *lbl_avg_wt;
GtkLabel *lbl_avg_rt;
GtkWidget *gantt_area;

/**
 * @brief Actualiza las etiquetas de métricas con los resultados de la simulación.
 */
void update_metrics_display() {
    char buffer[100];
    
    sprintf(buffer, "Avg TAT: %.2f", global_metrics.avg_turnaround_time);
    gtk_label_set_text(lbl_avg_tat, buffer);
    
    sprintf(buffer, "Avg WT: %.2f", global_metrics.avg_waiting_time);
    gtk_label_set_text(lbl_avg_wt, buffer);
    
    sprintf(buffer, "Avg RT: %.2f", global_metrics.avg_response_time);
    gtk_label_set_text(lbl_avg_rt, buffer);
    
    // Forzar redibujo del Gantt
    gtk_widget_queue_draw(gantt_area);
}

/**
 * @brief Callback cuando se presiona el botón "Run Simulation".
 */
static void on_run_clicked(GtkWidget *widget, gpointer data) {
    run_simulation();
    update_metrics_display();
    printf("Simulación ejecutada para el algoritmo: %s\n", current_algorithm);
}

/**
 * @brief Callback cuando se selecciona un nuevo algoritmo.
 */
static void on_algorithm_changed(GtkComboBox *combo_box, gpointer data) {
    current_algorithm = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box));
    if (current_algorithm) {
        printf("Algoritmo seleccionado: %s\n", current_algorithm);
        // Aquí se podría actualizar la visibilidad de los campos de parámetros (quantum, MLFQ config)
    }
}


// --- 3. Inicialización y Layout (Main Window) ---

/**
 * @brief Configura y muestra la ventana principal.
 */
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *vbox_main;
    GtkWidget *hbox_controls;
    GtkWidget *frame_gantt;
    GtkWidget *frame_metrics;

    // 0. Inicializar la simulación con datos de ejemplo
    load_workload_1();

    // Ventana Principal
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "CPU Scheduler Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Contenedor principal: VBox
    vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_main), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox_main);

    // --- A. Controles (Algoritmo, Parámetros, Botón Run) ---
    hbox_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_pack_start(GTK_BOX(vbox_main), hbox_controls, FALSE, FALSE, 5);

    // Selector de Algoritmo
    GtkWidget *algo_label = gtk_label_new("Algoritmo:");
    GtkWidget *algo_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algo_combo), "FIFO");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algo_combo), "SJF");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algo_combo), "STCF");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algo_combo), "Round Robin");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algo_combo), "MLFQ");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algo_combo), 0); // FIFO por defecto
    g_signal_connect(algo_combo, "changed", G_CALLBACK(on_algorithm_changed), NULL);

    // Botón Run
    GtkWidget *run_button = gtk_button_new_with_label("▶️ Run Simulation");
    g_signal_connect(run_button, "clicked", G_CALLBACK(on_run_clicked), NULL);

    // Empaquetar Controles
    gtk_box_pack_start(GTK_BOX(hbox_controls), algo_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox_controls), algo_combo, FALSE, FALSE, 5);
    // Aquí iría el input del Quantum/Prioridad...
    gtk_box_pack_start(GTK_BOX(hbox_controls), run_button, FALSE, FALSE, 15);

    // --- B. Gráfico de Gantt ---
    frame_gantt = gtk_frame_new("Gráfico de Gantt");
    gtk_box_pack_start(GTK_BOX(vbox_main), frame_gantt, TRUE, TRUE, 5); // TRUE para expandir

    gantt_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(gantt_area, 750, 150); // Tamaño inicial
    g_signal_connect(G_OBJECT(gantt_area), "draw", G_CALLBACK(draw_gantt_chart), NULL);
    gtk_container_add(GTK_CONTAINER(frame_gantt), gantt_area);

    // --- C. Dashboard de Métricas ---
    frame_metrics = gtk_frame_new("Métricas de Rendimiento");
    gtk_box_pack_start(GTK_BOX(vbox_main), frame_metrics, FALSE, FALSE, 5);

    GtkWidget *grid_metrics = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(frame_metrics), grid_metrics);
    gtk_grid_set_row_spacing(GTK_GRID(grid_metrics), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid_metrics), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid_metrics), 10);

    // Etiquetas estáticas
    gtk_grid_attach(GTK_GRID(grid_metrics), gtk_label_new("Tiempo de Retorno Promedio (TAT):"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_metrics), gtk_label_new("Tiempo de Espera Promedio (WT):"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_metrics), gtk_label_new("Tiempo de Respuesta Promedio (RT):"), 0, 2, 1, 1);
    // ... Agregar CPU Utilization, Throughput y Fairness Index ...

    // Etiquetas dinámicas (donde se mostrarán los resultados)
    lbl_avg_tat = GTK_LABEL(gtk_label_new("-"));
    lbl_avg_wt = GTK_LABEL(gtk_label_new("-"));
    lbl_avg_rt = GTK_LABEL(gtk_label_new("-"));
    
    gtk_grid_attach(GTK_GRID(grid_metrics), GTK_WIDGET(lbl_avg_tat), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_metrics), GTK_WIDGET(lbl_avg_wt), 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_metrics), GTK_WIDGET(lbl_avg_rt), 1, 2, 1, 1);


    gtk_widget_show_all(window);
}

/**
 * @brief Función principal de la aplicación GTK.
 */
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Inicializar el sistema GTK
    app = gtk_application_new("org.example.cpuscheduler", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
