#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cutils.h"
#include "cnets.h"

float progress_bar_status = 0;

typedef struct sparserow
{
    unsigned int i;
    unsigned int j;
    float d;
} SparseRow;

void progress_bar(float progress, int length, int print_distortion)
{     
    if ((int) (length*(progress - progress_bar_status)) >= 1 || progress == 1.0)
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
        printf(YEL);
        if (print_distortion){printf(" (D = %.4lf)", get_distortion());}
        printf(RESET_COLOR);
        fflush(stdout); 
        progress_bar_status = progress;
    }
}

SparseRow * PyList_to_SM(PyObject * list, unsigned long N_links){
    if (!PyList_CheckExact(list)){
        errprint("the sparse matrix must be an instance of list.\n");
    }
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

float * PyList_to_float(PyObject * Pylist, unsigned int N_elements){
    if (!PyList_CheckExact(Pylist)){
        errprint("conversions Py -> C of non-list type are not implemented yet.\n");
    }
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
    if (isNan(dist)){
        warnprint("euclidean distance turned out to be nan\n");
        print_float_array(pos1, dim);printf("\n");
        print_float_array(pos2, dim);printf("\n");
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

bool isOrdered(float * array, int n){
    bool sorted = true;
    for (int i = 0; i< n-1; i++){
        if (array[i] < array[i+1]){
            sorted = false;
            break;
        }
    }
    return sorted;
}

void sort_descendent(float * distances, int * indexes, int n)
{
    /* Please don't judge me, I could have done it fancier */
    float tmp_dist;
    int tmp_ind;
    while (!isOrdered(distances,n)){
        for (int i = 0; i < n-1; i++)
        {
            if (distances[i] < distances[i+1])
            {
                tmp_dist = distances[i];
                distances[i] = distances[i+1];
                distances[i+1] = tmp_dist;
                
                tmp_ind = indexes[i];
                indexes[i] = indexes[i+1];
                indexes[i+1] = tmp_ind;
            }
        }
    } 
}

void print_float_array(float * array, int length){
    printf("[ ");
    for (int d = 0; d < length; d++){
        printf("%lf,", array[d]);
    }
    printf("]");
}

void print_int_array(int * array, int length){
    printf("[ ");
    for (int d = 0; d < length; d++){
        printf("%d,", array[d]);
    }
    printf("]");
}


void insert_f(float * array, float value, int position, int length)
{
    /* Inserts an element in a float array by recursively shifting everything back */
    if (position >= length){
        errprint("insert_f - positioning outside array (%d >= %d )\n", position, length);
    }
    if (position != 0)
    {
        insert_f(array, (float) array[position], position - 1, length);
    }
    array[position] = value;
    return;
}
void insert_i(int * array, int value, int position, int length)
{
    /* Inserts an element in a int array by recursively shifting everything back */
    if (position >= length){
        errprint("insert_f - positioning outside array (%d >= %d )\n", position, length);
    }
    if (position != 0)
    {
        insert_i(array, (int) array[position], position - 1, length);
    }
    array[position] = value;
    return;
}