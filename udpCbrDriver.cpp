/* *****************************************************************************************/
/* Developer: Tushar Mathur
* Email: mathurt2@miamioh.edu
* This code is the driver to generate UDP packets from an ethernet port.
* There is a linked C file that actually transmits the packets from the eth interface
* The code's tested on Ubuntu 18.04.3 LTS (GNU/Linux 5.0.0-37-generic x86_64) and OSX 10.14.6
* -------------------------------------------------------------------------------------------
*********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <map>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <ctime>
#include <unistd.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

using namespace std::chrono;

extern "C" {
	int createSock ();
	void udp_packet_gen(unsigned char* sendbuf, int pktsize);
        void sendPkt(int sockfd, unsigned char* sendbuf, int size, struct sockaddr_in serverAddr);
}

unsigned long long current_timestamp() { // current timestamp in microseconds
struct timeval tv;
gettimeofday(&tv, NULL); // get current time
unsigned long long curr_time = 1000000 * tv.tv_sec + tv.tv_usec;
return curr_time;
}

int main(int argc, const char* argv[]) {
	std::cout << std::endl;
	std::cout << "/****************************************************************************************************************************/" << std::endl;
	std::cout << "* This tool is a custom traffic generator that transmits UDP packets.  The tool requires 5 input parameters:  " << std::endl;
	std::cout << "* Duration Of Run (minimum 15 seconds), Destination IP (A.B.C.D), Average Bit Rate (minimum 10 Mbps), IP PDU Size ( > 1200 Bytes;" << std::endl;
	std::cout << "* max 1500 bytes), UDP Port Number (default = 9)." << std::endl;
	std::cout << "/***************************************************************************************************************************/" << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "* Begin Logging *" << std::endl << std::endl;
	const int def_argc = 6;
	const char *def_argv[] = {" ", "15", "1.1.1.1", "100", "1500", "9" };

	if (argc < 6) {
	   std::cout << std::endl << "Tool will run with default parameters!" << std::endl;
	   std::cout << "!!**Length Of Run = 15 seconds, DIP = 1.1.1.1, Average Bit Rate = 100 Mbps, IP PDU Size = 1500 Bytess, UDP Port Number = 9**!!" << std::endl;
	   argc = def_argc;
	   for (int i = 1; i < def_argc; i++)
			argv[i] = def_argv[i];
	}
	unsigned long nRuns = std::atol(argv[1]);
	if (nRuns < 15) {
			std::cout << "Invalid run time. Changing to 15 seconds!" << std::endl;
			nRuns = 15.0;
	}
	const char* DIP = argv[2];
	unsigned long long cbr = atol (argv[3]);
	if (cbr < 10) {
			std::cout << "Bit-rate too low! Re-run the tool with valid value!" << std::endl << std::endl;
			return 1;
	}
	
	int ip_pdu = atoi (argv[4]);  //1450 bytes max Payload
	if (ip_pdu < 1200 || ip_pdu > 1500) {
			std::cout << "Using default IP PDU size = 1500 Bytes because passed value is invalid! " << std::endl;
			ip_pdu = 1500;
	}
	uint16_t port = atoi (argv[5]);
	if (port == 0) {
			std::cout << "0 is an invalid port number, defaulting to 9!" << std::endl;
			port = 9;
	}
	unsigned long long ms_nRuns = nRuns * 1000ULL;
	milliseconds nRuns_ms = (milliseconds) (ms_nRuns);  // change seconds to milliseconds
	std::cout << std::endl << "Duration of run : " << nRuns_ms.count() << std::endl;
	milliseconds runStartTime = duration_cast< milliseconds > (system_clock::now().time_since_epoch());
	std::cout << "Run start time : " << runStartTime.count() << std::endl;
	milliseconds runEndTime = runStartTime + nRuns_ms;
	std::cout << "Run  end  time : " << runEndTime.count() << std::endl;
	

	int pktsize = ip_pdu + 18;  //IP PDU + ethernet header // 14 bytes ethernet + 4 bytes CRC  20 bytes IPv4 + 8 bytes UDP header
	int pktsize_payload = ip_pdu - 28;   //20 bytes IPv4 + 8 bytes UDP header
	/* socket id and packet buffer size initialized */	
	int sockfd = createSock();
	unsigned char sendbuf[pktsize_payload];
	memset(sendbuf, 0, pktsize_payload);
	udp_packet_gen(sendbuf, pktsize_payload);
	
    /* Configure settings in address struct */
	struct sockaddr_in serverAddr;
        
	memset(&serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    inet_pton(AF_INET, DIP, &serverAddr.sin_addr);
       
	/* Token bucket algorithm */	
	long long tokenBucket = 0;
	unsigned long long stored_time = current_timestamp();
	while (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() <= runEndTime.count()) {
		unsigned long long cT = current_timestamp();
		tokenBucket +=  (cbr) * (cT - stored_time);
		while (tokenBucket > 0) {
			sendPkt (sockfd, sendbuf, pktsize_payload, serverAddr);
			tokenBucket -= (pktsize * 8);
		}
		stored_time = cT;
		usleep(100); //smoothen the packet flow
	}	
	close(sockfd);
	std::cout << "Time at simulation end  : " << duration_cast< milliseconds > (system_clock::now().time_since_epoch()).count()  << std::endl;
	std::cout << "Original run  end  time : " << runEndTime.count() << std::endl;
	std::cout << "Original run start time : " << runStartTime.count() << std::endl;
	return 0;
}

