/*
Module introduced for fast MDE calculation.

Network must be given in normalized mode:
  -> node labels start from 0
  -> no holes in labels
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cutils.h"
#include "cnets.h"

#define PROGRESSS_BAR_LENGTH 60

typedef struct sparserow
{
    unsigned int i;
    unsigned int j;
    float d;
} SparseRow;

typedef struct node
{
    unsigned int n;   // Node label
    float value;       // Value of a general scalar quantity
    unsigned int childs_number;
    unsigned int * childs;
    float * distances;
    float * position; // Position in the embedding
} Node;

// Link structure is useless at the moment,
// may bind it to Linkin python later
typedef struct link
{
    Node * node1;
    Node * node2;
    float distance;
} Link;

typedef struct graph
{
    unsigned int N_nodes;
    unsigned long N_links;
    Node * nodes;
    unsigned int embedding_dimension;
} Graph;

int RAND_INIT = 0;
float NEGATIVE_SAMPLING_FRACTION = 0.1;
Graph G;

// Link nodes in the Graph
void link_nodes(Node * node, unsigned int child_index, float distance){
    if (distance <= 0)
    {
        warnprint("link(%d,%d) skipped - Distance must be a positive number (%lf)\n",node->n, child_index, distance);
        return;
    }
    if (node->n == child_index)
    {
        warnprint("autolink not allowed: skipped");
        return;
    }
    // Adds label of child to child array
    node -> childs = (unsigned int *) realloc(node -> childs, sizeof(unsigned int)*( (node -> childs_number) + 1));
    if (node -> childs == NULL)
    {
        errprint("!!! cannot allocate memory !!!\n");
        exit(-1);
    }
    (node -> childs)[node -> childs_number] = child_index;

    // Adds distances to distances array
    node -> distances = (float *) realloc(node -> distances, sizeof(float)*((*node).childs_number + 1));
    if (node -> distances == NULL)
    {
        errprint("!!! cannot allocate memory !!!\n");
        exit(-1);
    }

    (node -> distances)[node -> childs_number] = distance;

    node -> childs_number = (node -> childs_number) + 1;
    return ;
}

Graph to_Net(SparseRow * SM, float * values, unsigned int N_elements, unsigned long N_links){
    Graph g;
    g.nodes = (Node *) malloc(sizeof(Node)*N_elements);
    if (g.nodes == NULL)
    {
        errprint("!!! cannot allocate memory for %d nodes !!!\n", N_elements);
        exit(-1);
    }
    // Values assignment
    for (unsigned int k = 0; k < N_elements; k++)
    {
        g.nodes[k].n = k;
        g.nodes[k].value = values[k];
        g.nodes[k].childs_number = 0;
        g.nodes[k].childs = (unsigned int *) malloc(sizeof(unsigned int)); // Since each node has at least one child
        g.nodes[k].distances = (float *) malloc(sizeof(float));
    }

    // Linking
    for (unsigned long k = 0; k < N_links; k++)
    {
        link_nodes(&(g.nodes[SM[k].i]), SM[k].j, SM[k].d);
        link_nodes(&(g.nodes[SM[k].j]), SM[k].i, SM[k].d);
    }
    g.N_nodes = N_elements;
    g.N_links = N_links;
    return g;
}

unsigned int child_local_index_by_child_name(unsigned int node_number, unsigned int child_name)
{
    for (unsigned int child_local_index = 0; child_local_index < G.nodes[node_number].childs_number; child_local_index++)
        {
            if (G.nodes[node_number].childs[child_local_index] == child_name)
            {
                return child_local_index;
            }
        }
    return (unsigned int) -1;
}

void random_init(){
    if (RAND_INIT == 0)
    {
        srand(time(0));
    }
    else
    {
        srand(RAND_INIT);
    }

    for (unsigned int n = 0; n < G.N_nodes; n++)
    {
        G.nodes[n].position = (float *) malloc(sizeof(float)*G.embedding_dimension);
        for (unsigned int d = 0; d < G.embedding_dimension; d++)
        {
            G.nodes[n].position[d] = ((float) rand())/((float) RAND_MAX);
        }
    }
    return;
}

// Python enters here first 90% of the time
PyObject * init_network(PyObject * self, PyObject * args){

    // The PyObjs for the two lists
    PyObject * Psparse = NULL;
    PyObject * Pvalues = NULL;
    unsigned int embedding_dim = 0;

    // The C sparse matrix and the values array
    SparseRow * SM = NULL;
    float * values = NULL;

    // Take the args and divide them in two pyobjects
    if (!PyArg_ParseTuple(args,"OOi",&Psparse, &Pvalues, &embedding_dim))
    {
        printf("\n");
        errprint("init_network got bad arguments\n");
        Py_RETURN_NONE;
    }

    unsigned int N_elements = (unsigned int) PyList_Size(Pvalues);
    unsigned long N_links = (unsigned long) PyList_Size(Psparse);
    if (N_links < 1 ||  N_elements < 2)
    {
        errprint("invalid network G = (%d, %ld)\n", N_elements, N_links);
        exit(2);
    }

    // Convert each element of the lists into a valid element of the C-object
    SM = PyList_to_SM(Psparse, N_links);
    values = PyList_to_float(Pvalues, N_elements);

    infoprint("Generating network...");
    G = to_Net(SM, values, N_elements, N_links);
    free(SM);
    free(values);
    G.embedding_dimension = embedding_dim;
    printf("\tDone.\n");

    // Initializes the position randomly
    infoprint("Random initialization in R%d...", G.embedding_dimension);
    random_init();
    printf("\tDone.\n");
    nancheck();
    Py_RETURN_NONE;
}

void move_away_from_random_not_child(unsigned int node, float eps){
    // Picks a guy at random until it is not a child
    unsigned int not_child;
    unsigned int draws = 0;
    do{
        not_child = (unsigned int)((G.N_nodes-1)*((float) rand()/RAND_MAX));
        draws++;
        if (draws > G.N_nodes){
            return;
        }
    }while(child_local_index_by_child_name(node, not_child) != (unsigned int)-1 || node == not_child);
    float dist = euclidean_distance(G.nodes[node].position, G.nodes[not_child].position, G.embedding_dimension);
    for (unsigned int d = 0; d < G.embedding_dimension; d++)
    {
        G.nodes[node].position[d] += eps/(dist*dist)*(G.nodes[node].position[d] - G.nodes[not_child].position[d]);
        if (isNan(G.nodes[node].position[d])){
            errprint("NAN detected\n");
            printf("%d <-> %d = %lf\n", node, not_child, dist);
            exit(5);
        }
    }
    return;
}

PyObject * MDE(PyObject * self, PyObject * args){
    float eps = 0., neg_eps = 0.;
    unsigned int number_of_steps = 0;

    if (!PyArg_ParseTuple(args, "ffi", &eps, &neg_eps, &number_of_steps))
    {
        errprint("parsing MDE args\n");
        Py_RETURN_NONE;
    }
    infoprint("starting MDE with eps = %.3lf, neg_eps = %.3lf, Nsteps = %d\n",eps, neg_eps, number_of_steps);
    nancheck();
    float actual_distance = 0., factor;
    unsigned int child_index;
    for (unsigned int i = 0; i < number_of_steps; i++)
    {   
        progress_bar(((float)i)/( (float) number_of_steps) , PROGRESSS_BAR_LENGTH, 1);
        for (unsigned int current_node = 0; current_node < G.N_nodes; current_node++)
        {
            for (unsigned int current_child = 0; current_child < G.nodes[current_node].childs_number; current_child++ )
            {
                child_index = G.nodes[current_node].childs[current_child];
                actual_distance = euclidean_distance(G.nodes[current_node].position, G.nodes[child_index].position, G.embedding_dimension);
                if (actual_distance == 0.0){
                    warnprint("MDE - skipped update (zero distance in embedding): link(%d, %d) = %lf",current_node, child_index, G.nodes[current_node].distances[current_child]);
                }
                else
                {              
                    factor = eps*(1.- G.nodes[current_node].distances[current_child]/actual_distance)/G.nodes[current_node].childs_number;
                    for (unsigned int d = 0; d < G.embedding_dimension; d++)
                    {
                        G.nodes[current_node].position[d] += factor*(G.nodes[child_index].position[d] - G.nodes[current_node].position[d]) ;
                    }
                }
            }
            if (neg_eps != 0.){
                for (unsigned int mv_aw = 0; mv_aw < (unsigned int)(NEGATIVE_SAMPLING_FRACTION*G.N_nodes); mv_aw++)
                {
                    move_away_from_random_not_child(current_node, neg_eps);
                }
            }
        }
    }
    printf("\n");
    infoprint("MDE end\n");
    nancheck();
    progress_bar_status = 0;
    Py_RETURN_NONE;
}

PyObject * get_positions(PyObject * self, PyObject * args){
    // infoprint("Checking and passing positions back to python...");
    nancheck();
    PyObject * list = PyList_New(G.N_nodes);
    for (unsigned int n = 0; n < G.N_nodes; n++)
    {
        PyObject * single = PyList_New(G.embedding_dimension);
        for (unsigned int d = 0; d < G.embedding_dimension; d++)
        {
            PyList_SetItem(single, d, PyFloat_FromDouble(G.nodes[n].position[d]));
        }
        PyList_SetItem(list, n, single);
    }
    return list;
}

PyObject * get_distanceSM(PyObject * self, PyObject * args)
{

    PyObject * distanceSM = PyList_New(G.N_nodes*G.N_nodes); // Mmmh, not so clever! N**2 - > 9*N**2
    float d;
    long row_index = 0;
    Node node, another_node;

    for (unsigned int node_index = 0; node_index < G.N_nodes; node_index++)
    {   
        for (unsigned int another_node_index = 0; another_node_index < G.N_nodes; another_node_index++)
        {   
            PyObject * row = PyList_New(3);

            node = G.nodes[node_index];
            another_node = G.nodes[another_node_index];

            d = euclidean_distance(node.position, another_node.position, G.embedding_dimension);
            PyList_SetItem(row, 0, PyLong_FromLong(node.n));
            PyList_SetItem(row, 1, PyLong_FromLong(another_node.n));
            PyList_SetItem(row, 2, PyFloat_FromDouble(d));

            PyList_SetItem(distanceSM, row_index, row);
            row_index ++;
        }
        
    }
    return distanceSM;
}

PyObject * matrix_to_list_of_list(float **mat, unsigned int N)
{
    /* Returns a matrix as list of lists (lol).
        Waiting to implement numpy arrays. */
    PyObject * lol = PyList_New(G.N_nodes);

    for (unsigned int i = 0; i < N; i++)
    {   
        PyObject * i_th_row = PyList_New(N);
        for (unsigned int j = 0; j < N; j++)
        {   
            PyList_SetItem(i_th_row, j , PyFloat_FromDouble((double) mat[i][j]));
        }
        PyList_SetItem(lol, i , i_th_row);  
    }
    return lol;
}

PyObject * get_distanceM(PyObject * self, PyObject * args)
{
    infoprint("getting distances...");
    /* Returns a matrix of distances as list of lists.
        Waiting to implement numpy arrays. */
    float ** distanceM = (float**) malloc(sizeof(float*)*G.N_nodes);
    for (unsigned int k=0; k< G.N_nodes; k ++)
    {
        distanceM[k] = (float*) malloc(sizeof(float)*G.N_nodes);
    }
    float d;
    Node node, another_node;

    for (unsigned int node_index = 0; node_index < G.N_nodes; node_index++)
    {   
        for (unsigned int another_node_index = node_index; another_node_index < G.N_nodes; another_node_index++)
        {   
            node = G.nodes[node_index];
            another_node = G.nodes[another_node_index];
            d = euclidean_distance(node.position, another_node.position, G.embedding_dimension);
            distanceM[node_index][another_node_index] = d;
            distanceM[another_node_index][node_index] = d;
        }        
    }
    printf("Done.\n");
    return matrix_to_list_of_list(distanceM, G.N_nodes);
}

PyObject * set_target(PyObject * self, PyObject * args)
{
    // printf("cnets - updating target distances\n");
    PyObject * PySM;
    unsigned int node1_number, node2_number;

    if(! PyArg_ParseTuple(args, "O", &PySM))
    {
        errprint("set_target: paring failed\n");
    }
    SparseRow * SM = PyList_to_SM(PySM, G.N_links);
    for (unsigned long link = 0; link < G.N_links; link++)
    {
        node1_number = SM[link].i;
        node2_number = SM[link].j;
        // printf("cnet - set_target - (row %d of SM) --> (%d, %d) was (%lf), now is (%lf)\n", link, node1_number,node2_number, G.nodes[node1_number].distances[child_local_index_by_child_name(node1_number, node2_number)] , SM[link].d);
        
        G.nodes[node1_number].distances[child_local_index_by_child_name(node1_number, node2_number)] = SM[link].d;
        G.nodes[node2_number].distances[child_local_index_by_child_name(node2_number, node1_number)] = SM[link].d;

    }
    Py_RETURN_NONE;
}

PyObject * set_seed(PyObject * self, PyObject * args)
{
    int seed;
    if (!PyArg_ParseTuple(args, "i", &seed)){
        errprint("parsing failed in set_seed()\n");
    }
    RAND_INIT = seed;
    Py_RETURN_NONE;
}

PyObject * set_negative_sampling_fraction(PyObject * self, PyObject * args)
{
    float neg_samp_frac;
    if (!PyArg_ParseTuple(args, "f", &neg_samp_frac)){
        errprint("parsing failed in set_negative_sampling()\n");
    }
    if (neg_samp_frac > 1.){
        errprint("negative sampling fration must be > 0 and < 1\n");
    }
    NEGATIVE_SAMPLING_FRACTION = neg_samp_frac;
    Py_RETURN_NONE;
}


PyObject * stupid_knn(PyObject * self, PyObject * args)
{
    /* Implementation of a really stupid knn graph contruction
    by stupid N*N lookup in euclidean ndistance 
    I'm not proud of this but I'm waiting to implement EFANNA
    
    Args
    ----
        objects 
            a list of vectors in Rn
        k
            the k in knn

    */
    nancheck();
    PyObject * objects;
    int k;
    if (!PyArg_ParseTuple(args, "Oi", &objects, &k)){
       errprint("supid_knn - parsing failed\n");
       Py_RETURN_NONE;
    }
    if (k == 0){
        errprint("stupid_knn - k cannot be zero\n");
        exit(70000);
    }
    if (!PyList_CheckExact(objects)){
        errprint("stupid_knn - the given list of objects is not a list (numpy array not supported yet).\n");
        exit(5);
    }
    int N_objs = PyList_Size(objects);
    unsigned int obj_space_dim = PyList_Size(PyList_GetItem(objects, 0));

    infoprint("requested knn with k = %d of %d objects in R%d\n",k,N_objs, obj_space_dim);
    if (k > N_objs)
    {
        errprint("stupid_knn - k cannot be more than the total number of elements\n");
        exit(4);
    }
    // big stupid loop
    // please god forgive me for what I'm doing
    PyObject * sparse_knn = PyList_New(0), * tmp;
    int neighbor_indexes[k], init_index, k_, insertion_index, number_of_links = 0, found_initials = 0;
    float neighbor_distances[k], current_distance;
    float *obj_pos, *other_obj_pos;
    for (long obj_index = 0; obj_index < N_objs; obj_index++)
    {
        progress_bar(((float)obj_index)/( (float) N_objs) , PROGRESSS_BAR_LENGTH, 0);
        obj_pos = PyList_to_float(PyList_GetItem(objects, obj_index) , obj_space_dim);

        // Random initialization of neighbors array
        found_initials = 0;
        k_ = 0;
        while (found_initials < k)
        {
            init_index = (int) (obj_index + k_ + 1 )%N_objs;
            other_obj_pos = PyList_to_float(PyList_GetItem(objects, init_index), obj_space_dim);
            current_distance = euclidean_distance(obj_pos, other_obj_pos, obj_space_dim);
            if (current_distance != 0.0)
            {
                neighbor_indexes[found_initials] = init_index;
                neighbor_distances[found_initials] = current_distance;
                found_initials++;
            }else{
                k_ ++;
            }
        }
        sort_descendent(neighbor_distances, neighbor_indexes, k);
        for (long other_obj_index = 0; other_obj_index < N_objs; other_obj_index++)
        {
            if (other_obj_index != obj_index)
            {
                other_obj_pos = PyList_to_float(PyList_GetItem(objects, other_obj_index), obj_space_dim);
                current_distance = euclidean_distance(obj_pos, other_obj_pos, obj_space_dim);
                insertion_index = -1;

                for (k_ = 0; k_ < k; k_++)
                {
                    // cycles over the current closest neighbors
                    //
                    // e.g.:
                    // 5.0                      5.0                     5.0
                    // 8.0 6.0 2.0 1.0      8.0 6.0 2.0 1.0     8.0 6.0 2.0 1.0
                    if (current_distance > neighbor_distances[k_]) break;
                    else insertion_index = k_;
                }
                if (insertion_index > -1)
                {   
                    // this prevents redundancy like
                    // 8.0 6.0 2.0 2.0
                    // 1    2   3   3
                    if (neighbor_indexes[insertion_index] != other_obj_index)
                    {
                        insert_f(neighbor_distances, (float) current_distance, insertion_index, k); // the typecasting is the only thing that makes it work
                        insert_i(neighbor_indexes, (int) other_obj_index, insertion_index, k);
                    }
                }
            }
        }
        for (k_ = 0; k_ < k; k_++)
        {
            tmp = PyList_New(3);
            PyList_SetItem(tmp, 0, PyLong_FromLong(obj_index));
            PyList_SetItem(tmp, 1, PyLong_FromLong(neighbor_indexes[k_]));
            PyList_SetItem(tmp, 2, PyFloat_FromDouble(neighbor_distances[k_]));
            PyList_Append(sparse_knn, tmp);
            number_of_links++;
            if(obj_index == neighbor_indexes[k_]){
                errprint("something went wrong\n");
                exit(23974);
            }
        }
    }
    printf("\n");
    infoprint("stupid_knn done.\n");
    progress_bar_status = 0;
    return sparse_knn;
}

// TODO
PyObject * ball_neighbours(PyObject * self, PyObject * args){
    /* Given a set of points, cycles though them and collects, for each one,
    all the points which distance is less than a given threshold.
    */
}

// TODO 
PyObject * variable_metric_ball_neighbours(PyObject * self, PyObject * args){
    /* Using a variable metric generates a network in which each node has at least
    one neighbour but the number of neighbours is not fixed.

    To do so, first executes a first-nearest-neighbour.
    The closest neighbour distance is the unit distance of the metric.

    See UMAP algorithm.
    */
}



void nancheck()
{
    for (unsigned int d = 0; d < G.embedding_dimension; d++ )
    {
        for (unsigned int i = 0; i < G.N_nodes; i++)
        {
            if (isNan(G.nodes[i].position[d]))
            {
                errprint("NaN detected\n");
                exit(5);
            }
        }

    }
    return;
}

float get_distortion()
{
    unsigned int node = 0, child = 0;
    float distortion = 0, actual_distance;
    float * child_position;
    for (node = 0; node < G.N_nodes; node++)
    {
        for (child = 0; child < G.nodes[node].childs_number; child++)
        {
            child_position =  G.nodes[G.nodes[node].childs[child]].position;
            actual_distance = euclidean_distance(G.nodes[node].position, child_position, G.embedding_dimension);
            distortion += pow(actual_distance - G.nodes[node].distances[child], 2);
        }
    }
    distortion /= G.N_nodes;
    return distortion;
}

// Python wrapper
PyObject * Py_get_distortion(){
    return PyFloat_FromDouble(get_distortion());
}

// Python link part - follow the API ------------------------------------------------------------------------------------

// Methods table definition
static PyMethodDef cnetsMethods[] = {
    {"init_network", init_network, METH_VARARGS, "Initializes the network given a sparse list and a list of values.\nARGS\n\tsparse link matrix\t(list)\n\tvalues array\t(list)\n\tembedding dimension\t(int)"},
    {"MDE", MDE, METH_VARARGS, "Executes minumum distortion embedding routine"},
    {"get_positions", get_positions, METH_VARARGS, "Gives the computed positions of the network"},
    {"get_distanceSM", get_distanceSM, METH_VARARGS, "Returns the computed distance sparse matrix"},
    {"get_distanceM", get_distanceM, METH_VARARGS,"Returns the distance matrix"},
    {"get_distortion", Py_get_distortion, METH_VARARGS, "Returns the distortion of the network"},
    {"set_target", set_target, METH_VARARGS,"sets the target sparse matrix"},
    {"set_seed", set_seed, METH_VARARGS, "Set the seed for random numbers"},
    {"set_negative_sampling_fraction", set_negative_sampling_fraction, METH_VARARGS, "Set the fraction of random nodes used to perform negative sampling (0.0 < neg_samp_frac < 1.0)"},
    {"stupid_knn", stupid_knn, METH_VARARGS, "A stupid k-nearest neighbor graph generator."},
    {NULL, NULL, 0, NULL}//Guardian of The Table
};

// Module definition
static struct PyModuleDef cnetsmodule = {
    PyModuleDef_HEAD_INIT,
    "cnets",
    "Module for fast network computing",
    -1,
    cnetsMethods
};

// Initialization function for the module
PyMODINIT_FUNC PyInit_cnets(void) {
    return PyModule_Create(&cnetsmodule);
}
