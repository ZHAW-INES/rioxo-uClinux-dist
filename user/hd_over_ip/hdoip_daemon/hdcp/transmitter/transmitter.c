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
** Description : Authentification and Key Exchange for a Transmitter
**
** Change History
** Date       |Name         |Modification
**************|*************|**************************************************
** 17.09.2010 | Lukas Itin  |file created
******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>   //for function select()
#include <sys/types.h>  //for function select()
#include <unistd.h>     //for function select()
#include "../rsaes-oaep/rsaes.h"
#include "../rsassa_pkcs1/rsassa.h"
#include "../aes128/aes_encrypt.h"
#include "../sha256/sha256.h"
#include "../protocol/protocol.h"
#include <debug.h>


/*  Edit this define to enable/disable the HDCP time limit checks
 *  Options are:
 *      ENABLE
 *      DISABLE */
#define TIME_LIMIT DISABLE

/*Check receivers certificate*/

/******************************************************************************
****************************** main program ***********************************
******************************************************************************/
//int transmitter(char* port_nr, char* ip_nr, char* session_key, char* riv, uint32_t keys[]){
int transmitter(int sockfd, char* ip_nr, char* session_key, char* riv, uint32_t keys[]){

  /**************************** Global constants: ******************************/
  //kpub_dcp (3072-bit public key with exponent)
  char n[]="A2C7555754CBAAA77A2792C31A6DC231CF12C224BF897246A48D2083B2DD04DA7E01A919EF7E8C4754C859725C8960629F39D0E480CAA8D41E91E30E2C77556D58A89E3EF2DA783EBAD1053707F288740CBCFB68A47A27AD63A51F67F1458516498AE6341C6E80F5FF1372855DC1DE5F0186558671E8103314702A5F157B5C653C463A1779ED546AA6C9DFEB2A812A802A46A206DBFDD5F3CF74BB665648D77C6A03141E5556E4B6FA382B5DFB879F9E782187C00C633E8D0FE2A719109B15E11187493349B86632287C87F5D22EC5F3662F79EF405AD41485745F064350CDDE84E73C7D8E8A49CC5ACF73A18A13FF37133DAD57D85122D6321FC0684CA05BDD5F78C89F2D3AA2B81E4AE408556405E694FBEB036A0ABE831894D4B6C3F2589C7A24DDD13AB73AB0BBE5D128ABAD2454720E76D28932EA46D378D0A96778C12D18B033DEDB27CCB07CC9A4BDDF2B64103244068121B3BACF3385491E864CBDF23D34EFD6237A9F2CDA84F08383717DDA6E4496CD1D05DE30F61E2F9C999C6007";
  char e[]="03"; 
  //char ip_nr[]="localhost";
  //char ip_nr[]="192.168.1.201"; //the ip number of the receiver
  //char port_nr[]="55000";		    //the used port number
  // DCP LLC global constant
  char lc128[]="93ce5a56a0a1f4f73c658a1bd2aef0f7";

  static int debug = 0;		   //'1' if you want to see all debug messages

  char data[1200]; 	//receiver buffer
  int okay, i, j;
//int sockfd;
  char repeater[]="00"; 
  //the two parts of the certificate
  char message[277];  
  char signature[769];

  char ciph[257];
  char kpubrx_n[257];  //public key from receiver
  char kpubrx_e[7];	
  char rrx[19];
  char H[67]; //69  //TODO: Eigentlich müsste ein 67 char genügen, gibt aber segmentation fault-->abklären wieso
  char H_own[67];
  char dkey0[]="00000000000000000000000000000000";
  char dkey1[]="00000000000000000000000000000000";
  char kd[65]="";
  char plaintext1[33];
  char plaintext2[33];
  char L_rec[67];
  char no_repeater[]="00";
  char L[67];
  char key_l[65];
  char rrx_pad[65]="000000000000000000000000000000000000000000000000";
  char dkey2[]="00000000000000000000000000000000";
  char plaintext3[33];
  char Edkey_ks[33];  
  char key2[33];
  char xor_val[65];
  char go[10];
  //char ks_lc128[33];

  // The random numbers
  char rtx[]="0000000000000000";
  char temp[]="0000000000000000";
  //char rn[]="0000000000000000";
  char rn[97]; 
  //char riv[] ="0000000000000000";
  char seed[65]="";  
  char km[33]="";
  char ks[33] ="";

  // The default random numbers, for test only!
 /* char rtx[]="18fae4206afb5149";  
  char seed[65]="000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"; 
  char km[]="68bcc51ba9db1bd0faf15e9ad8a5afb9";
  char rn[17]="32753ea878a6381c";
  char ks[] ="f3df1dd95796123f989789b421e12de1"; 
  riv ="402b6b43c5e886d8";*/
  
  /* initialize select() timer for locality check etc. */
  fd_set rfds;
  struct timeval tv;
  int retval;

  //to save time later, generate random numbers now if possible
  if (generate_random_nr(rtx)==1) {
    printf("Couldn't generate random number for rtx! Try pseudo random number generator!\n");
    if (pseudo_random_nr(rtx)==1) {
      printf("Couldn't generate pseudo random number for rtx! EXIT!\n");
      return 0;
    }   
  }
  if (generate_random_nr(riv)==1) {
    printf("Couldn't generate random number for riv! Try pseudo random number generator!\n");
    if (pseudo_random_nr(riv)==1) {
      printf("Couldn't generate pseudo random number for riv! EXIT!\n");
      return 0;
    }   
  }
  for (i=0;i<4;i++){
    if (generate_random_nr(temp)==1){
      printf("Couldn't generate random number for seed! Try pseudo random number generator!\n");
      if (pseudo_random_nr(temp)==1) {
        printf("Couldn't generate pseudo random number for seed! EXIT!\n");
        return 0;
      }   
    }
    strcat(seed,temp);  
  }
  for (i=0;i<2;i++){
    if (generate_random_nr(temp)==1){
      printf("Couldn't generate random number for km! Try pseudo random number generator!\n");
      if (pseudo_random_nr(temp)==1) {
        printf("Couldn't generate pseudo random number for km! EXIT!\n");
        return 0;
      }   
    }
    strcat(km,temp);  
  }
  for (i=0;i<2;i++){
    if (generate_random_nr(temp)==1){
      printf("Couldn't generate random number for ks! Try pseudo random number generator!\n");
      if (pseudo_random_nr(temp)==1) {
        printf("Couldn't generate pseudo random number for ks! EXIT!\n");
        return 0;
      }   
    }
    strcat(ks,temp);  
  }

  if (debug) printf("The random numbers:\n");
  if (debug) printf("rtx : %s\n",rtx);
  if (debug) printf("riv : %s\n",riv);
  if (debug) printf("seed: %s\n",seed);
  if (debug) printf("km  : %s\n",km);
  if (debug) printf("ks  : %s\n\n",ks);

  printf("\n********************************************************************************");
  printf("\n*********************** HDCP TRANSMITTER START *********************************");
  printf("\n********************************************************************************\n");
  //printf("Port_nr: %s\n",port_nr);
  //sockfd = start_server(port_nr); //run server 
  receive_mess(go,sockfd); 
  if (go[1]!='0') {
    printf("Wrong message or protocol aborted by receiver! Expected message id: 0! EXIT!\n");
    return 0;
  }


  //sockfd = start_transmitter(ip_nr, port_nr);//try to connect to receiver 

  /****************** Generate rtx an send it to receiver (AKE_Init) ***********/  

  printf("\n****************************** SEND AKE INIT ***********************************\n");
  AKE_Init(rtx, sockfd);

  /******* wait for the certificate from the receiver and verify signature *****/
  receive_mess(data,sockfd);
  //check if message id is correct, EXIT if wrong! 
  if (data[1]!='3') {
    printf("Wrong message or protocol aborted by receiver! Expected message id: 3! EXIT!\n");
    return 0;
  }
  //check if repeater
  if (data[3]=='1') repeater[1]='1';
  else repeater[1] ='0';
  //delete the first four chars to get original certificate
  for (i=0;i<1045;i++){
    data[i]=data[i+4];
  }
  
  printf("Received certificate:\n%s\n",data);
  //verify signature:  
  //divide the certificate into message an signature
  for (i=0;i<276;i++){ 
    message[i]=data[i];
  }
  message[276]='\0';
  for (i=0;i<768;i++){//769
    signature[i]=data[i+276]; 
  }
  signature[768]='\0'; 
  //call function to check the signature
  //okay = rsassa_verify(n, e, message, signature);
  if (rsassa_verify(n, e, message, signature)==1) printf("Receiver certificate is valid!\n");
  else{
    printf("\nReceiver certificate is not valid! EXIT!\n");
    return 0;
  }
  //printf("\n");
  //TODO: check revocation list!!

  /* if certificate is valid, generate and encryp km and send it to receiver */
  /* Watch sockfd to see when it has input. */
  FD_ZERO(&rfds);
  FD_SET(sockfd, &rfds); 
  /* Wait up to one second. */
  tv.tv_sec = 1;
  tv.tv_usec = 0;
 
  printf("************************** Encrypt and send km *********************************\n");
  //get receiver public key out of the certificate
  for (i=0;i<256;i++){
    kpubrx_n[i]=message[i+10];  
  }
  kpubrx_n[256]='\0';
  for (i=0;i<6;i++){
    kpubrx_e[i]=message[i+266];  
  }
  kpubrx_e[6]='\0';
  if (debug) printf("Receiver public key (n):\n%s\n",kpubrx_n);
  if (debug) printf("Receiver public key (e): %s\n",kpubrx_e);
  rsaes_encrypt(kpubrx_n,kpubrx_e,km,ciph,seed); 
  printf("Encrypted km to send to receiver: \n%s\n", ciph);
  AKE_No_Stored_km(ciph, sockfd);
  
  /*************** wait for rrx (AKE_Send_rrx) and H (AKE_Send_H_prime) ********/
  printf("******************************* Wait for rrx ***********************************\n");
  /* Wait up to one second until H is received */
  tv.tv_sec = 5; //TODO: only for test
  tv.tv_usec = 0;
  retval = select(sockfd+1, &rfds, NULL, NULL, &tv); /* Don't rely on the value of tv now! */
#if TIME_LIMIT == DISABLE
  retval = 1; 
#endif  
  if (retval){
   // printf("Reserve microseconds after received rrx: %ius\n",(int)tv.tv_usec);
   // printf("Reserve seconds after received rrx     : %is\n",(int)tv.tv_sec);
    printf("rrx is available within time!\n"); /* FD_ISSET(0, &rfds) will be true. */
  }
  else{
    printf("rrx is not available within time! Exit!\n");
    return 0;
  }
  receive_mess(rrx,sockfd); 
  //check if message id is correct, EXIT if wrong! 
  if (rrx[1]!='6'){
    printf("Wrong message or protocol aborted by receiver! Expected message id: 6! EXIT!\n");
    return 0;
  }
  if (debug) printf("Received rrx: %s\n",rrx);
  for (i=0;i<16;i++){  //get rid of the message id
    rrx[i]=rrx[i+2];
  }
  rrx[16]='\0';  
  printf("********************************* Wait for H ***********************************\n");
  retval = select(sockfd+1, &rfds, NULL, NULL, &tv); /* Don't rely on the value of tv now! */
#if TIME_LIMIT == DISABLE
    retval = 1; 
#endif  
  if (retval){
   // printf("\nReserve microseconds after received H: %ius\n",(int)tv.tv_usec);
   // printf("Reserve seconds after received H     : %is\n",(int)tv.tv_sec);
    printf("H is available within time!\n"); /* FD_ISSET(0, &rfds) will be true. */
  }
  else{
    printf("H is not available within time! EXIT!\n");
    return 0;
  }
  receive_mess(H,sockfd);
  //check if message id is correct, EXIT if wrong! 
  if (H[1]!='7'){
    printf("Wrong message or protocol aborted by receiver! Expected message id: 7! EXIT!\n");
    return 0;
  }
  
  /**** calculate H (H_own) with own values and compare it with received H *****/
  if (debug) printf("Received H:\n%s\n",H);
  printf("******************* Calculate own H and compare with received ******************\n");
  for (i=0;i<16;i++){
    plaintext1[i]=rtx[i];
    plaintext2[i]=rtx[i];
  }
  //calculate dkey0 and dkey1  
  plaintext1[16]='\0';
  plaintext2[16]='\0';
  strcat(plaintext1,"0000000000000000"); 
  strcat(plaintext2,"0000000000000001"); 
  aes_128(km,plaintext1,dkey0);
  if (debug) printf("\nplaintext1\n%s",plaintext1);
  aes_128(km,plaintext2,dkey1);
  if (debug) printf("\nplaintext2\n%s",plaintext2);
  //connect dkey0 and dkey1 to kd
  strcat(kd,dkey0); 
  strcat(kd,dkey1); 
  if (debug) printf("\nkd: %s\n",kd);
  //calculate HMAC
  hmac(rtx,kd,H_own,repeater);
  if (debug) printf("H_own:\n%s",H_own);
  //Compare own and received H
  for (i=0;i<64;i++){  //get rid of the message id
    H[i]=H[i+2];
  }
  H[64]='\0';
  if (debug) printf("\nReceived H: %s\n",H);
  if (debug) printf("Own      H: %s\n",H_own);
  if(strcmp(H,H_own) == 0){
    printf("HMACS are equal!\n");
  }
  else{
    printf("\nHMACS are not equal!!! EXIT!\n");
    return 0;
  }
  
  //TODO:impelemt pairing

  /******** if AKE was successful, start with locality check **************/
  printf("************************** start locality check ********************************\n");
  for (i=0;i<1024;i++){  //try 1024 times if data is available within time
    // TODO: generate new rn

    //FD_ZERO(&rfds);
    //FD_SET(sockfd, &rfds);  
     
  if (generate_random_nr(rn)==1) {  //generate new rn
    printf("Couldn't generate random number for rn! Try pseudo random number generator!\n");
    if (pseudo_random_nr(rn)==1) {
      printf("Couldn't generate pseudo random number for rn! EXIT!\n");
      return 0;
    }   
  }

    printf("rn: %s\n", rn);
    LC_init(rn,sockfd);
    //wait for LC_Send_L_prime


    tv.tv_sec = 0; 
    tv.tv_usec = 6000;  //5000 okay, 4000 not possible to connect

    /*LC_Send_L_prime has to arrive within 7ms*/
    /* Wait up to seven miliseconds. */

    retval = select(sockfd+1, &rfds, NULL, NULL, &tv); /* Don't rely on the value of tv now! */  
#if TIME_LIMIT == DISABLE
    retval = 1; 
#endif   
    if (retval){
     // printf("\nReserve microseconds after locality check: %ius\n",(int)tv.tv_usec);
      printf("Data is available within time!\n"); /* FD_ISSET(0, &rfds) will be true. */
      i=1024; //break loop
    }
    else{
      printf("No data within time! Round %i\n",i);
      if (i==1023) exit(0);
    }
    receive_mess(L_rec,sockfd);
    //check if message id is correct, EXIT if wrong! 
    if (L_rec[1]!='a'){
      printf("Wrong message or protocol aborted by receiver! Expected message id: a! EXIT!\n");
      return 0;
    }
  }
  /*if data was available within time:*/
  for (i=0;i<65;i++){  //get rid of the message id
    L_rec[i]=L_rec[i+2];
  }

  if (debug) printf("\nkd \n%s\n",kd);
  //use Hardware HMAC calc. 
  strcat(rn,kd);
  strcat(rn,rrx);
  //if (hmac_sha256(rn,L)){
    //try software HMAC calculator if HW doesn't work
    //calculate L = HMAC-SHA256(rn,kd XOR rrx)
    // XOR kd with rrx
    //pad rrx with zeros to 256 bits
    rn[16]='\0';  //undo the concatenation before
    strcat(rrx_pad,rrx);
    xor_strings(rrx_pad, kd, key_l, 64);
    hmac(rn,key_l,L,no_repeater);
 // }     
  //compare received and calculated L
  if (debug) printf("Received L: %s\n",L_rec);
  if (debug) printf("Own      L: %s\n",L);
  if(strcmp(L,L_rec) == 0){
    printf("HMACS of L are equal!\n");
  }
  else{
    printf("\nHMACS of L are not equal!!! EXIT!\n"); 
    return 0;
  }

  /************************* Session key exchange **********************/
  //if locality check was successful start with the session key exchange
  printf("********************* start session key exchange *******************************\n");
  for (i=0;i<17;i++){
    plaintext3[i]=rtx[i];
  }
  plaintext3[16]='\0';
  strcat(plaintext3,"0000000000000002"); 
  if (debug) printf("Plaintext3: %s\n",plaintext3);
  if (debug) printf("rn und km: %s %s \n",rn,km);
  xor_strings(rn,km,key2,32);
  if (debug) printf("key2      : %s\n",key2);
  aes_128(key2,plaintext3,dkey2);  
  if (debug) printf("dkey2     : %s\n",dkey2);
  xor_strings(dkey2, rrx, xor_val, 64);
  xor_strings(ks, xor_val,Edkey_ks,32);
  //send session key and riv to receiver
  SKE_Send_Eks(Edkey_ks,riv,sockfd);
  if (debug) printf("\n*************************** THE SESSION KEY ks *********************************\n");
  if (debug) printf("************************** %s ********************",ks);
  if (debug) printf("\n********************************************************************************\n");
  //xor session key with lc128
  xor_strings(ks, lc128, session_key,32);
  if (debug) printf("\n****************** Session key XORed with  global constant *********************\n");
  if (debug) printf("************************** %s ********************",session_key);
  if (debug) printf("\n********************************************************************************\n");
  printf("\n****************** HDCP Session key exchange successful! ***********************\n");
  //close the socket
  //close(sockfd); 

  //uint32_t keys[6];
  char temp_key[9];
  //convert the session key to an integer array
  for (j=0;j<4;j++){
    for (i=0;i<8;i++){
      temp[i]=session_key[i+j*8];
    }
    temp[8]='\0';
    keys[j]=strtoul(temp, NULL, 16);
  }
  //convert riv to an integer array
  for (j=0;j<2;j++){
    for (i=0;i<8;i++){
      temp[i]=riv[i+j*8];
    }
    temp[8]='\0';
    keys[j+4]=strtoul(temp, NULL, 16);
  }
  return 1; //success

}


