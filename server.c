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

#include "graph.h"
#include "dijkstra.h"

/* You will also need to add #include for your graph library header files */
#define NDEBUG
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf( stderr, "DEBUG %s: %d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

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

int main(int argc, const char *argv[]) {
    int serverSocket, clientSocket;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;


    char connect_message[] = "+OK CONNECTED\n";
    char command[MESSAGE_SIZE];
    char *p;
    char write_message[MESSAGE_SIZE];
    char read_message[MESSAGE_SIZE];
    int nid, from, to, nidCount, i;
    double weight;
    Edge *edge;
    int *ids;
    char tmp[128];
    Vertex *vertex;
    Edge **pEdges;
    Path *path;
    int pnEntry = 0;
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
        nidCount = 0;

        debug("read message %s\n", read_message);
        if (strstr(read_message, COMMAND_QUIT)) {
            debug("接收到退出命令\n");
            write(clientSocket, "+OK\n", sizeof("+OK\n"));
            break;
        } else if (strstr(read_message, COMMAND_ROUTE_TABLE)) {
            debug(COMMAND_ROUTE_TABLE);
            nid = 0;
            nidCount = 0;
            sscanf(read_message, "%s %d", command, &nid);
            debug("command : %s nid %d\n", command, nid);
            // net id > 0
            if (nid <= 0) {
                sprintf(write_message, "%s\n", "-ERR ROUTE-TABLE: NetID ");
            } else {
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
            }

        } else if (strstr(read_message, COMMAND_ROUTE_HOP)) {
            debug("ROUTE-HOP");
            from = 0, to = -1;
            sscanf(read_message, "%s %d %d", command, &from, &to);
            debug("command : %s from %d to %d\n", command, from, to);
            if (from <= 0 || to <= 0) {
                sprintf(write_message, "%s\n", "-ERR ROUTE-HOP:NetID");
            } else {
                if (from == to || find_vertex(graph, from) == NULL || find_vertex(graph, to) == NULL) {
                    sprintf(write_message, "%s %d %d\n", "-ERR ROUTE-HOP", from, to);
                } else {
                    path = dijkstra(graph, from, &nidCount);
                    sprintf(write_message, "%s %d %.2f\n", "+OK ROUTE-HOP", path[to].next_hop, path[to].weight);
                    free(path);
                }
            }
        } else if (strstr(read_message, COMMAND_NET_LIST)) {
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
        } else if (strstr(read_message, COMMAND_ROUTE_SHOW)) {
            debug("ROUTE-SHOW\n");
            nid = 0;
            sscanf(read_message, "%s %d", command, &nid);
            debug("command : %s nid %d\n", command, nid);
            // net id > 0
            if (nid <= 0) {
                sprintf(write_message, "%s\n", "-ERR ROUTE-SHOW: NetID ");
            } else {
                if ((vertex = find_vertex(graph, nid)) == NULL) {
                    sprintf(write_message, "%s\n", "-ERR ROUTE-SHOW: NetID NOT EXISTED");
                } else {
                    pEdges = get_edges(graph, vertex, &nidCount);
                    sprintf(write_message, "%s %d\n", "+OK ROUTE-SHOW", nidCount);
                    for (int i = 0; i < nidCount; i++) {
                        sprintf(tmp, "%d\n", pEdges[i]->vertex->id);
                        strcat(write_message, tmp);
                    }
                }
            }
        } else if ((strstr(read_message, COMMAND_NET_ADD)) != NULL) {
            debug("NET-ADD\n");
            nid = 0;
            sscanf(read_message, "%s %d", command, &nid);
            debug("command : %s nid %d\n", command, nid);
            // net id > 0
            if (nid <= 0) {
                sprintf(write_message, "%s\n", "-ERR Added: NetID ");
            } else {
                if (find_vertex(graph, nid) == NULL) {
                    add_vertex(graph, nid);
                    sprintf(write_message, "%s %d\n", "+OK Added", nid);
                } else {
                    sprintf(write_message, "%s %d %s\n", "-ERR Added", nid, "Has EXISTED");
                }
            }
        } else if ((p = strstr(read_message, COMMAND_NET_DELETE)) != NULL) {
            debug("NET-DELETE\n");
            nid = 0;
            sscanf(read_message, "%s %d", command, &nid);
            debug("command : %s nid %d\n", command, nid);
            // net id > 0
            if (nid <= 0) {
                sprintf(write_message, "%s\n", "-ERR Deleted: NetID ");
            } else {
                if (find_vertex(graph, nid) != NULL) {
                    remove_vertex(graph, nid);
                    sprintf(write_message, "%s %d\n", "+OK Deleted", nid);

                } else {
                    sprintf(write_message, "%s %d %s\n", "-ERR Deleted", nid, "No EXISTED");
                }
            }

        } else if ((p = strstr(read_message, COMMAND_ROUTE_ADD)) != NULL) {
            debug("ROUTE-ADD\n");
            from = 0, to = 0, weight = 0.0;
            sscanf(read_message, "%s %d %d %lf", command, &from, &to, &weight);
            debug("command : %s from %d to %d weight %f\n", command, from, to, weight);
            if (from <= 0 || to <= 0 || weight <= 0) {
                sprintf(write_message, "%s\n", "-ERR ROUTE-ADD");
            } else {
                if (find_vertex(graph, from) == NULL || find_vertex(graph, to) == NULL) {
                    sprintf(write_message, "%s %d %d %s\n", "-ERR ROUTE-ADD", from, to, " NOT EXISTED");
                } else {
                    if ((edge = get_edge(graph, from, to)) == NULL) {
                        add_edge_undirected(graph, from, to, weight);
                        sprintf(write_message, "%s %d %s %d %s %lf\n", "+OK ROUTE-ADD:", from, " -> ", to, " : ",
                                weight);
                    } else {
                        debug("the route has exist from %lf to %lf\n", edge->weight, weight);
                        sprintf(write_message, "%s %d %s %d %s %lf %s %lf\n", "+OK ROUTE-ADD EXISTED ", from, " -> ",
                                to, " : ", edge->weight, " -->> ", weight);
                        edge->weight = weight;
                    }
                }
            }

        } else if ((p = strstr(read_message, COMMAND_ROUTE_DELETE)) != NULL) {
            debug("ROUTE-DELETE\n");
            from = 0, to = 0;
            sscanf(read_message, "%s %d %d", command, &from, &to);
            debug("command : %s from %d to %df\n", command, from, to);
            if (from <= 0 || to <= 0) {
                sprintf(write_message, "%s\n", "-ERR ROUTE-DELETE: NetID");
            } else {
                if (get_edge(graph, from, to) && get_edge(graph, to, from)) {
                    remove_edge(graph, from, to);
                    remove_edge(graph, to, from);
                    sprintf(write_message, "%s: %d <-> %d\n", "+OK ROUTE-DELETE", from, to);
                } else {
                    sprintf(write_message, "%s %d <-> %d %s\n", "-ERR ROUTE-DELETE", from, to, "NOT EXIST");
                }
            }
        } else {
            sprintf(write_message, "%s", "-ERR NOT Implemented\n");
        }
        printf("%s\n", write_message);
        //write(clientSocket, write_message, sizeof(write_message));
    }
    free_graph(graph);
    close(clientSocket);
    close(serverSocket);

    return 0;
}
