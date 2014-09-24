/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 512

#define MAXATTRIBUTES 2
//Frame(message) contents of the SBCP protocol

struct Attr{
uint16_t attrib_type,attrib_len;
char *payload;
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
    int sockfd, sockmax, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv,i,j,k;
    int numbytes;
    char present,buf[MAXDATASIZE];
    char *usrns[atoi(argv[3])];
 
    for(i=0; i< atoi(argv[3]); i++)
	usrns[i] = NULL;

    //Checking specification of command line options
    if (argc != 4) {
        fprintf(stderr,"usage: server server_ip server_port max_clients\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // FD_SET variables for select() 
    fd_set master,read_fds;

    // clear the master and temp sets
    FD_ZERO(&master);    
    FD_ZERO(&read_fds);

    // add the socket descriptor to the master set
    FD_SET(sockfd, &master);

    // keep track of the largest socket descriptor to use with select()
    sockmax = sockfd; // so far, it's this one

    printf("server: waiting for connections...\n");

    while(1) 
    {  // main accept() loop
	read_fds = master;
	if(select(sockmax+1,&read_fds,NULL,NULL,NULL) == -1) {
	        printf("Error with select \n");
        	perror("select");
	        exit(1);
    	}

	//Looping through all incoming socket connections 
	for(i=0; i<=sockmax; i++) {
		if(FD_ISSET(i, &read_fds)) {
			if(i == sockfd) {
			//Accept the new connection
		        sin_size = sizeof their_addr;
		        new_fd =accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if(new_fd == -1) {
			        perror("accept");
			        continue;
        		}
			else {
				printf("server: Adding to master %d\n",new_fd);
				FD_SET(new_fd, &master); //Adding to master set
				if(new_fd > sockmax) {
					sockmax = new_fd;
				}

		        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		        printf("server: %d got connection from %s\n", sockfd,s);	
			}
           		}
			
			else {
                    	// handle data from a client
                    	if ((numbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                                printf("server: %s LEFT the chat room \n",usrns[i-sockfd-1]);
                        	if (numbytes == 0) {
	                        // connection closed
                        	printf("server: socket %d hung up! Nothing received\n", i);
                        	} 
				else {
				printf("server: some error receiving \n");
                            	perror("recv");
                        	}
				free(usrns[i-sockfd-1]);
				usrns[i-sockfd-1]=NULL;
                        	close(i); // bye!
                        	FD_CLR(i, &master); // remove from master set
                    	} 
			else {
                        // we got some data from a client
			buf[numbytes] = '\0';
			uint16_t *vrs_ty = (uint16_t *)&buf;
			//printf(" %d %x %x %x\n", ntohs(msg_buff->at[0].attrib_type), &msg_buff->at[0].attrib_len, &msg_buff->at[0].payload, &msg_buff->at[1].attrib_type);
			if(((ntohs(*vrs_ty))&0x7F) == 2)
			{
				if(usrns[atoi(argv[3])-1]!=NULL) {
                                printf("server: MAX CLIENTS of %d reached. Sorry Try again later!\n",atoi(argv[3]));
                                close(i); // bye!
                                FD_CLR(i, &master); // remove from master set
	                   	}
                        	else {
				present = 0;
                                for(j=0 ; j < atoi(argv[3]) ;j++) {
                                        if(usrns[j]!=NULL && strncmp(&buf[8],usrns[j],numbytes-8)==0) {
                                                printf("server: USERNAME(%s) already present!. Try again\n",usrns[j]);
                                                close(i); // bye!
		                                FD_CLR(i, &master); // remove from master set
						present=1;
						break;
                		        }
				}
				if(!present) {				
				usrns[i-sockfd-1]=(char*) malloc(numbytes-8);
				strncpy(usrns[i-sockfd-1],&buf[8],numbytes-8);
				printf("server: %s JOINED the chat room\n",usrns[i-sockfd-1]);	
				}
				}
			}	
			else {
				printf("%s: %s \n",usrns[i-sockfd-1],&buf[8]);

/*                        for(j = 0; j <= sockmax; j++) {
                            	// send to everyone!
                        	if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if ((j > sockfd) && (j != i)) {
				    struct SBCP msg;
				    msg.vrsn_type = (3<<7)|(3);
				    msg.at[0].attrib_type = 2;
				    msg.at[0].payload = malloc(strlen(usrns[i-sockfd-1]));
				    strcpy(msg.at[0].payload,usrns[i-sockfd-1]); //username initially to join
				    msg.at[0].attrib_len = strlen(msg.at[0].payload)+4;
				    msg.frame_len = msg.at[0].attrib_len + 4;

				    msg.at[1].attrib_type = 4;
				    msg.at[1].payload = malloc(strlen(msg_buff->at[0].payload));
				    strcpy(msg.at[1].payload,msg_buff->at[0].payload);	
				    msg.at[1].attrib_len = strlen(msg.at[1].payload)+4;
				    msg.frame_len += msg.at[1].attrib_len ;
				
				    //memcpy(msg.at[0].payload, (char *)&msg.at[1], msg.at[1].attrib_len);

				    msg.vrsn_type = htons (msg.vrsn_type);
				    msg.frame_len = htons (msg.frame_len);

				    for(k=0;k<MAXATTRIBUTES;k++) {
                                    msg.at[k].attrib_type = htons (msg.at[k].attrib_type);
                                    msg.at[k].attrib_len = htons (msg.at[k].attrib_len);
				    }
				    
				    printf("%x %x %x %s \n",&msg.at[0].payload,&msg.at[1].attrib_type,&msg.at[1].attrib_len,msg.at[1].payload);
				    //Sending Chat text and username (FWD)
				    if (send(j, (char *)&msg, sizeof(msg), 0) == -1){
				    printf("Error sending\n");
				    perror("send");
			    	    }
    				}
				}
			}
*/			
			}
			}
			}
		}
	}
    }	
    return 0;
}
