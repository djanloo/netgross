#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cutils.h"

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
        printf(GRN"\33[2K\r");
        for (i = 0; i < length; i++)
        {
            if (i == 0 || i == length - 1)
            {
                //string[i] = '|';
                printf("|");
            }
            else if (i < (int) (progress * length))
            {
                //string[i] = '|';
                printf("\xE2\x96\x92");
            }
            else
            {
                //string[i] = ' ';
                printf(" ");
            }
            string[length] = '\0';
        }
        printf("%d %%", (int)(100*progress));
        //printf(GRN"\33[2K\r%s %d %%", string, (int)(100*progress));
        printf(RESET_COLOR);
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
    SM[k].d = PyFloat_AsDouble(PyList_GetItem(row,2));
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

bool isNan(float number){
    if (number != number){
        return true;
    }
    else
    {
        return false;
    }
}


