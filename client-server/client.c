/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "23340" // the port client will be connecting to 

#define MAXDATASIZE 512 // max number of bytes we can get at once 
#define MAXATTRIBUTES 2
//Frame(message) contents of the SBCP protocol

<<<<<<< HEAD
struct SBCP{
uint16_t vrsn_type, frame_len;
=======
struct Attr{
uint16_t attrib_type,attrib_len;
>>>>>>> 6ea6440... Fine tuning needed
char payload[MAXDATASIZE];
};

struct SBCP{
uint16_t vrsn_type, frame_len;
struct Attr at[MAXATTRIBUTES];
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    //Checking specification of command line options
    if (argc != 4) {
        fprintf(stderr,"usage: client username server_ip server_port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //Obtaining address of server to connect
    if ((rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //Obtaining the IPv4/IPv6 addresses in text format
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    // Populating the SBCP message frame
    // Initailizing with username
<<<<<<< HEAD
    struct SBCP msg;
    uint16_t attrib_len,attrib_type;
    msg.vrsn_type = (3<<7)|(2);
    attrib_type = 2;
    memcpy(msg.payload, &attrib_type, sizeof(attrib_type));
    //printf("\n%c %x %x\n",msg.payload[0],&msg.payload[1],&attrib_type); 
    attrib_len = strlen(argv[1])+4;
    msg.frame_len = attrib_len + 4; 
    memcpy(&msg.payload[2], &attrib_len, sizeof(attrib_len));
    memcpy(&msg.payload[4], argv[1], strlen(argv[1]));
    memset(msg.payload, '\0', sizeof(msg.payload));

    msg.vrsn_type = htons (msg.vrsn_type);
=======
    struct SBCP msg; 
    msg.vrsn_type = (3<<7)|(2);
    msg.at[0].attrib_type = 2;
    memset(msg.at[0].payload, '\0', sizeof(msg.at[0].payload));
    strcpy(msg.at[0].payload,argv[1]); //username initially to join
    msg.at[0].attrib_len = strlen(msg.at[0].payload)+4;
    msg.frame_len = msg.at[0].attrib_len + 4; 

    msg.vrsn_type = htons (msg.vrsn_type);
    msg.at[0].attrib_type = htons (msg.at[0].attrib_type);
    msg.at[0].attrib_len = htons (msg.at[0].attrib_len);
>>>>>>> 6ea6440... Fine tuning needed
    msg.frame_len = htons (msg.frame_len);
    
    printf("\nVRSN_type = %d %x\t frame_len = %d %x\t payload = %s %x\t\n",msg.vrsn_type,&msg, msg.frame_len, &msg.frame_len, msg.payload, &msg.payload);

<<<<<<< HEAD
    if (send(sockfd, (char*)&msg, ntohs(msg.frame_len), 0) == -1){
=======
    if (send(sockfd, (char *)&msg, ntohs(msg.frame_len), 0) == -1){
>>>>>>> 6ea6440... Fine tuning needed
        printf("Error sending\n");
        perror("send");
    }

    printf("client: JOIN, now SEND/RECV \n"); 
    // FD_SET tmp variable for select() 
    fd_set tmp;

    while(1)
    {

    FD_ZERO(&tmp);
    FD_SET(0,&tmp);
    FD_SET(sockfd,&tmp);

    if(select(sockfd+1,&tmp,NULL,NULL,NULL) == -1) {
	printf("Error with select \n");
	perror("select");
	exit(1);
    }
    
    if(FD_ISSET(0,&tmp))
    {
	printf("%s: ",argv[1]);
	fgets(buf,MAXDATASIZE,stdin);
	msg.vrsn_type = (3<<7) | 4;
<<<<<<< HEAD
        attrib_type = 4;
        memcpy(msg.payload, &attrib_type, sizeof(attrib_type));
        attrib_len = strlen(buf)-1+4;
        msg.frame_len = attrib_len + 4;
        memcpy(&msg.payload[2], &attrib_len, sizeof(attrib_len));
        memcpy(&msg.payload[4], buf, strlen(buf)-1);
        memset(msg.payload, '\0', sizeof(msg.payload));

        msg.vrsn_type = htons (msg.vrsn_type);
	msg.frame_len = htons (msg.frame_len);

	if (send(sockfd, (char*)&msg, ntohs(msg.frame_len), 0) == -1){
=======
	msg.at[0].attrib_type = 4;
	memset(msg.at[0].payload, '\0', sizeof(msg.at[0].payload));
	strcpy(msg.at[0].payload,buf); 
	msg.at[0].attrib_len = strlen(msg.at[0].payload)-1+4;
	msg.frame_len = msg.at[0].attrib_len + 4;
	
	msg.vrsn_type = htons (msg.vrsn_type);
        msg.at[0].attrib_type = htons (msg.at[0].attrib_type);
        msg.at[0].attrib_len = htons (msg.at[0].attrib_len);
        msg.frame_len = htons (msg.frame_len);

	if (send(sockfd, (char *)&msg, ntohs(msg.frame_len), 0) == -1){
>>>>>>> 6ea6440... Fine tuning needed
        printf("Error sending\n");
        perror("send");
    	}

    }

    if(FD_ISSET(sockfd,&tmp))
    {
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
            perror("recv");
     	       exit(1);
	}
	buf[numbytes] = '\0';
	struct SBCP *recv_msg=(struct SBCP*) &buf;
	printf("NUMBYTES: %d SAMPLE : %d %d\n",numbytes,ntohs(recv_msg->at[1].attrib_type),ntohs(recv_msg->at[1].attrib_len));
	//FWD msg
<<<<<<< HEAD
//	if(((ntohs(recv_msg->vrsn_type))&0x7F) == 3) { 
//		if((ntohs(recv_msg->)) == 2)
//			printf("%s: ",recv_msg->at->payload);
//		else
			printf("%s\n",recv_msg->payload);
			//printf("VRSN:%d\tFRAMELEN:%d\tATTRIBTYPE:%d\tATTRLEN:%d\tPAYLOAD:%s\n",recv_msg->vrsn_type, recv_msg->frame_len, recv_msg->attrib_type, recv_msg->attrib_len, recv_msg->payload);
//	}
=======
	if(((ntohs(recv_msg->vrsn_type))&0x7F) == 3) { 
		printf("%s: %s\n",recv_msg->at[0].payload,recv_msg->at[1].payload);
	}
>>>>>>> 6ea6440... Fine tuning needed
    }

    }

    close(sockfd);

    return 0;
}
