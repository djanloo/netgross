#include <Python.h>
#include <stdbool.h>

typedef struct sparserow SparseRow;
extern float progress_bar_status;

void progress_bar(float progress, int length);
SparseRow * PyList_to_SM(PyObject * list, unsigned long N_links);
float * PyList_to_double(PyObject * Pylist, unsigned int N_elements);
float euclidean_distance(float * pos1, float * pos2, unsigned int dim);
bool isNan(float number);