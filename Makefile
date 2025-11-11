# =================================================================
# CONFIGURACIÓN DEL PROYECTO
# =================================================================
# Target principal (GTK por defecto)
TARGET = scheduler_simulator_gtk

# Compiladores
CC = gcc

# Directorios
SRCDIR = src
INCLUDEDIR = include
TESTDIR = tests
DATADIR = data

# Archivos fuente principales
SRCS = $(SRCDIR)/scheduler.c $(SRCDIR)/algorithms.c $(SRCDIR)/metrics.c $(SRCDIR)/report.c

# Archivos objeto
OBJS = $(SRCS:$(SRCDIR)/%.c=%.o)

# =================================================================
# FLAGS DE COMPILACIÓN Y LIBRERÍAS
# =================================================================

# CFLAGS base: Advertencias, optimización de debug, incluir directorio de headers
CFLAGS = -Wall -Wextra -g -I$(INCLUDEDIR)

# Flags para GTK
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0) -lm

# Flags para NCURSES
NCURSES_LIBS = -lncurses -lm

# =================================================================
# REGLAS DE COMPILACIÓN PRINCIPAL (GTK)
# =================================================================

# Regla default: Compila la versión GTK
all: $(TARGET)

$(TARGET): $(OBJS) $(SRCDIR)/gui_gtk.c
	$(CC) $(CFLAGS) $(OBJS) $(SRCDIR)/gui_gtk.c -o $@ $(GTK_LIBS)

# Regla para compilar el módulo GUI por separado si es necesario
gui_gtk.o: $(SRCDIR)/gui_gtk.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# Regla genérica para compilar los archivos .o de la lógica central
%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# =================================================================
# REGLAS ALTERNATIVAS (NCURSES)
# =================================================================

scheduler_simulator_ncurses: $(OBJS) $(SRCDIR)/gui_ncurses.c
	$(CC) $(CFLAGS) $(OBJS) $(SRCDIR)/gui_ncurses.c -o $@ $(NCURSES_LIBS)
	@echo "✅ Ejecutable ncurses creado: scheduler_simulator_ncurses"

# =================================================================
# REGLAS PARA PRUEBAS UNITARIAS
# =================================================================

TEST_OBJS = $(SRCDIR)/algorithms.o $(SRCDIR)/metrics.o $(SRCDIR)/scheduler.o

# Compila y ejecuta todas las pruebas
test: test_fifo test_sjf test_stcf test_rr test_mlfq

# Regla genérica para construir un ejecutable de prueba
define TEST_RULE
test_$(1): tests/test_$(1).c $(TEST_OBJS)
	$(CC) $(CFLAGS) tests/test_$(1).c $(TEST_OBJS) -o tests/test_$(1)_bin $(NCURSES_LIBS)
	@echo "\n--- Ejecutando Test: $(1) ---"
	@./tests/test_$(1)_bin
	@rm -f tests/test_$(1)_bin
endef

$(eval $(call TEST_RULE,fifo))
$(eval $(call TEST_RULE,sjf))
$(eval $(call TEST_RULE,stcf))
$(eval $(call TEST_RULE,rr))
$(eval $(call TEST_RULE,mlfq))

# =================================================================
# REGLAS DE LIMPIEZA
# =================================================================

clean:
	@echo "Limpiando archivos objeto y binarios..."
	rm -f *.o $(TARGET) scheduler_simulator_ncurses
	rm -f $(TESTDIR)/*_bin
	rm -f report.md
