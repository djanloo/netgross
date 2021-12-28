#include <stdlib.h>
#include <stdio.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define infoprint(...) printf("cnets - INFO: ");printf(__VA_ARGS__);fflush(stdout);
#define errprint(...) fprintf(stderr, "cnets - ERROR: ");fprintf(stderr, __VA_ARGS__);fflush(stderr);

float progress_bar_status = 0;

typedef struct sparserow
{
    unsigned int i;
    unsigned int j;
    float d;
} SparseRow;


void progress_bar(float progress, int length)
{     
    if ((int) (length*(progress - progress_bar_status)) > 1)
    {   
        int i = 0;
        char * string = (char *) malloc( sizeof(char) * (length+1));

        for (i = 0; i < length; i++)
        {
            if (i == 0 || i == length - 1)
            {
                string[i] = '|';
            }
            else if (i < (int) (progress * length))
            {
                string[i] = '#';
            }
            else
            {
                string[i] = ' ';
            }
            string[length] = '\0';
        }
        printf("\33[2K\r%s %d %%", string, (int)(100*progress));
        fflush(stdout); 
        progress_bar_status = progress;
    }
}

SparseRow * PyList_to_SM(PyObject * list, unsigned long N_links){

    SparseRow * SM = (SparseRow *) malloc(sizeof(SparseRow)*N_links);
    for (unsigned long k = 0; k < N_links;k++)
    {
        PyObject * row = PyList_GetItem(list,k);
        if (!PyList_Check(row))
        {
            errprint("Invalid sparse list (row %li)\n", k);
            return NULL;
    }
    SM[k].i = (unsigned int) PyLong_AsLong(PyList_GetItem(row,0));
    SM[k].j = (unsigned int) PyLong_AsLong(PyList_GetItem(row,1));
    SM[k].d = (unsigned int) PyFloat_AsDouble(PyList_GetItem(row,2));
    }
    return SM;
}

float * PyList_to_double(PyObject * Pylist, unsigned int N_elements){

    float * dlist = (float *) malloc(sizeof(float)*N_elements);
    for (unsigned int i = 0; i < N_elements; i++)
    {
        dlist[i] = (float) PyFloat_AS_DOUBLE(PyList_GetItem(Pylist, i));
    }
    return dlist;
}

float euclidean_distance(float * pos1, float * pos2, unsigned int dim){

    float dist = 0.;
    for (unsigned int i = 0; i < dim; i++)
    {
        dist += pow(pos1[i] - pos2[i], 2);
    }
    return sqrt(dist);
}


