#include "graph.h"

#include <stdio.h>
#include <stdlib.h>

/* linked list advance */
void remove_linked_list(LinkedList *list, void *data) {
    if (!list)return;
    if (list->head->data == data) {
        remove_head_linked_list(list);
    } else if (list->tail->data == data) {
        remove_tail_linked_list(list);
    } else {
        Node *cur = list->head;
        while (cur) {
            if (cur->data == data) {
                Node *pre = cur->prev;
                Node *nxt = cur->next;
                pre->next = nxt;
                nxt->prev = pre;
                free_node(cur);
                cur = nxt;
            } else {
                cur = cur->next;
            }
        }
    }
}


/* initialise an empty graph */
/* return pointer to initialised graph */
Graph *init_graph(void) {
    return initialise_linked_list();
}

/* release memory for graph */
void free_graph(Graph *graph) {
    Node *cur;
    if (!graph) return;

    cur = graph->head;
    while (cur) {
        free_vertex((Vertex *) (cur->data));
        cur = cur->next;
    }
    free_linked_list(graph);
}

/* initialise a vertex */
/* return pointer to initialised vertex */
Vertex *init_vertex(int id) {
    Vertex *v = (Vertex *) malloc(sizeof(Vertex));
    if (!v) {
        fprintf(stderr, "error: unable to initialise vertex.\n");
        exit(EXIT_FAILURE);
    }
    v->id = id;
    v->edges = initialise_linked_list();
    return v;
}

/* release memory for initialised vertex */
void free_vertex(Vertex *vertex) {
    LinkedList *edges;
    Node *cur;
    if (!vertex)
        return;
    edges = vertex->edges;
    cur = edges->head;
    while (cur) {
        free_edge((Edge *) cur->data);
        cur = cur->next;
    }

    free_linked_list(vertex->edges);
    vertex->id = 0;
    free(vertex);
}

/* initialise an edge. */
/* return pointer to initialised edge. */
Edge *init_edge(void) {
    Edge *edge = (Edge *) malloc(sizeof(Edge));
    if (!edge) {
        fprintf(stderr, "error: unable to initialise edge.\n");
        exit(EXIT_FAILURE);
    }
    edge->weight = 0;
    edge->vertex = NULL;
    return edge;
}

/* release memory for initialised edge. */
void free_edge(Edge *edge) {
    if (!edge) return;
    free(edge);
}

/* remove all edges from vertex with id from to vertex with id to from graph. */
void remove_edge(Graph *graph, int from, int to) {
    Edge *e = NULL;
    if (!graph)
        return;

    while ((e = get_edge(graph, from, to)) != NULL) {
        remove_linked_list(find_vertex(graph, from)->edges, e);
        free_edge(e);
    }
}

/* remove all edges from vertex with specified id. */
void remove_edges(Graph *graph, int id) {
    Vertex *v;
    LinkedList* edges;
    Node* cur;
    if (!graph) return;

    if ((v = find_vertex(graph, id)) == NULL)
        return;

    edges = v->edges;
    cur = edges->head;

    while(cur) {
        free_edge((Edge*)(cur->data));
        cur = cur->next;
    }

    free_linked_list(edges);
    v->edges = initialise_linked_list();
}

static void print_edge(void *data) {
    Edge *edge = (Edge *) data;
    printf("%d[%.2f] ", edge->vertex->id, edge->weight);
}

/* output all vertices and edges in graph. */
/* each vertex in the graphs should be printed on a new line */
/* each vertex should be printed in the following format: */
/* vertex_id: edge_to_vertex[weight] edge_to_vertex[weight] ... */
/* for example: */
/* 1: 3[1.00] 5[2.00] */
/* indicating that vertex id 1 has edges to vertices 3 and 5 */
/* with weights 1.00 and 2.00 respectively */
/* weights should be output to two decimal places */
void print_graph(Graph *graph) {
    Node *next;
    if (!graph)return;
    next = graph->head;

    while (next) {
        Vertex *v = (Vertex *) (next->data);
        printf("%d: ", v->id);
        print_linked_list(v->edges, print_edge);
        printf("\n");
        next = next->next;
    }
}


/* find vertex with specified id in graph. */
/* return pointer to vertex, or NULL if no vertex found. */
Vertex *find_vertex(Graph *graph, int id) {
    Node *cur;
    if (!graph) return NULL;
    cur = graph->head;
    while (cur) {
        Vertex *v = (Vertex *) (cur->data);
        if (v->id == id)
            return v;
        cur = cur->next;
    }
    return NULL;
}

/* create and add vertex with specified id to graph. */
/* return pointer to vertex or NULL if an error occurs. */
/* if vertex with id already exists, return pointer to existing vertex. */
Vertex *add_vertex(Graph *graph, int id) {
    Vertex *v;
    if (!graph) return NULL;
    if ((v = find_vertex(graph, id)) != NULL)
        return v;

    v = init_vertex(id);
    append_linked_list(graph, v);
    return v;
}

/* remove vertex with specified id from graph. */
/* remove all edges between specified vertex and any other vertices in graph. */
void remove_vertex(Graph *graph, int id) {
    Vertex *v;
    if (!graph) return;
    if ((v = find_vertex(graph, id)) == NULL)
        return;
    remove_linked_list(graph, v);
    free_vertex(v);
}

/* add directed edge with specified weight between vertex with id from */
/* to vertex with id to. */
/* if no vertices with specified ids (from or to) exist */
/* then the vertices will be created. */
/* multiple vertices between the same pair of vertices are allowed. */
/* return pointer to edge, or NULL if an error occurs found. */
Edge *add_edge(Graph *graph, int from, int to, double weight) {
    Vertex *f;
    Vertex *t;
    Edge *e;
    if (!graph) return NULL;

    f = add_vertex(graph, from);
    t = add_vertex(graph, to);

    e = init_edge();
    e->vertex = t;
    e->weight = weight;

    append_linked_list(f->edges, e);

    return e;
}

/* add two edges to graph, one from vertex with id from to vertex with id to, */
/* and one from vertex with id to to vertex with id from. */
/* both edges should have the same weight */
/* if no vertices with specified ids (from or to) exist */
/* then the vertices will be created. */
/* multiple vertices between the same pair of vertices are allowed. */
void add_edge_undirected(Graph *graph, int from, int to, double weight) {
    add_edge(graph, from, to, weight);
    add_edge(graph, to, from, weight);
}

/* return array of node ids in graph. */
/* array of node ids should be dynamically allocated */
/* set count to be the number of nodes in graph */
/* return NULL if no vertices in graph */
int *get_vertices(Graph *graph, int *count) {
    int i;
    Node *next;
    int tmp[65536];
    Vertex *v;
    int *ids;
    if (!graph) return NULL;
    next = graph->head;
    *count = 0;

    while (next) {
        v = (Vertex *) next->data;
        tmp[*count] = v->id;
        (*count)++;
        next = next->next;
    }
    ids = (int *) malloc(sizeof(int) * (*count));
    for (i = *count - 1; i >= 0; i--) {
        ids[i] = tmp[i];
    }
    return ids;
}

/* return array of pointers to edges for a given vertex. */
/* array of edges should be dynamically allocated */
/* set count to be number of edges of vertex */
/* return NULL if no edges from/to vertex */
Edge **get_edges(Graph *graph, Vertex *vertex, int *count) {
    int i = 0;
    Edge *edge;
    Edge *edges[65536];
    Node *next;
    Edge **pEdge;
    if (!graph || !vertex) return NULL;
    vertex = find_vertex(graph, vertex->id);
    if (!vertex)
        return NULL;
    *count = 0;

    next = vertex->edges->head;
    while (next) {
        edge = (Edge *) next->data;
        edges[*count] = edge;
        (*count)++;
        next = next->next;
    }
    pEdge = (Edge **) malloc(sizeof(Edge *) * (*count));
    for (i = *count - 1; i >= 0; i--) {
        pEdge[i] = edges[i];
    }
    return pEdge;
}

/* return pointer to edge from vertex with id from, to vertex with id to. */
/* return NULL if no edge */
Edge *get_edge(Graph *graph, int from, int to) {
    Vertex *v;
    LinkedList *edges;
    Node *cur;
    if ((v = find_vertex(graph, from)) == NULL)
        return NULL;

    edges = v->edges;
    cur = edges->head;
    while (cur) {
        Edge *e = (Edge *) (cur->data);
        if (e && e->vertex && e->vertex->id == to)
            return e;
        cur = cur->next;
    }
    return NULL;
}

/* return id of destination node of edge. */
int edge_destination(Edge *edge) {
    if (!edge || !edge->vertex)return 0;
    return edge->vertex->id;
}

/* return weight of edge. */
double edge_weight(Edge *edge) {
    if (!edge) return 0;
    return edge->weight;
}
