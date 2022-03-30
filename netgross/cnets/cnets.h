#define PROGRESSS_BAR_LENGTH 60

typedef struct sparserow SparseRow;

typedef struct node Node;

// Link structure is useless at the moment,
// may bind it to Linkin python later
typedef struct link Link;

typedef struct graph Graph;

// Global variables remain the same call after call
extern Graph G;

float get_distortion();
void nancheck();