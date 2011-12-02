// ----------------------------- sender --------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define ETHERTYPE_LEN		2
#define MAC_ADDR_LEN		6
#define BUFFER_LEN 			1518
#define END_TAG_POSITION	30
#define FILL_CHARACTER      0xa0
#define LOCALPACKETSIZE		1024
#define MEASURETIME 		10
#define MAX_PACKET_SIZE 	1518
typedef unsigned char MacAddress[MAC_ADDR_LEN];
extern int errno;

int 		sockSend 	= 0;
int 		retValue = 0;
char 		buffer[BUFFER_LEN];
char    	dummyBuf[LOCALPACKETSIZE];
struct 		sockaddr_ll destAddr;
short int 	etherTypeT = 0;
MacAddress 	localMac	= {0x00, 0x08, 0xA1, 0x8E, 0xE4, 0x52};
MacAddress 	destMac		= {0x00, 0x17, 0x9A, 0xB3, 0x9E, 0x16};

// ----------------------------- receiver ------------------------------

int sockReceive;
int uid;
int packetsize = LOCALPACKETSIZE;

int sendno = 0;
int receiveno = 0;
struct sockaddr_ll sockinfo;
char packet[MAX_PACKET_SIZE];

pthread_t rec_thread;
int tparam = 1;
time_t start;


void prepare_receiver(void)
{
	uid = getuid(); // Are you root?
	if(uid != 0) { printf("You must have UID 0 instead of %d.\n",uid); exit(1); }
	if( (sockReceive = socket(AF_INET,SOCK_PACKET,htons(0x3))) == -1) { perror("socket"); exit(1); } 
}

void dump(char* name, char* addr, int size)
{
	int active = 0;
	if (!active) return;

	int k 	= 0 ;	
	unsigned char * cp = (unsigned char*) addr;

	printf("\ndump: %s\n",name);
	for (k=0;k<size;k++)
	{
		printf("%2X ",*cp);
		cp++;
		if ((k+1)%8 == 0)printf(" ");
		if ((k+1)%16 == 0)printf("  ");			
		if ((k+1)%32 == 0)printf("\n");		
	} 
	printf("\n");
}

void sendpacket(void)
{
	if((retValue = sendto(sockSend, buffer, LOCALPACKETSIZE, 0, 
	(struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) 
	{
		printf("ERROR! sendto() call failed (Error No: %d \"%s\").\n", errno, strerror(errno));
		exit(1);
	}
	sendno++;
}

void prepare_sender(void)
{
	etherTypeT = htons(0x8200);	
	memset(&destAddr, 0, sizeof(struct sockaddr_ll));
	memset(buffer, 0, BUFFER_LEN);
	
	if((sockSend = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) 	
	{
		printf("ERROR! socket() call failed (Error No: %d ).\n", errno);
		exit(1);
	}
	//printf("Socket creation success.\n");
	destAddr.sll_family = htons(PF_PACKET);
	destAddr.sll_protocol = htons(ETH_P_ALL);
	destAddr.sll_halen = 6;
	destAddr.sll_ifindex = 2;
	memcpy(&(destAddr.sll_addr), destMac, MAC_ADDR_LEN);

	/* Ethernet Header Construction */
	memcpy(buffer, localMac, MAC_ADDR_LEN);
	memcpy((buffer+MAC_ADDR_LEN), destMac, MAC_ADDR_LEN);
	memcpy((buffer+(2*MAC_ADDR_LEN)), &(etherTypeT), sizeof(etherTypeT));

	/* Add some data */
	memset(dummyBuf, FILL_CHARACTER, sizeof(dummyBuf));
	memcpy((buffer+ETHERTYPE_LEN+(2*MAC_ADDR_LEN)), dummyBuf, LOCALPACKETSIZE-14);		
}

void *f_recieve(void)
{
	int 	dbg = 0;
	int 	len = 0;	
	unsigned char  c1,c2,c3,c4,c5,c6,c7,c8;


	if(dbg) printf("\nthread running..\n");

	fd_set  readfds;
	int 	ret;
	struct	timeval timeout;

	FD_ZERO(&readfds);

	while(1)
	{
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		FD_SET( sockReceive, &readfds );
		ret = select( 5, &readfds, NULL, NULL, &timeout );
	
		if (ret == 0) {
			pthread_exit(0); //
			//printf("\nTimeout, no packet received.\n");
			//exit(1);
		} 
		else if (ret == -1) {
			printf("\nError processing.\n");
		} 
		else 
		{	
			if (FD_ISSET(sockReceive, &readfds)) 
			{
				// criteria for local packets
				c1 = packet[17]; c2 = packet[18]; 
				c3 = packet[19]; c4 = packet[20];
				c5 = packet[21]; c6 = packet[22]; 
				c7 = packet[23]; c8 = packet[24];				
				len = read(sockReceive,packet,LOCALPACKETSIZE);
				if (
					c1 == FILL_CHARACTER && c2 == FILL_CHARACTER && 
					c3 == FILL_CHARACTER && c4 == FILL_CHARACTER &&
					c5 == FILL_CHARACTER && c6 == FILL_CHARACTER && 
					c7 == FILL_CHARACTER && c8 == FILL_CHARACTER 
					)
				{
					// valid packet
					receiveno++;
				}
			}
		}			
	}
}


	
int ethernet_test(void)
{
	float    a,b;	
	prepare_sender();
	prepare_receiver();
	// start receiver therad

	sendno= 0;
	receiveno = 0;

	pthread_create(&rec_thread,NULL,(void *)f_recieve,(void*)&tparam);
	sleep(2); // wait thread creation
	
	// sending loop
	start  	= time(0);
	int sendflag  = 1;

	while(sendflag)
	{
		sendpacket();
		if (time(0)-start > (MEASURETIME-1)) sendflag = 0;
	}
	
	printf("\nnumber of sent packets     (%i Bytes) = %i",LOCALPACKETSIZE,sendno);	
	printf("\nnumber of received packets (%i Bytes) = %i\n",LOCALPACKETSIZE,receiveno);
	
	// speed calculation
	a = (float) sendno*8;
	b = (float) LOCALPACKETSIZE*MEASURETIME;
	printf("\nSpeed = %f MBit/s\n",a/b);//sendno/1024);
	pthread_join(rec_thread, NULL);

	close(sockReceive);
	close(sockSend);
	getchar();

	return(0);
}


