/* 
 * File:   socket.c
 * Author: Ronak
 *
 * Created on March 12, 2016, 12:34 AM
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define MAX_TRY_CONN 10
#define CHUNK_SIZE 256

#define MY_MAXHOST 192
#define MY_MAXSERV 16
#define SEND_P 1	/* send potato */
#define REPORT_P 2  /* player's hops is 0, report to master */
#define EXIT 3 

typedef struct player_info_st{
    char host[MY_MAXHOST+1];
    char port[MY_MAXSERV+1];
    int id;
}player_info;


typedef struct master_core_st{
    char host[MY_MAXHOST+1];
    char port[MY_MAXSERV+1];
    int port1;
    int player_number;
    int hops;
    int lis_socket;   /* init in init_master(), destroyed in main() */
    player_info * player_list;
}master_core;

typedef struct potato_st{
    int type;
    int hops;
    int sent_times;
    int sender; /* -1: master, other number: player's index */
    int receiver;
}potato_msg;

typedef struct _neighbor_msg_st{
	int exit;
	int left_index;
	char left_host[MY_MAXHOST+1];
	char left_port[MY_MAXSERV+1];
	int right_index;
	char right_host[MY_MAXHOST+1];
	char right_port[MY_MAXSERV+1];
}neighbor_msg;

typedef struct player_core_st{
    /* this player's own host/port info */
    char host[MY_MAXHOST+1];
    char port[MY_MAXSERV+1];

    /* master's host/port info */
    char master_host[MY_MAXHOST+1];
    char master_port[MY_MAXSERV+1];


    int left_index;
    char left_host[MY_MAXHOST+1];
    char left_port[MY_MAXSERV+1];
    int right_index;
    char right_host[MY_MAXHOST+1];
    char right_port[MY_MAXSERV+1];

    int index;

    int lis_socket;  /* init in get_neighbors_info() */
    //int lis_port;

    int base_port; /* base_port is master's port, every player's port is BASE_PORT+index+1 */
}player_core;

int createSocket(){
    int soc;
    int yes = 1;
    soc= socket(AF_INET, SOCK_STREAM, 0);
    
    if (soc == -1){
        //printf("error in socket creation\n");
        exit(-1);
    }
/*
    if(setsocketopt(soc,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))== -1){
	printf("Error setting socketoptions\n");
    }
*/
    setsockopt(soc,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    //perror("setsockopt");

    return soc;
}

void bindSocket(int soc, int listen_port){
    //struct sockaddr_in sin;
    //struct hostent *hp;
    int rc;
    int j = 1;
    struct sockaddr_in serv_addr;
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(listen_port);
     
    //hp = gethostbyname(master_engine->host);
    
    //sin.sin_family = AF_INET;
    //sin.sin_port = htons(listen_port);
    //memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
    
    rc = bind(soc,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //perror("bind");
    while( rc < 0 && j<=MAX_TRY_CONN ){
        sleep(1);
        rc = bind(soc,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
        j++;
    }
 
    if ( rc < 0 ) {
        //printf("Error while binding socket\n");
        exit(rc);
    }  
    
}

void listenSocket(int soc){
    
    if(listen(soc,SOMAXCONN) == -1) {
        //printf("listen error\n");
        exit(-1);
    }  
    
}

int acceptConnection(int soc){
    int conn_port;
    struct sockaddr_in sock_client;
    int client_len = sizeof(sock_client);
    char *host[MY_MAXHOST+1];
    char *port[MY_MAXSERV+1];
    
    conn_port = accept(soc, (struct sockaddr *) &sock_client, &client_len);

    if (conn_port == -1) {
        //printf("accept call failed! \n");
        exit(-1);
    }
    
    return conn_port;
}

int connect_socket(char *hostname, const char *service){
    struct sockaddr_in serv_addr;
    struct hostent *hp;
    int rc,portno;
    int j=1;
    
    int sockfd = createSocket();
    portno = atoi(service);
    //printf("%d\n",portno);
    
    hp = gethostbyname(hostname);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr, (char *)&serv_addr.sin_addr.s_addr,hp->h_length);
    serv_addr.sin_port = htons(portno);
    
    rc = connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    
    while( rc<0 && j<=MAX_TRY_CONN ){
        sleep(1);
        rc = connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
        j++;
    }
    
    if ( rc < 0 ) {
        //printf("Error while connecting socket\n");
        exit(rc);
    }  
    
    return sockfd;
}




#endif /* SOCKET_H */

