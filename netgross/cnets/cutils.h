#include <Python.h>
#include <stdbool.h>

// Some colors for a fancy output (aestetics never goes out of fashion)
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

#define BRED "\e[1;31m"
#define RESET_COLOR "\e[0m"

#define infoprint(...) printf("cnets - "BLU "INFO" RESET_COLOR ": ");printf(__VA_ARGS__);printf(RESET_COLOR);fflush(stdout);
#define errprint(...) fprintf(stderr, "\ncnets - "BRED "ERROR" RESET_COLOR ": ");fprintf(stderr, __VA_ARGS__);printf(RESET_COLOR);fflush(stderr);
#define warnprint(...) fprintf(stderr, "\ncnets - "YEL "WARNING" RESET_COLOR ": ");fprintf(stderr, __VA_ARGS__);printf(RESET_COLOR);fflush(stderr);

typedef struct sparserow SparseRow;
extern float progress_bar_status;

void progress_bar(float progress, int length, int print_distortion);
SparseRow * PyList_to_SM(PyObject * list, unsigned long N_links);
float * PyList_to_float(PyObject * Pylist, unsigned int N_elements);
float euclidean_distance(float * pos1, float * pos2, unsigned int dim);
bool isNan(float number);
void print_float_array(float * array, int length);
void print_int_array(int * array, int length);
void sort_descendent(float * distances, int * indexes, int n);
void insert_f(float * array, float value, int position, int length);
void insert_i(int * array, int value, int position, int length);