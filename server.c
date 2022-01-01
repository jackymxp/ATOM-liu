/*
 *  NetworkServer.c
 *  ProgrammingPortfolio Skeleton
 *
 */

/* You will need to include these header files to be able to implement the TCP/IP functions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <float.h>

/* You will also need to add #include for your graph library header files */
#include "graph.h"
#include "dijkstra.h"

// for debug
#define COMMAND_GRAPH_SHOW          "GRAPH-SHOW"

#define NDEBUG
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf( stderr, "DEBUG %s: %d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

// command
#define COMMAND_QUIT                "QUIT"
#define COMMAND_NET_ADD             "NET-ADD"
#define COMMAND_NET_DELETE          "NET-DELETE"
#define COMMAND_ROUTE_ADD           "ROUTE-ADD"
#define COMMAND_ROUTE_DELETE        "ROUTE-DELETE"
#define COMMAND_NET_LIST            "NET-LIST"
#define COMMAND_ROUTE_SHOW          "ROUTE-SHOW"
#define COMMAND_ROUTE_HOP           "ROUTE-HOP"
#define COMMAND_ROUTE_TABLE         "ROUTE-TABLE"

#define MESSAGE_SIZE 512

static void graph_show(Graph* graph, char* read_message, char* write_message) {
    char command[MESSAGE_SIZE];
    debug("GRAPH-SHOW\n");
    print_graph(graph);

    sprintf(write_message, "%s\n", "+OK GraphShow");
}
static void net_add(Graph* graph, char* read_message, char* write_message) {
    char command[MESSAGE_SIZE];
    int nid = -1;
    debug("NET-ADD\n");
    sscanf(read_message, "%s %d", command, &nid);
    debug("command : %s nid %d\n", command, nid);
    // net id > 0
    if (nid <= 0) {
        sprintf(write_message, "%s\n", "-ERR Added: NetID ");
        return ;
    } 
    if (find_vertex(graph, nid) != NULL) {
        sprintf(write_message, "%s %d %s\n", "-ERR Added", nid, "Has EXISTED");
        return ;
    }
    add_vertex(graph, nid);
    sprintf(write_message, "%s %d\n", "+OK Added", nid);
}


static void net_delete(Graph* graph, char* read_message, char* write_message) {
    char command[MESSAGE_SIZE];
    int nid = -1;
    debug("NET-DELETE\n");
    sscanf(read_message, "%s %d", command, &nid);
    debug("command : %s nid %d\n", command, nid);
    // net id > 0
    if (nid <= 0) {
        sprintf(write_message, "%s\n", "-ERR Deleted: NetID ");
        return ;
    }
    if (find_vertex(graph, nid) == NULL) {
        sprintf(write_message, "%s %d %s\n", "-ERR Deleted", nid, "No EXISTED");
        return ;
    }
    remove_vertex(graph, nid);
    sprintf(write_message, "%s %d\n", "+OK Deleted", nid);
}

static void route_add(Graph* graph, char* read_message, char* write_message) {
    int from = -1, to = -1, nid = -1;
    double weight = 0.0;
    char command[MESSAGE_SIZE];
    Edge* ft = NULL, *tf = NULL;

    debug("ROUTE-ADD\n");
    sscanf(read_message, "%s %d %d %lf", command, &from, &to, &weight);
    debug("command : %s from %d to %d weight %f\n", command, from, to, weight);
    if (from <= 0 || to <= 0 || weight <= 0) {
        sprintf(write_message, "%s\n", "-ERR ROUTE-ADD");
        return ;
    } 
    if (find_vertex(graph, from) == NULL || find_vertex(graph, to) == NULL) {
        sprintf(write_message, "%s %d %d %s\n", "-ERR ROUTE-ADD", from, to, " NOT EXISTED");
        return ;
    } 
    ft = get_edge(graph, from, to);
    tf = get_edge(graph, to, from);
    if (ft == NULL && tf == NULL) {
        add_edge_undirected(graph, from, to, weight);
        sprintf(write_message, "%s %d %s %d %s %lf\n", "+OK ROUTE-ADD:", from, " -> ", to, " : ",
                weight);
    } else if(ft != NULL && tf != NULL){
        debug("the route has exist from %lf to %lf\n", ft->weight, weight);
        sprintf(write_message, "%s %d %s %d %s %lf %s %lf\n", "+OK ROUTE-ADD EXISTED ", from, " -> ",
                to, " : ", ft->weight, " -->> ", weight);
        ft->weight = weight;
        tf->weight = weight;
    } else {
        sprintf(write_message, "%s\n", "-ERR : ROUTE-ADD: MUST NOT OCCUR");
    }
}


static void route_delete(Graph* graph, char* read_message, char* write_message) {
    int from = -1, to = -1 ,nid = -1;
    char command[MESSAGE_SIZE];
    debug("ROUTE-DELETE\n");
    sscanf(read_message, "%s %d %d", command, &from, &to);
    debug("command : %s from %d to %df\n", command, from, to);
    if (from <= 0 || to <= 0) {
        sprintf(write_message, "%s\n", "-ERR ROUTE-DELETE: NetID");
        return ;
    }

    if (!get_edge(graph, from, to) || !get_edge(graph, to, from)) {
        sprintf(write_message, "%s %d <-> %d %s\n", "-ERR ROUTE-DELETE", from, to, "NOT EXIST");
        return ;
    } 
    remove_edge(graph, from, to);
    remove_edge(graph, to, from);
    sprintf(write_message, "%s: %d <-> %d\n", "+OK ROUTE-DELETE", from, to);
}

static void route_show(Graph* graph, char* read_message, char* write_message) {
    int nid = -1,  nidCount = 0;
    Edge** pEdges = NULL;
    char command[MESSAGE_SIZE];
    char tmp[MESSAGE_SIZE];
    Vertex *vertex;

    debug("ROUTE-SHOW\n");
    sscanf(read_message, "%s %d", command, &nid);
    debug("command : %s nid %d\n", command, nid);
    // net id > 0
    if (nid <= 0) {
        sprintf(write_message, "%s\n", "-ERR ROUTE-SHOW: NetID ");
        return;
    } 
    if ((vertex = find_vertex(graph, nid)) == NULL) {
        sprintf(write_message, "%s\n", "-ERR ROUTE-SHOW: NetID NOT EXISTED");
        return ;
    }

    pEdges = get_edges(graph, vertex, &nidCount);
    sprintf(write_message, "%s %d\n", "+OK ROUTE-SHOW", nidCount);
    for (int i = 0; i < nidCount; i++) {
        sprintf(tmp, "%d\n", pEdges[i]->vertex->id);
        strcat(write_message, tmp);
    }
    // free edges
    free(pEdges);
}


static void net_list(Graph* graph, char* read_message, char* write_message) {
    int* ids;
    int nidCount = 0, i = 0;
    char command[MESSAGE_SIZE];
    char tmp[MESSAGE_SIZE];
    debug("NET-LIST\n");
    ids = get_vertices(graph, &nidCount);
    sprintf(write_message, "%s %d\n", "+OK", nidCount);
    for (i = 0; i < nidCount; i++) {
        debug("  %d\n", ids[i]);
        sprintf(tmp, "%d\n", ids[i]);
        strcat(write_message, tmp);
    }
    // free netId
    free(ids);
}

static void route_hop(Graph* graph, char* read_message, char* write_message) {
    int from = -1, to = -1, nidCount = 0;
    char command[MESSAGE_SIZE];
    Path* path;
    debug("ROUTE-HOP");
    sscanf(read_message, "%s %d %d", command, &from, &to);
    debug("command : %s from %d to %d\n", command, from, to);
    if (from <= 0 || to <= 0) {
        sprintf(write_message, "%s\n", "-ERR ROUTE-HOP:NetID");
        return ;
    } 
    if (from == to || find_vertex(graph, from) == NULL || find_vertex(graph, to) == NULL) {
        sprintf(write_message, "%s %d %d\n", "-ERR ROUTE-HOP", from, to);
        return ;
    } 
    path = dijkstra(graph, from, &nidCount);
    sprintf(write_message, "%s %d %.2f\n", "+OK ROUTE-HOP", path[to].next_hop, path[to].weight);
    free(path);
}

static void route_table(Graph* graph, char* read_message, char* write_message) {
    int nid = 0, nidCount = 0, i = 0;
    Path* path;
    int* ids;
    int pnEntry = 0;
    char tmp[MESSAGE_SIZE];
    char command[MESSAGE_SIZE];
    debug(COMMAND_ROUTE_TABLE);
    sscanf(read_message, "%s %d", command, &nid);
    debug("command : %s nid %d\n", command, nid);
    // net id > 0
    if (nid <= 0) {
        sprintf(write_message, "%s\n", "-ERR ROUTE-TABLE: NetID ");
        return ;
    } 
    path = dijkstra(graph, nid, &pnEntry);
    ids = get_vertices(graph, &nidCount);

    sprintf(write_message, "%s %d\n", "+OK ROUTE-TABLE", nid);

    for (i = 0; i < nidCount; i++) {
        Path *p = path + ids[i];
        debug("%d\n", ids[i]);
        if(nid == ids[i]) continue;
        if(p->weight != DBL_MAX)
            sprintf(tmp, "%d -> %d, next-hop %d, weight %8.2f\n", nid, ids[i], p->next_hop, p->weight);
        else
            sprintf(tmp, "%d -> %d, next-hop %d, weight     INF\n", nid, ids[i], p->next_hop);
        strcat(write_message, tmp);
    }
    free(ids);
    free(path);
}



int main(int argc, const char *argv[]) {
    int serverSocket, clientSocket;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    char connect_message[] = "+OK CONNECTED\n";
    char command[MESSAGE_SIZE];
    char write_message[MESSAGE_SIZE];
    char read_message[MESSAGE_SIZE];

    printf("Programming Portfolio 2021 Implementation\n");

    /* Insert your code here to create, and the TCP/IP socket for the Network Server
     *
     * Then once a connection has been accepted implement protocol described in the coursework description.
     */

    if (argc != 2) {
        fprintf(stderr, "usage: ./server <port>\n");
        exit(EXIT_FAILURE);
    }

    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "server socket error\n");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));


    if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        fprintf(stderr, "bind error\n");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) == -1) {
        fprintf(stderr, "listen error\n");
        exit(EXIT_FAILURE);
    }

    clnt_addr_size = sizeof(clnt_addr);
    if ((clientSocket = accept(serverSocket, (
                        struct sockaddr *) &clnt_addr, &clnt_addr_size)) == -1) {
        fprintf(stderr, "accept error\n");
        exit(EXIT_FAILURE);
    }

    // 第一次连接成功
    getsockname(clientSocket, (struct sockaddr *) &clnt_addr, &clnt_addr_size);

    debug("clint addr : %d\n", clnt_addr.sin_port);

    write(clientSocket, connect_message, sizeof(connect_message));

    Graph *graph = init_graph();

    while (1) {
        read(clientSocket, read_message, MESSAGE_SIZE);

        debug("read message %s\n", read_message);
        if (strstr(read_message, COMMAND_QUIT)) {
            break;
        } else if (strstr(read_message, COMMAND_ROUTE_TABLE)) {
            route_table(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_ROUTE_HOP)) {
            route_hop(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_NET_LIST)) {
            net_list(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_ROUTE_SHOW)) {
            route_show(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_NET_ADD)) {
            net_add(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_NET_DELETE)) {
            net_delete(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_ROUTE_ADD)) {
            route_add(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_ROUTE_DELETE)) {
            route_delete(graph, read_message, write_message);
        } else if (strstr(read_message, COMMAND_GRAPH_SHOW)) {
            graph_show(graph, read_message, write_message);
        } else {
            sprintf(write_message, "%s", "-ERR: NOT Implemented\n");
        }
        printf("%s\n", write_message);
        //write(clientSocket, write_message, sizeof(write_message));
    }
    printf("+OK QUIT\n");
    free_graph(graph);
    close(clientSocket);
    close(serverSocket);

    return 0;
}
