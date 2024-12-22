CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2
LDFLAGS = -shared

# Папки
SRC_DIR = allocators
LIB_DIR = libs
BENCH_DIR = benchmarks
TEST_DIR = tests

# Исходники и библиотеки
FIRST_FIT_SRC = $(SRC_DIR)/first_fit_allocator.c
BEST_FIT_SRC = $(SRC_DIR)/best_fit_allocator.c
FIRST_FIT_LIB = $(LIB_DIR)/libfirstfit.dylib
BEST_FIT_LIB = $(LIB_DIR)/libbestfit.dylib
BENCHMARK = benchmark
TEST = allocator_test

# Компиляция библиотек
$(FIRST_FIT_LIB): $(FIRST_FIT_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BEST_FIT_LIB): $(BEST_FIT_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Компиляция бенчмарка
$(BENCHMARK): $(BENCH_DIR)/benchmark_allocator.c
	$(CC) -o $@ $^ -ldl

# Компиляция тестов
$(TEST): $(TEST_DIR)/allocator_test.c
	$(CC) -o $@ $^ -ldl

# Правила
all: $(FIRST_FIT_LIB) $(BEST_FIT_LIB) $(BENCHMARK) $(TEST)

clean:
	rm -f $(FIRST_FIT_LIB) $(BEST_FIT_LIB) $(BENCHMARK) $(TEST)
