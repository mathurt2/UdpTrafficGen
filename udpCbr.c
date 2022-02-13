/* **********************************************************************/
/* Developer: Tushar Mathur
* Email: mathurt2@miamioh.edu
* This code opens UDP socket, forms the payload, and sends packet to the DIP.
* There is a linked C++ file that drives these functions.
* The code has been tested on OSX 10.14.6
*
*  ------------------------------------------------------------------------
*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>


int createSock () {
  int sockfd;
   /* Create UDP socket */
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
   perror ("socket creation failure");
   exit (EXIT_FAILURE);
  }
return sockfd;
}
     /* Packet data */
void udp_packet_gen(unsigned char* sendbuf, int payload) {
   for (int l = 0; l < (payload); l++) {
    sendbuf[l] = 0x54; 
   } 
}
    /* Send packets */
void sendPkt(int sockfd, unsigned char* sendbuf, int pktsize, struct sockaddr_in serverAddr) {
  if (sendto (sockfd, sendbuf, pktsize, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 ) {
     fprintf(stderr, "Value of errno: %d\n", errno);
     perror("Error printed by perror");
     printf("\n");
     }
}
