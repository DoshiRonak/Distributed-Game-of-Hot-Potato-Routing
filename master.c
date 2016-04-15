/******************************************************************************
 *
 *  File Name........: master.c
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




master_core *master_engine = NULL;

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
    //struct sockaddr_in sin;
    //struct hostent *hp;
    int rc;
    
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
    int client_len = sizeof(sock_client);
    char *host[MY_MAXHOST+1];
    char *port[MY_MAXSERV+1];
    
    conn_port = accept(soc, (struct sockaddr *) &sock_client, &client_len);

    if (conn_port == -1) {
        printf("accept call failed! \n");
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


neighbor_msg *create_neighbor_msg(int left, int right){
    
    neighbor_msg* m;
    
    m = (neighbor_msg*)malloc( sizeof(neighbor_msg) );
    
    strcpy(m->left_host,master_engine->player_list[left].host);
    strcpy(m->right_host,master_engine->player_list[right].host);
    strcpy(m->left_port,master_engine->player_list[left].port);
    strcpy(m->right_port,master_engine->player_list[right].port);
    
    m->left_index=left;
    m->right_index=right;
    m->exit=0;
    
    return m;    
}

void close_game(){
    int i;
    int send_socket;
    potato_msg * msg;

    msg = (potato_msg*)malloc(sizeof(potato_msg));

    for(i=0; i<master_engine->player_number; i++){
        
        send_socket = connect_socket( master_engine->player_list[i].host, master_engine->player_list[i].port);
        
        if(send_socket<0){
            continue;
        }
        
        msg->sender=-1;
        msg->type=EXIT;
        //msg->hops=master_engine->hops;
        msg->hops=0;
        msg->sent_times=0;
        msg->receiver=i;
        //printf("sending message\n");
        send(send_socket, msg, sizeof(char)*(sizeof(potato_msg)), 0);
        
        close(send_socket);
        //free(msg);
    } /* end of for */

    int j = close(master_engine->lis_socket);
    //printf("\n%d\n",j);
}

int createPotato(){
    int p_index;
    potato_msg * msg=NULL;
    int connfd;     /* socket to accept player's report */
    socklen_t addrlen;
    struct sockaddr_storage clientaddr;
    int p_socket;   /* socket to initially send the potato */
    int i, j;
    
    msg = (potato_msg*)malloc( sizeof(potato_msg));
    
    do{
	int value = rand();
	p_index = (value % master_engine->player_number);
    }
    while(p_index < 0 || p_index >= master_engine->player_number);
    //p_index = 0;
    //printf("Initial potato to %d\n",p_index);
    msg->sender=-1;
    msg->type=SEND_P;
    msg->hops=master_engine->hops;
    msg->sent_times=0;
    msg->receiver=p_index;
 
    p_socket = connect_socket(master_engine->player_list[p_index].host, master_engine->player_list[p_index].port);
    /*
    i=1;
    while( p_socket<0 && i<=MAX_TRY_CONN ){
        sleep(1);
        p_socket = connect_socket(master_engine->player_list[p_index].host, master_engine->player_list[p_index].port);
        i++;
    }
    */
    
    if(p_socket < 0){
        //printf("Error connecting the first player\n");
        exit(1);
    }
    
    int n = send(p_socket, msg, sizeof(char)*(sizeof(potato_msg)), 0);
    //printf("potato sent - %d\n",n);
    free(msg);
    close(p_socket);
    
    return p_index;
    
}

void waitforPotato(){
    int connfd,j;
    char *buf;
    potato_msg *msg;
    
    struct sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    
    msg = (potato_msg*)malloc(sizeof(potato_msg));
    
    while(1){
        //connfd = acceptConnection(master_engine->lis_socket);
        connfd = accept(master_engine->lis_socket, (struct sockaddr *) &clientaddr, &addrlen);
        //printf("Received potato\n");
        if(connfd < 0){
            //printf("Invalid connection\n");
            continue;
        }
        
        recv(connfd, msg, sizeof(char)*(sizeof(potato_msg)), 0);
        //msg = buf;
        int recv_l;
	int to_recv_l;
	int to_recv_block;
	int sent_l;
	int to_send_l;
	int to_send_block;
        
        if(msg->type = REPORT_P){
            int size_recv , total_size= 0;
            int *track,*old_track;
            track = (int*)malloc( sizeof(int)*msg->sent_times + 1 );

            to_recv_l = sizeof(int) * (msg->sent_times);
                
                if(to_recv_l>CHUNK_SIZE){
                    to_recv_block = CHUNK_SIZE;
                    old_track = track;
                    //printf("Receiving large data\n");

                    while(to_recv_l>0){
                        recv_l = recv(connfd, old_track, to_recv_block, 0);
                        old_track+=( recv_l / (sizeof(int)) );
                        to_recv_l-=recv_l;
                        if( to_recv_l < CHUNK_SIZE)
                            to_recv_block = to_recv_l;
                    }
                }else{
                    recv(connfd, track, sizeof(int) * (msg->sent_times)+1, 0);
		}
            
            printf("Trace of potato:\n");
            
            for(j=0; j < msg->sent_times; j++){
                printf("%d", *(track+j) );
                if( j < (msg->sent_times-1) )
                    printf(",");
            }
            
            close_game(); /* shutdown the ring */

            free(track);
            break;           
        }
        free(msg);
	close(connfd);        
    }
}


void send_neighbors_info(){
    int i;
    int connfd;
    int left_neighbor, right_neighbor;

    neighbor_msg * nei_msg = NULL;
    char * buf=NULL;
    int struct_size;
    int j=0;
    
    //printf("sending neigh info\n");

    /*
    struct_size = sizeof(neighbor_msg);
    buf = (char*)malloc( sizeof(char)*(struct_size+1) );
    if(buf==NULL){
        exit(0);
    }
*/
    for(i=0; i<master_engine->player_number; i++){
        /* connect to a socket */
     
        connfd = connect_socket(master_engine->player_list[i].host, master_engine->player_list[i].port);
      

        if(connfd < 0){
            /* !!!!! should do some cleanup work !!!!! */
            //printf("Ending game\n");
            close_game();
            exit(0);
        }
        
        //printf("Connected to player %d\n",i);

        /* if the this player is in a "middle" position */
        if( i>0 && i< master_engine->player_number-1 ){
            left_neighbor = i-1;
            right_neighbor = i+1;
        }else{
            if( i==0 ){
                left_neighbor = master_engine->player_number-1;
                right_neighbor = i+1;
            }else if( i==master_engine->player_number-1 ){
                left_neighbor = i-1;
                right_neighbor = 0;
            }else{
                //printf("un-handled case\n");
            }
        } /* end of, calculating left/right neighbor */

        nei_msg = create_neighbor_msg(left_neighbor,right_neighbor);
        


        //memset(buf, 0, sizeof(char)*(struct_size+1) );
        //memcpy(buf, nei_msg, sizeof(char)*struct_size );
        //send(connfd, buf, sizeof(char)*struct_size, 0);
        
        send(connfd, nei_msg, sizeof(char)*sizeof(neighbor_msg), 0);

        close(connfd);
//        free(nei_msg);
    } /* end of for */
    //free(buf);

} /* end of sends_neighbors_info() */

void init_master(){
    int sock,connfd;
    int i=0;
    struct sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    
    sock = createSocket();
    //master_engine->port1 = bindSocket(sock,master_engine->port1,master.ip_addr);
    bindSocket(sock,master_engine->port1);
    listenSocket(sock);
    
    master_engine->lis_socket = sock;
    
    while(i < master_engine->player_number){
        //connfd = acceptConnection(sock);
        
        connfd = accept(sock, (struct sockaddr *) &clientaddr, &addrlen);
        
        if(connfd < 0){
            //printf("Invalid connection\n");
            i++;
            continue;
        }
        
        getnameinfo((struct sockaddr *) &clientaddr, addrlen, master_engine->player_list[i].host,
		sizeof(char)*MY_MAXHOST, master_engine->player_list[i].port, sizeof(char)*MY_MAXSERV, NI_NUMERICSERV);


        memset( master_engine->player_list[i].port, 0, sizeof(char)*(MY_MAXSERV+1) );
        sprintf(master_engine->player_list[i].port, "%d", ( master_engine->port1 + 1 + i) );

        send(connfd, &i, sizeof(int), 0);

        /* print out required player's info */
        printf("player %i is on %s\n", i, master_engine->player_list[i].host);

        i++;

        close(connfd);
        
    }	

}

int main (int argc, char *argv[]){

  /* read port number from command line */
    if(argc!=4){
        exit(0);
    }

    srand( time(NULL) );
    
    master_engine = (master_core*) malloc( sizeof(master_core) );
    if(master_engine==NULL){
        exit(0);
    }
    //memset(master_engine->host, 0, sizeof(char)*(MY_MAXHOST+1) );/**/
    //memset(master_engine->port, 0, sizeof(char)*(MY_MAXSERV+1) );

    sprintf(master_engine->port, "%s", argv[1]);
    master_engine->player_number = atol(argv[2]);
    
    //gethostname( master_engine->host, sizeof(char)*MY_MAXHOST );
    //master_engine->player_list = NULL;
   // master_engine->lis_socket = -1;
    master_engine->port1 = atoi( argv[1] );

    if( master_engine->player_number < 2){
        exit(0);
    }
    
    master_engine->hops = atol(argv[3]);
    if(master_engine->hops<0 ){
        exit(0);
    }
    /* init all player's info */
    master_engine->player_list = (player_info*)malloc( master_engine->player_number * sizeof(player_info) );
    
    if( master_engine->player_list==NULL ){
        exit(0);
    }
    int i;

    for(i = 0;i < master_engine->player_number;i++){
        master_engine->player_list[i].id = -1;
        //memset( master_engine->player_list[i].host, 0, sizeof(char)*(MY_MAXHOST+1) );
	//memset( master_engine->player_list[i].port, 0, sizeof(char)*(MY_MAXSERV+1) );
       // strcpy(master_engine->player_list[i].host,'');
        //strcpy(master_engine->player_list[i].port,'');
    }  
    //strcpy(master_engine->host,'');
    
    gethostname( master_engine->host, sizeof(char)*MY_MAXHOST );
    
    printf("Potato Master on %s\n", master_engine->host);
    printf("Players = %d\n", master_engine->player_number);
    printf("Hops = %d\n", master_engine->hops);
    
    init_master();
    send_neighbors_info();
    
    if(master_engine->hops != 0){
        int firstPlayer = createPotato();
        printf("All players present, sending potato to player %d\n", firstPlayer);
        waitforPotato();
    }else{
//        printf("aa\n ");
        //sleep(1);
        close_game();
    }
    
    close(master_engine->lis_socket);
    //printf("closed master\n");
    return (EXIT_SUCCESS);
}

/*........................ end of master.c ..................................*/
