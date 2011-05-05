/******************************************************************************
**  _____       ______  _____                                                 *
** |_   _|     |  ____|/ ____|  Zürcher Hochschule für                        *
**   | |  _ __ | |__  | (___    Angewandte Wissenschaften                     *
**   | | | '_ \|  __|  \___ \   Institute of Embedded Systems                 *
**  _| |_| | | | |____ ____) |  (University of Applied Sciences)              *
** |_____|_| |_|______|_____/   8401 Winterthur, Switzerland                  *             
*******************************************************************************
**
** Project     : HD over ip with HDCP encryption
**
** Description :-All message functions are collected her 
**		-Functions to acess HW drivers
**		-All auxiliary functions like xor_strings etc.	
**
** Change History
** Date       |Name         |Modification
**************|*************|**************************************************
** 02.12.2010 | Lukas Itin  |file created
******************************************************************************/

#include "../bigdig/bigd.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "protocol.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stddef.h>

//TODO: could be optimized to just one function for all messages


static int debug = 0;  //set to '1', all debug messages will be displayed
int i;

//*************************** TX_Start ****************************************
//  Function to inform the transmitter to start authentication
//  message id = 0 			
//*****************************************************************************				  
void TX_Start(int sock){
  char payload[3];
  payload[0]='0';//set message id
  payload[1]='0';
  payload[2]='\0';
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** AKE_Init ****************************************
//  Function to initate authentication
//  message id = 2 			
//*****************************************************************************				  
void AKE_Init(char* rtx, int sock){
  if (strlen(rtx) !=16){
    printf("AKE_Int: wrong lenght of rtx\n");
    exit(0);
  }
  char payload[19];
  payload[0]='0';//set message id
  payload[1]='2';
  payload[18]='\0';
  for (i=0;i<16;i++){
    payload[i+2]=rtx[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** AKE_Send_Cert ***********************************
//  receiver sends certificate
//  message id = 3 	
//  repeater: '01'=true, '00'=false 
//  total number of octets: 1048					 
//*****************************************************************************
void AKE_Send_Cert(char* certrx, char* repeater, int sock){
  if (strlen(certrx) !=1044){
    printf("AKE_Send_Cert: wrong lenght of certrx\n");
    exit(0);
  }
  char payload[1049];
  payload[0]='0';//set message id
  payload[1]='3';
  payload[2]=repeater[0];
  payload[3]=repeater[1];
  payload[1048]='\0';
  for (i=0;i<1044;i++){
    payload[i+4]=certrx[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}

//****************************** AKE_No_Stored_km *****************************
//  transmitter sends encrypted km
//  message id = 4 	
//  total number of octets: 258					  
//*****************************************************************************
void AKE_No_Stored_km(char* km_enc, int sock){
  if (strlen(km_enc) !=256){
    printf("AKE_No_Stored_km: wrong lenght of km_enc\n");
    exit(0);
  }
  char payload[259];
  payload[0]='0';//set message id
  payload[1]='4';
  payload[258]='\0';
  for (i=0;i<256;i++){
    payload[i+2]=km_enc[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//***************************** AKE_Stored_km *********************************
//  transmitter sends Ekh_km and m
//  message id = 5 	
//  total number of chars: 66					  
//*****************************************************************************
void AKE_Stored_km(char* Ekh_km, char* m, int sock){
  if ((strlen(Ekh_km) !=32) || (strlen(m) !=32)){
    printf("AKE_Stored_km: wrong lenght of Ekh_km or m\n");
    exit(0);
  }
  char payload[67];
  payload[0]='0';//set message id
  payload[1]='4';
  payload[66]='\0';
  for (i=0;i<64;i++){
    if (i<32)
      payload[i+2]=Ekh_km[i];
    if (i>31)
      payload[i+2]=m[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** AKE Send rrx ************************************
//  receiver sends rrx
//  message id = 6 	
//  total number of octets: 18					  
//*****************************************************************************
void AKE_Send_rrx(char* rrx, int sock){
  if (strlen(rrx) !=16){
    printf("AKE_Send_rrx: wrong lenght of rrx\n");
    exit(0);
  }
  char payload[19];
  payload[0]='0';//set message id
  payload[1]='6';
  payload[18]='\0';
  for (i=0;i<18;i++){
    payload[i+2]=rrx[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** AKE_Send_H_prime ********************************
//  receiver sends h prime
//  message id = 7 	
//  total number of octets: 66					  
//*****************************************************************************
void AKE_Send_H_prime(char* H, int sock){
  if (strlen(H) !=64){
    printf("AKE_Send_H_prime: wrong lenght of H\n");
    exit(0);
  }
  char payload[67];
  payload[0]='0';//set message id
  payload[1]='7';
  payload[66]='\0';
  for (i=0;i<64;i++){
    payload[i+2]=H[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** AKE_Send_Pairing_Info ***************************
//  receiver sends pairing info
//  message id = 8 	
//  total number of octets: 34					  
//*****************************************************************************
void AKE_Send_Pairing_Info(char* ekh_km, int sock){
  if (strlen(ekh_km) !=32){
    printf("AKE_Send_Pairing_Info: wrong lenght of ekh_km\n");
    exit(0);
  }
  char payload[35];
  payload[0]='0';//set message id
  payload[1]='8';
  payload[34]='\0';
  for (i=0;i<34;i++){
    payload[i+2]=ekh_km[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** LC_init *****************************************
//  transmitter starts locality check
//  message id = 9 	
//  total number of octets: 18					  
//*****************************************************************************
void LC_init(char* rn, int sock){
  if (strlen(rn) !=16){
    printf("LC_init: wrong lenght of rn\n");
    exit(0);
  }
  char payload[19];
  payload[0]='0';//set message id
  payload[1]='9';
  payload[18]='\0';
  for (i=0;i<16;i++){
    payload[i+2]=rn[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** LC Send L Prime *********************************
//  receiver replies to locality check request
//  message id = 10 	
//  total number of octets: 66					  
//*****************************************************************************
void LC_Send_L_prime(char* L_s, int sock){
  if (strlen(L_s) !=64){
    printf("LC_Send_L_prime: wrong lenght of L_s\n");
    exit(0);
  }
  char payload[67];
  payload[0]='0';//set message id
  payload[1]='a';
  payload[66]='\0';
  for (i=0;i<64;i++){
    payload[i+2]=L_s[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** SKE Send Eks  ***********************************
//  transmitter sends session key and riv
//  message id = 11 	
//  total number of octets: 50					  
//*****************************************************************************
void SKE_Send_Eks(char* Edkey_ks, char* riv, int sock){
  if ((strlen(Edkey_ks) !=32) || (strlen(riv) !=16)){
    printf("SEK Send Eks: wrong lenght!\n");
    exit(0);
  }
  char payload[51];
  payload[0]='0';//set message id
  payload[1]='b';
  payload[50]='\0';
  for (i=0;i<32;i++){
    payload[i+2]=Edkey_ks[i];
  }
  for (i=0;i<16;i++){
    payload[i+34]=riv[i];
  }
  if (debug) printf("payload: %s\n", payload);
  send_mess(payload,sock);
}
//*************************** start transmitter *******************************
// Function to start the transmitter
// Input:	ip_nr (string with ip number), port_nr (string with port nr)
// Return:	The socket number
//*****************************************************************************
void error(char *msg){
  perror(msg);
  exit(0);
}
int start_transmitter(char* ip_nr, char* port_nr){
  int sockfd, portno;
  struct sockaddr_in serv_addr;
  struct hostent *server;   

  portno = atoi(port_nr); //port Nr.
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");
  server = gethostbyname(ip_nr); //IP Nr.
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  bbzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bbcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR connecting");
  return sockfd;
}
//****************************** send_mess ************************************
// Function to send a message
// Input:	data (the message, a string), sockfd (The socket number)
//*****************************************************************************
void send_mess(char* data, int sockfd){
    char buffer[650];
    int n, i;  
    unsigned char bytes[650];     //maximum message lenght: 600 bytes
    char_to_unsigned(data,bytes); //convert string to unsigned char octets
    bytes[strlen(data)/2]='\0';
    if (debug) printf("Going to send message...\n");
    for (i=0;i<(int)(strlen(data)/2);i++){
       buffer[i]=(char)bytes[i];
    }
    n = write(sockfd,buffer,strlen(data)/2);  //write buffer to socket
    if (n < 0) error("ERROR writing to socket");
}
//****************************** receive_mess *********************************
// Function to wait for a new message (blocking)
// Input:	sockfd (The socket number)
// Output:	data (The received string. The string lenght is determined on 
//		the base of the message id.)
//*****************************************************************************
void receive_mess(char* data, int sockfd){  
  int n,j;
  char buffer[1200];
  char tempchar[]="00";
  data[0]='\0';
  bbzero(buffer,1200);   //reset buffer
  //wait for a message  
  n = read(sockfd,buffer,1199);
  if (n < 0) error("ERROR reading from socket");
  if (debug) printf("Received message:\n");
  //convert to string (from unsigned char)
  for (j=0;j<message_lenght(buffer[0]);j++){
    if (debug) printf("%02x ",(unsigned char)buffer[j]);
    sprintf(tempchar,"%02x",(unsigned char)buffer[j]);
    strcat(data,tempchar); 
  }
}
//*************************** start_server ************************************
// Function to start the server
// Input:	port_nr (string)
// Return:	The new socket (int)
//*****************************************************************************
/*int start_server(char* port_nr){
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);  //open a socket
  if (sockfd < 0) error("ERROR opening socket");
  bbzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(port_nr);                    //convert string to int
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);        //bind socket to address and port
  if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR on binding");
  listen(sockfd,3); //make ready for a client to connect 
  clilen = sizeof(cli_addr); 
  //the new socket (file descriptor) for the client
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0) error("ERROR on accept");
  return newsockfd;
}*/
//***************************** message_lenght ********************************
// This function determines the message lenght on the basis of the message_id
// Input:        buffer_start (1 char with message id )
// Return:       Number of chars 
//*****************************************************************************
int message_lenght(char buffer_start){
  switch(buffer_start){
	case (char)0x00 : return 1; 
              		  break;
        case (char)0x02 : return 9; 
              		  break;
        case (char)0x03 : return 524; 
	                  break;
	case (char)0x04 : return 129; 
	           	  break;
        case (char)0x06 : return 9; 
	           	  break;
	case (char)0x07 : return 33;     
	           	  break;
	case (char)0x08 : return 17; 
	           	  break;
        case (char)0x09 : return 9; 
	           	  break;
	case (char)0x0a:  return 33;     
	           	  break;
        case (char)0x0b:  return 25;     
	           	  break;
	default  	: return 524; //580
     }
}
//***************************** generate_random_nr ****************************
// This function calls the device driver for the HW true random nubmer genertor 
// and returns a 64bit random nuber. The function returns '1' if it was not
// successful 
// output:    random_nr (string with 64bit)				
//*****************************************************************************
int generate_random_nr(char* random_nr){
  int i;
  random_nr[16]='\0';			
  FILE *ptr;			                    /* Pointer to the file*/ 				
  if ((ptr = fopen("../dev/trng","r"))!=NULL){      /* Open the file  */
    if (debug) printf("File sucessfully opened\n");
    for (i=0;i<16;i++){  	             /*read the characters from file */
      random_nr[i]=fgetc(ptr);		
    }
  }
  else{
    if (debug) printf("Couldn't open file\n");
    return 1;
  }
  fclose(ptr);			             /* Close the file.*/
  if (debug) printf("The random nr: %s\n",random_nr);
  return 0;
}


//***************************** pseudo_random_nr *****************************
// This function calls /dev/urandom and generates a 64bit pseudo random number
// This pseudo random numbers are theoretically vulnerable to a cryptographic 
// attacks. The function returns '1' if it was not successful. 
// output:    random_nr (string with 64bit)				
//*****************************************************************************
int pseudo_random_nr(char* random_nr){
  FILE *fp;
  int i=0;
  random_nr[0]='\0';
  if ((fp=fopen("/dev/urandom","r"))!=NULL){
    if (debug) printf("File /dev/urandom sucessfully opened\n");
    for (i=0;i<8;i++){
      sprintf(random_nr,"%s%02x",random_nr,fgetc(fp));
    }
    random_nr[16]='\0';
  }
  else{
    printf("Could not open /dev/urandom\n");
    return 1;
  }
  if (debug) printf("Pseudo Random Nr: %s\n",random_nr);
  fclose(fp);
  return 0;
}

//******************************* hmac_sha256 *********************************
// function to call the Hardware block for calculating the HMAC-SHA256 value
// returns 0 if successful 
// input:     input (sting with 96 chars: rn || kd || rrx), 
//            first 64bits is the message, next 256bits is the key, next 
//            64bits will be xored with the key 
// output:    sha (a 256bit string) 
//	      This function is optimised for the locality check!
//*****************************************************************************				       
int hmac_sha256(char* input, char* sha){
  int i;	
  FILE *ptr;			               /* Pointer to the file*/				
  if ((ptr = fopen("/dev/sha","w"))!=NULL){    /* Open the file  */
    if (debug) printf("File sucessfully opened for writing\n");
    fprintf(ptr, "%s", input);                 //write string to file 	
  }
  else{
    printf("Couldn't open file to write (HMAC function)\n");
    return 1;
  }
  fclose(ptr);			               /* Close the file.*/
  if ((ptr = fopen("/dev/sha","r"))!=NULL){    //open file to read sha value
    if (debug) printf("File sucessfully opened for reading\n");   
    sha[64]='\0';			
    for (i=0;i<64;i++){  	             /*read the characters from file */
    sha[i]=fgetc(ptr);		
    }
  }  	
  else{
    printf("Couldn't open file to read (HMAC function)\n");
    return 1;
  }
  fclose(ptr);
  if (debug) printf("The sha value: %s\n",sha);
  return 0;
}
//******************************* add_leading_zeros ***************************
//   Function to fill up a sting with leading zeros
//   Input/Output:     inout (sting), size (number of reserved chars)
//   The function will add leading zeros up to the reserved stringlenght	
//*****************************************************************************	     
void add_leading_zeros(char* inout, int size){
  int i,j;
  j = strlen(inout);  	          //get actual stringlenght
  for (i=0;i<(j+1);i++){          //shift the string to the right side of 
    inout[size-1-i]=inout[j-i];	  //the reserved space
  }
  for (i=0;i<(size-1-j);i++){	  //fil the empty left side with zeros
    inout[i]='0';
  }
  inout[size-1]='\0';
}
//************************************* xor_strings ***************************
//   Function to xor to hex strings  
//   Input:     s1, s2 (the two strings), lenght (lenght of output string)
//   Output:    out (the output hex string)
//   The function will xor two strings with hex chars bitwise and add leading 
//   zeros up to the desired output stringlenght (lenght)		                      
//*****************************************************************************
void xor_strings(char* s1, char* s2, char*out, int lenght){
  BIGD a, b, c;
  a=bdNew();
  b=bdNew();   
  c=bdNew();
  bdConvFromHex(a, s1); 	   //convert to bigdig objects
  bdConvFromHex(b, s2);
  bdXorBits(c, a, b);  		   //XOR bits
  bdConvToHex(c, out, lenght+1);   //convert back to a hex string
  add_leading_zeros(out,lenght+1); //add leading zeros to desired string lenght
  bdFree(&a);			   //free bigdig objects
  bdFree(&b);
  bdFree(&c);
}
//************************************* bbzero ********************************
//   Function to zero "count" bytes of memory pointed to by "to"
//*****************************************************************************
void bbzero (char* to, int count){
  while (count-- > 0){
    *to++ = 0;
  }
}
//******************************* bbcopy **************************************
//   Function to copy memory regions of arbitrary length
//   Copy "lenght" bytes from memory region pointed to by "source0" to memory
//   region pointed to by "dest0"
//*****************************************************************************
void bbcopy (void const *source0, void *dest0, size_t length){
  char const *source = source0;
  char *dest = dest0;
  if (source < dest)
    for (source += length, dest += length; length; --length)
      *--dest = *--source;
  else if (source != dest)
    for (; length; --length)
      *dest++ = *source++;
}
//******************************* char_to_unsigned ****************************
//   Function to convert a string with hex chars to unsigned chars
//   Input:     input (the input hex string)
//   Output:    unsig_char (pointer to the memory with the unsig. char values)
//*****************************************************************************
void char_to_unsigned(char* input, unsigned char* unsig_char){
  int d;
  size_t i;
  for (i=0;i<strlen(input)/2;i++){
    sscanf(input+i*2,"%02x", &d);
    unsig_char[i]=(unsigned char)d;
  }
  
}

