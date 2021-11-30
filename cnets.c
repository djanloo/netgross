/*
Module introduced for fast MDE calculation.

Network must be given in normalized mode:
  -> node labels start from 0
  -> no holes in labels
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
// #include <numpy/ndarrayobject.h>
// #include <numpy/ndarraytypes.h>

typedef struct sparserow{
    long i;
    long j;
    double d;
} SparseRow;

typedef struct node{
    long n;
    double value;
    long childs_number;
    long * childs;
    double * distances;
    double * position;
} Node;

typedef struct link{
    Node * node1;
    Node * node2;
    double distance;
} Link;

typedef struct graph{
    long N_nodes;
    long N_links;
    Node * nodes;
} Graph;

// global variables remain the same call after call (?)
Graph G;

void link_nodes(Node * node, long child_index, double distance){
    // adds label of child to child array
    printf("\tnode %li has %li childs currently\n", node -> n, node -> childs_number);
    long * new_childs = (long *) malloc(sizeof(long)*    ((*node).childs_number + 1)   );
    for (long k = 0; k < node -> childs_number; k++){
        if (node -> childs_number != 0) {
            printf("\t-- init copy\n");
            new_childs[k] = node->childs[k];
            printf("\t-- end copy\n");
        }
    }
    new_childs[node -> childs_number] = child_index;
    (*node).childs = new_childs;

    // adds distance of child to distances
    double * new_distances = (double *) malloc(sizeof(double)*((*node).childs_number + 1));

    for (long k = 0; k < node -> childs_number; k++){
        new_distances[k] = node->distances[k];
    }

    new_distances[node -> childs_number] = distance;
    node -> distances = new_distances;

    node -> childs_number = (node -> childs_number) + 1;
    printf("\tnode %li has %li childs now (added node %li - %lf)\n", node -> n, node -> childs_number, node->childs[node -> childs_number-1], node->distances[node -> childs_number-1]);
    return ;
}

Graph to_Net(SparseRow * SM, double * values, long N_elements, long N_links){
    printf("creating network (N_elements = %li, N_links = %li)\n", N_elements, N_links);
    Graph g;
    g.nodes = (Node *) malloc(sizeof(Node)*N_elements);

    // values assignment
    for (long k = 0; k < N_elements; k++){
        printf("\tassigning node %li to %lf\n",k, values[k]);
        g.nodes[k].n = k;
        g.nodes[k].value = values[k];
        g.nodes[k].childs_number = 0;
    }

    // linking
    for (long k = 0; k < N_links; k++){
        printf("linking %li to %li at distance %lf\n",SM[k].i, SM[k].j, SM[k].d);
        link_nodes(&(g.nodes[SM[k].i]), SM[k].j, SM[k].d);
        link_nodes(&(g.nodes[SM[k].j]), SM[k].i, SM[k].d);
    }

    g.N_nodes = N_elements;
    g.N_links = N_links;

    //check 
    printf("summing up:\n");
    for (long k = 0; k < N_elements; k++){
        printf("\tnode %li has %li chids (", k, g.nodes[k].childs_number);
        for (long c = 0; c < g.nodes[k].childs_number;c++){
            printf("%lf - ", g.nodes[k].distances[c]);
        }
        printf(")\n");
    }
    return g;
}

double * PyList_to_double(PyObject * Pylist, long N_elements){
    double * dlist = (double *) malloc(sizeof(double)*N_elements);

    for (long i = 0; i < N_elements; i++){
        dlist[i] = PyFloat_AS_DOUBLE(PyList_GetItem(Pylist, i));
    }

    // for (int k = 0; k< N_elements; k++){
    //     printf("values[%d] = %lf\n", k, values[k]);
    // }
    return dlist;
}

SparseRow * PyList_to_SM(PyObject * list, long N_links){
    SparseRow * SM = (SparseRow *) malloc(sizeof(SparseRow)*N_links);
    for (long k = 0; k < N_links;k++){
        PyObject * row = PyList_GetItem(list,k);
        if (!PyList_Check(row)){
            printf("Invalid sparse list (row %li)\n", k);
            return NULL;
        }
    SM[k].i = PyLong_AsLong(PyList_GetItem(row,0));
    SM[k].j = PyLong_AsLong(PyList_GetItem(row,1));
    SM[k].d = PyFloat_AsDouble(PyList_GetItem(row,2));
    }
    return SM;
}

PyObject * init_network(PyObject * self, PyObject * args){

    // the pyobj for the two lists
    PyObject * Psparse = NULL;
    PyObject * Pvalues = NULL;

    // the sparse matrix and the values array
    SparseRow * SM = NULL;
    double * values = NULL;

    //take the args and divide them in two pyobjects
    if (!PyArg_ParseTuple(args,"OO",&Psparse, &Pvalues)) Py_RETURN_NONE;
    printf("Parsing stage passed\n");

    long N_elements = (long) PyList_Size(Pvalues);
    long N_links = (long) PyList_Size(Psparse);
    printf("N_element & N_links stage passed\n");

    // convert each element of the lists into a valid element of the C-object
    SM = PyList_to_SM(Psparse, N_links);
    values = PyList_to_double(Pvalues, N_elements);
    printf("Conversion stage passed\n");

    G = to_Net(SM, values, N_elements, N_links);
    printf("Network generation stage passed\n");

    for (long n = 0; n < G.N_nodes; n++){
        printf("node %li has %li childs:\n", G.nodes[n].n, G.nodes[n].childs_number);
        for (long c = 0; c < G.nodes[n].childs_number; c++){
            printf("\t%li: %li at distance %lF\n",c,  G.nodes[n].childs[c],G.nodes[n].distances[c]);
            //printf("\tnode %li at distance %lf\n",G.nodes[n].childs[c], G.nodes[n].distances[c]);
        }
    }
    Py_RETURN_NONE;
}

// Python link part - follow the API

//Method table definition: I have to undertsand whether it can contain more entries
static PyMethodDef cnetsMethods[] = {
    {"init_network", init_network, METH_VARARGS, "Initializes the network given a sparse list and a list of values"},
    {NULL, NULL, 0, NULL}//Guardian of The Table
};

//Module definition
static struct PyModuleDef cnetsmodule = {
    PyModuleDef_HEAD_INIT,
    "cnets",
    "Module for fast network computing",
    -1,
    cnetsMethods
};

//the motherfucking initialization function for the module
PyMODINIT_FUNC PyInit_cnets(void) {
    return PyModule_Create(&cnetsmodule);
}
