/******************************************************************************
 *
 *  File Name........: player.c
 *
 *
 *****************************************************************************/

/*........................ Include Files ....................................*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "socket.h"
#include <errno.h>

#define LEN	64

player_core * player_engine;

void sendall(int fd, void *buf, int len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;

    while(total < len) 
    {
      n = send(fd, (buf+total), bytesleft, 0);
      //if (n == -1) { break; }
      if (n < 0 || n == 0) { break; }
      total += n;
      bytesleft = bytesleft - n;
    }

    len = total; // return number actually sent here

    //return n==-1?-1:0; // return -1 on failure, 0 on success
    //return (n);
}  
/*
int createSocket(){
    int soc;
    
    soc= socket(AF_INET, SOCK_STREAM, 0);
    
    if (soc == -1){
        printf("error in socket creation\n");
        exit(-1);
    }
    
    return soc;
}

void bindSocket(int soc, int listen_port){
    struct sockaddr_in sin;
    struct hostent *hp;
    int rc;
    
    hp = gethostbyname(player_engine->host);
    
    sin.sin_family = AF_INET;
    sin.sin_port = htons(listen_port);
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
    
    rc = bind(soc,(struct sockaddr *)&sin, sizeof(sin));
    if ( rc < 0 ) {
        printf("Error while binding socket\n");
        exit(rc);
    }  
    
}

void listenSocket(int soc){
    
    if(listen(soc,SOMAXCONN) == -1) {
        printf("listen error\n");
        exit(-1);
    }  
    
}

int acceptConnection(int soc){
    int conn_port;
    struct sockaddr_in sock_client;
    int slen = sizeof(sock_client);
    
    conn_port = accept(soc, (struct sockaddr *) &sock_client, &slen);

    if (conn_port == -1) {
        printf("accept call failed! \n");
        exit(-1);
    }
    return conn_port;
}
*/
/*
int connect_socket(char *hostname, const char *service){
    struct sockaddr_in sin;
    struct hostent *hp;
    int rc;
    
    int sockfd = createSocket();
    
    
    hp = gethostbyname(hostname);
    printf("%s\n",hp->h_addr_list[0]);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(service);
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
    
    rc = connect(sockfd,(struct sockaddr *)&sin, sizeof(sin));
    if ( rc < 0 ) {
        printf("Error while connecting socket\n");
        exit(rc);
    }  
    
    return sockfd;
}
*/
/*
int connect_socket(char *hostname, const char *service){
    struct sockaddr_in serv_addr;
    struct hostent *hp;
    int rc,portno;
    int j = 1;
    
    int sockfd = createSocket();
    portno = atoi(service);
    
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
        printf("Error while connecting socket\n");
        exit(rc);
    }  
    
    return sockfd;
}
*/
int receiveall(int fd, void *buf, int len)
{
	int total = 0;
	int bytesleft = len;
	int n;
	
	while (total < len)
	{
		n = recv(fd, (buf+total), bytesleft, 0);		//receive accept/connect order
	  //if ( n < 0 ) {break;}
	  if (n < 0 || n == 0) { break; }
	  total = total + n;
	  bytesleft = bytesleft - n;
  }
  len = total;
  //return n == -1? -1:0;
  return (n);
}

void listen_msg(){
	//socklen_t addrlen;
	int addrlen;
	struct sockaddr_storage clientaddr;
	addrlen = sizeof(clientaddr);
	int connfd;
	potato_msg * msg=NULL;
	int select_neighbor;
	int * old_track=NULL;
	int * new_track=NULL;
	int nei_socket;
	int * t_track;

	/* variables to handle large track size */
	int recv_l;
	int to_recv_l;
	int to_recv_block;
	int sent_l;
	int to_send_l;
	int to_send_block;
        
        msg = (potato_msg*)malloc(sizeof(potato_msg));
        //old_track = (int*) malloc( sizeof(int) * 10000);
        //new_track = (int*) malloc( sizeof(int) * 10000);

	while(1){
            //printf("Listening for msg\n");
            connfd = accept(player_engine->lis_socket, (struct sockaddr *) &clientaddr, &addrlen);
            //printf("Connection accepted\n");
	    //perror("accept");
	    //printf("%d\n",errno);
            if (connfd < 0){
                //printf("Connection failure -%d-\n",connfd);
                continue;
            }
            recv(connfd, msg, sizeof(char)*sizeof(potato_msg), 0);
            //printf("Msg reveived by me\n");
            if(msg==NULL)
                    continue;
            
            /* if this player receives a potato */
            if(msg->type==SEND_P){

                    msg->sent_times++;
                    //printf("Times - %d\n",msg->sent_times);
                    msg->hops--;

                    /* will receive/create the track, and send it to a neighbor/master */
                    if (msg->sender != -1) { 
                //        printf("alloc old_track %d\n",( sizeof(int) * (msg->sent_times - 1)+1));
                            old_track = (int*) malloc( sizeof(int) * (msg->sent_times - 1)+1);
                  //          printf("old_track allocated\n");
                            //memset(old_track, 0, sizeof(int) * (msg->sent_times - 1) +1 );

                            /* !!!!  more work needed to handle large track */
                            to_recv_l = sizeof(int) * (msg->sent_times - 1);
                            if(to_recv_l>CHUNK_SIZE){
                    //            printf("Large size\n");
                                    to_recv_block = CHUNK_SIZE;
                                    t_track = old_track;

                                    while(to_recv_l>0){
                                            recv_l = recv(connfd, t_track, to_recv_block, 0);
                                            t_track+=( recv_l / (sizeof(int)) );
                                            to_recv_l-=recv_l;
                                            if( to_recv_l<CHUNK_SIZE)
                                                    to_recv_block = to_recv_l;
                                    }
                            }else{
                                    recv(connfd, old_track, sizeof(int) * (msg->sent_times - 1), 0);
                                    
                                    //printf("Track received\n");
                            }

                            /* copy to new track */
                            new_track = (int*) malloc( sizeof(int) * (msg->sent_times) + 1 );
                           
                            //memset( new_track, 0, sizeof(int) * (msg->sent_times) + 1 );
                            memcpy(new_track, old_track, sizeof(int) * (msg->sent_times - 1) );
                            //memcpy(new_track, old_track, sizeof(old_track) );
                            //strcpy(new_track, old_track);
                            *(new_track+msg->sent_times - 1) = player_engine->index;  /* append this player's index */
                            //free(old_track);
                            //printf("%d\n",*new_track);
                            //printf("%d\n",*(new_track+1));
                    } else { 
                        //printf("Potato from master\n");
                        new_track = (int*) malloc(sizeof(int) + 1);
                        *(new_track) = player_engine->index;
                        //printf("Potato from master1\n");
                    }
                    //printf("Track updated\n");
                    msg->sender=player_engine->index;
                    /* send msg/track to one of its neighbors/master */
                    if (msg->hops != 0) {
                            int value = rand();
                            select_neighbor = (value % 2);

                            if(select_neighbor == 0){

                                nei_socket = connect_socket(player_engine->left_host,player_engine->left_port);
                                msg->receiver = player_engine->left_index;
                            }else{
                                nei_socket = connect_socket(player_engine->right_host,player_engine->right_port);
                                msg->receiver = player_engine->right_index;
                            }

                            msg->type = SEND_P;

                            printf("Sending potato to %d\n", msg->receiver);

                    } else {
                            nei_socket = connect_socket(player_engine->master_host,player_engine->master_port);
                            msg->type = REPORT_P;

                            printf("I'm it\n");
                    }

                    send(nei_socket, msg, sizeof(char)*(sizeof(potato_msg)), 0);

                    /* !!!!  more work needed to handle large track */
                    to_send_l = sizeof(int) * (msg->sent_times);
                    if(to_send_l>CHUNK_SIZE){
                            to_send_block = CHUNK_SIZE;
                            t_track = new_track;

                            while( to_send_l>0 ){
                                    /* printf("---->test2, to_send_block:%d, to_send_l:%d\n", to_send_block, to_send_l); */
                                    sent_l = send(nei_socket, t_track, to_send_block, 0);
                                    /* printf("---->test3, sent_l:%d\n", sent_l); */
                                    t_track+=( sent_l / (sizeof(int)) );
                                    /* printf("---->test4, sent_l / (sizeof(int)):%d\n", sent_l / (sizeof(int)) ); */
                                    to_send_l-=sent_l;
                                    if( to_send_l<CHUNK_SIZE)
                                            to_send_block = to_send_l;
                            }

                    }else{
                        send(nei_socket, new_track, (sizeof(int) * (msg->sent_times)), 0); /* send new track */
                    }

                    close(nei_socket);
                    /* end if SEND_P */
		}else if(msg->type==EXIT){  /* if master requires to shutdown */
			close(connfd);
			close(player_engine->lis_socket);
			exit(0);
		}
                //free(old_track);
                //old_track = NULL;
		close(connfd);
		//free(msg);
                //free(new_track);
	}/* end while*/

}  /* end of listen_msg */


void get_neighbors_info(){
    neighbor_msg * nei_msg = NULL;
    nei_msg = (neighbor_msg*)malloc(sizeof(neighbor_msg));
    int lis_port;
    int sock,connfd;
    int i=0;
    struct sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    //printf("Ready to receive info\n");
    sock = createSocket();
    //master_engine->port1 = bindSocket(sock,master_engine->port1,master.ip_addr);
    lis_port = atoi(player_engine->port);
    //printf("%d\n",lis_port);
    bindSocket(sock,lis_port);
    //bindSocket(sock,player_engine->port);
    listenSocket(sock);
    
    player_engine->lis_socket = sock;
    
    //connfd = acceptConnection(sock);
    connfd = accept(sock, (struct sockaddr *) &clientaddr, &addrlen);
        
    if(connfd < 0){
        //printf("Invalid connection\n");
        exit(0);
    }

    /*
    struct_size = sizeof(neighbor_msg);
    buf = (char*)malloc( sizeof(char)*(struct_size+1) );
    if(buf==NULL){
        exit(0);
    }
*/
    //printf("Master connected to me\n");
    recv(connfd, nei_msg, sizeof(char)*sizeof(neighbor_msg), 0);
    
    strcpy(player_engine->left_host, nei_msg->left_host);
    strcpy(player_engine->left_port, nei_msg->left_port);
    strcpy(player_engine->right_host, nei_msg->right_host);
    strcpy(player_engine->right_port, nei_msg->right_port);
    player_engine->left_index = nei_msg->left_index;
    player_engine->right_index = nei_msg->right_index;
    
    //printf("Left player - %d\n",player_engine->left_index);
    //printf("Right player - %d\n",player_engine->right_index);
    
    close(connfd);
    free(nei_msg);
} /* end of get_neighbors_info() */

void init_player(){
    int sock,connfd;
    int i=0;
    struct sockaddr_in my_sock_addr;
    socklen_t my_sock_len;
    my_sock_len = sizeof(my_sock_addr);
    int index;
    //int sockfd = createSocket();
    connfd = connect_socket(player_engine->master_host, player_engine->master_port);
    

    if(connfd < 0){
        //printf("cannot connect to Master\n");
        exit(0);
    }
        
    getsockname(connfd, &my_sock_addr, &my_sock_len);
    /* converts an address into a host name and a service name */
    getnameinfo((struct sockaddr *) &my_sock_addr, my_sock_len,
                    player_engine->host, sizeof(char)*MY_MAXHOST, player_engine->port,
                    sizeof(char)*MY_MAXSERV, NI_NUMERICHOST);  
    
    recv(connfd, &index, sizeof(int), 0 );
    player_engine->index = index;
    sprintf(player_engine->port,"%d",(player_engine->base_port + 1 + player_engine->index));
    //player_engine->lis_port = atoi(player_engine->port);
    printf("Connected as player %d\n", player_engine->index);
    close(connfd);
    
}


int main (int argc, char *argv[]){

  /* read port number from command line */
    if(argc != 3){
      //printf("Number of arguments passed are less\n");
      exit(1);
    }
    
    srand( time(NULL) );
  
    player_engine = (player_core*)malloc(sizeof(player_core));

    if(player_engine == NULL){
      exit(0);
    }
    
    /*
    player_engine->index=-1;
    player_engine->left_index=-1;
    player_engine->right_index=-1;
    player_engine->lis_socket=-1;
    memset( player_engine->host, 0, sizeof(char)*(MY_MAXHOST+1) );
    memset( player_engine->port, 0, sizeof(char)*(MY_MAXSERV+1) );
    memset( player_engine->master_host, 0, sizeof(char)*(MY_MAXHOST+1) );
    memset( player_engine->master_port, 0, sizeof(char)*(MY_MAXSERV+1) );
    memset( player_engine->left_host, 0, sizeof(char)*(MY_MAXHOST+1) );
    memset( player_engine->left_port, 0, sizeof(char)*(MY_MAXSERV+1) );
    memset( player_engine->right_host, 0, sizeof(char)*(MY_MAXHOST+1) );
    memset( player_engine->right_port, 0, sizeof(char)*(MY_MAXSERV+1) );

    sprintf(player_engine->master_host, "%s", argv[1]);
    sprintf(player_engine->master_port, "%s", argv[2]);
   */
    
    strcpy(player_engine->master_host, argv[1]);
    strcpy(player_engine->master_port, argv[2]);

    player_engine->base_port= atoi(argv[2]);
    
    init_player();
    get_neighbors_info();
    
    listen_msg();
    
    return (EXIT_SUCCESS);
}

/*........................ end of speak.c ...................................*/

