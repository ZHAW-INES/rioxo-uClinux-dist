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
** Description : Authentification and Key Exchange for a receiver
**
** Change History
** Date       |Name         |Modification
**************|*************|**************************************************
** 20.09.2010 | Lukas Itin  |file created
******************************************************************************/
#define LENGHT_CERTRX 1044
#define LENGHT_PRIVATE_KEY 128
#define LENGHT_LC128 32
#define LENGHT_OFFSET 8

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../rsaes-oaep/rsaes.h"
#include "../rsassa_pkcs1/rsassa.h"
#include "../aes128/aes_encrypt.h"
#include "../sha256/sha256.h"
#include "../protocol/protocol.h"
#include "../bcrypt/b_main.h"

/******************************************************************************
****************************** main program ***********************************
******************************************************************************/
int receiver(char* port_nr, char* ip_nr, char* session_key, char* riv, uint32_t keys[]){

  /**************************** Global constants: ******************************/
  // DCP LLC public key certificate
 char certrx[]= "745bb8bd04bc83c79578f90c914b8938055aa4ac1fa80393827975af6622de43808dcd5d90b83cb3d89eb00d0944f43f5fabb9c4c996ef78b58f6977b47d08149c81a08f041fa088e120c7344a49356599cf5319f0c68176055cb9deddab3db092a1234f0c71304278f655aebd36258e250d4e5e8e776a60e3c1e9eecd2b9e186397d4e67501000100001d0a61eaabf8a82b0269a134fd91ac2bf28f348bd484fa62bc014a4aa2b214bfb5f4dfac80930d13ec9ce5d83470519a6680ebbecc7e45f0e6396384c9b98e8caf9ca9d40eeb9a572a1741ca97f31996b55d0f30a384e573a2ed05697a22ce841f3e399e2876c9bc895b70b17bf4edb67412ab482964ce6c6004eba97aa215a6589aad32c75339e5fef037a7a0c5ffecd9b005bb2513a0a4c70b2a5dc68f5111cb36ed5c177e2220c3eb408c67bb1cd247b0e0bde74ccd5dd52312f83b1d913bf3c760ea902448e592216cf6d95e768d2b86a67c16aea83608a037141ad703e14031ca6c95e010b043cfb7e03005b9acb70868cd7e11472a033beb74c819628b2f1191b6064fe02a44204329131fddd04a116c0e83bf22623bebecd77628ba648842c873a79e4a693ab20c4b3ad950db7c51ee15e06b2c63a69157ddbf174723ad15cbb991180b518ff91c5167c10b78f5d955dc48e4c083a5df75e2dc88d2c6dddf1f379035f6fddae0043269c1afd9f911c5aa7458321c71aaa714fb231722";   
  
  //char port_nr[]="55000";
  //receiver private key
  char p[]="dc1a02b836ed3ae87474cd72284aee3190e4d06af9f6f8d35029c2849798105dea7b88fd36c50499adab270a5a2af9187b7db0c3cbe35ac29a10f7c99a183eb5";
  char q[]="db42e942e32a78c96f2b7b74d69baeb93df4e735901cc45ab4228c3c089ba529645729b2c480f9eec694303ed2339fbb6a430389149b8f20b8601f7f303b20c1";
  char dp[]="73d1a418b79e81df0c58e23aee04efee59266e9dbc473f8c42a496dd1ac043ec8794d5f318bcf7bcbe6c4fb0dcddbc122bf969e8be0337212bdd3de67215cbf9";
  char dq[]="091de61f0edd043ab3f1a5e77cc8ea61ef6e90728cb47581a3fdcfc0eb46b57e5c1ab7b424318cb2ddf4e970a342dc4069b1b1a2f0856b551bf57b39c9a29bc1";
  char qInv[]="8958a5a363d9a9ee0e7ea1c0562d59fcf8661c2648219de061e4a8069764c7017747118ea281d200dd5a1b8f7a1b2c685639cfcdd36aff73811d913d48b4434c";
  // DCP LLC global constant
  char lc128[]="93ce5a56a0a1f4f73c658a1bd2aef0f7";
  char repeater[]="01";
  int i,j;
  /**************************************************************/
  //read the encrypted keys from file
/*  unsigned long sz;
  char *input;
  char *str;
  
  char *certrx;
  char *p;
  char *q;
  char *dp;
  char *dq;
  char *qInv; 
  char *lc128;
  
  //get the decrypted data
  bcrypt_main(&sz, &input);
  //alloc a string and copy the data to the string
  str = (char*)calloc(sz, sizeof(char));

  //copy input to a string
  for (i=0;i<sz;i++){   
    str[i]=*(input+i);
  }

  //get the keys out of the string
  certrx=&str[LENGHT_OFFSET];
  str[LENGHT_OFFSET+LENGHT_CERTRX]='\0';
  printf("CERTRX: %s\n",certrx);

  p=&str[2*LENGHT_OFFSET+LENGHT_CERTRX+1];
  str[2*LENGHT_OFFSET+LENGHT_CERTRX+LENGHT_PRIVATE_KEY+1]='\0';
  printf("p: %s\n",p);

  q=&str[3*LENGHT_OFFSET+LENGHT_CERTRX+LENGHT_PRIVATE_KEY+2];
  str[3*LENGHT_OFFSET+LENGHT_CERTRX+2*LENGHT_PRIVATE_KEY+2]='\0';
  printf("q: %s\n",q);
  
  dp=&str[4*LENGHT_OFFSET+LENGHT_CERTRX+2*LENGHT_PRIVATE_KEY+3];
  str[4*LENGHT_OFFSET+LENGHT_CERTRX+3*LENGHT_PRIVATE_KEY+3]='\0';
  printf("dp: %s\n",dp);

  dq=&str[5*LENGHT_OFFSET+LENGHT_CERTRX+3*LENGHT_PRIVATE_KEY+4];
  str[5*LENGHT_OFFSET+LENGHT_CERTRX+4*LENGHT_PRIVATE_KEY+4]='\0';
  printf("dq: %s\n",dq);

  qInv=&str[6*LENGHT_OFFSET+LENGHT_CERTRX+4*LENGHT_PRIVATE_KEY+5];
  str[6*LENGHT_OFFSET+LENGHT_CERTRX+5*LENGHT_PRIVATE_KEY+5]='\0';
  printf("qInv: %s\n",qInv);

  lc128=&str[7*LENGHT_OFFSET+ +LENGHT_CERTRX+5*LENGHT_PRIVATE_KEY+6];
  str[7*LENGHT_OFFSET+LENGHT_CERTRX+5*LENGHT_PRIVATE_KEY+LENGHT_LC128+6]='\0';
  printf("lc128: %s\n",lc128);
*/
  /**************************************************************/

  static int debug = 0;

  //char rrx[]="3ba0bede0c46a991"; //TODO: random nr
  char rrx[17];  //a random number
  char data[600];
  int sockfd;
  char rtx[19];
  char in_hmac[97];
  char km[33];
  char dkey0[]="0000000000000b6c6ede4de766248c9033e9507ddaac9723f2a9f975a04a9400000000000000000000";
  char dkey1[]="00000000000000000000000000000000";
  char kd[65]="";
  char plaintext1[33];
  char plaintext2[33];
  char H[67];
  char key_l[65];
  char no_repeater[]="00";
  //char rn[19];
  char rn[97]; 
  char L[67];  //FOR TEST ONLY
  char rrx_pad[65]="000000000000000000000000000000000000000000000000";
  char Ed_riv[51];
  char Edkey_ks[33];
  char dkey2[]="00000000000000000000000000000000";
  char plaintext3[33];
  char key2[33];
  char ks[33];
  char xor_val[65];
  char temp[]="0000000000000000";
  //char session_key[33];

  //sockfd = start_server(port_nr); //run server 

  //to save time later, generate random number for rrx now
  if (generate_random_nr(rrx)==1) {
    printf("Couldn't generate random number for rrx! Try pseudo random number generator!\n");
    if (pseudo_random_nr(rrx)==1) {
      printf("Couldn't generate pseudo random number for rrx! EXIT!\n");
      return 0;
    }  
  }
  printf("IP: %s\n",ip_nr);
  printf("Port_nr: %s\n",port_nr);
  sockfd = start_transmitter(ip_nr, port_nr);//try to connect to receiver
  TX_Start(sockfd); //force transmitter to start SKE

  printf("\n********************************************************************************");
  printf("\n**************************** RECEIVER START ************************************");
  printf("\n********************************************************************************\n");
  printf("\n********************** wait for AKE_Init ***************************************\n");

  receive_mess(rtx,sockfd);
  //check if SKE or reset_key has to start  
  //if (rtx[1]='f'){
    //printf("Start reset HDCP keys...\n");
    /*Routine to reset the HDCP keys comes here
     *
     */
  //}

  
  printf("\n*********** Received rtx: %s, send certificate *****************\n", rtx);
  AKE_Send_Cert(certrx,repeater,sockfd);
  printf("\n************************* wait for encrypted km ********************************\n");
  receive_mess(data,sockfd);
  //check if message id is correct  
  if (data[1]!='4'){
    printf("Wrong message or protocol aborted by transmitter! Expected message id: 4! EXIT!\n");
    return 0;
  }
  //delete the first two chars to get original Ekpub(km)
  for (i=0;i<256;i++){
    data[i]=data[i+2];
  }  
  data[256]='\0';
  printf("Received AKE_No_Stored:\n%s", data);
  //generate rrx and send AKE_Send_rrx
  
  if (debug) printf("\n********************* send rrx: %s *******************************\n",rrx);
  AKE_Send_rrx(rrx,sockfd);

  //decrypt km with kprivrx  
  rsaes_decrypt(data,km,p,q,dp,dq,qInv);
  if (debug) printf("*************** decrypted km: %s *****************\n",km);
  
  //compute HMAC-SHA256
  //calculate dkey0 and dkey1  
  for (i=0;i<16;i++){
    plaintext1[i]=rtx[i+2];//i+2: delete the first two chars 
    plaintext2[i]=rtx[i+2];
    rtx[i]=rtx[i+2];
  }
  plaintext1[16]='\0';
  plaintext2[16]='\0';
  rtx[16]='\0';
  strcat(plaintext1,"0000000000000000"); 
  strcat(plaintext2,"0000000000000001"); 
  aes_128(km,plaintext1,dkey0);
  //aes_128(km1,plaintext1,dkey1);
  if (debug) printf("\nplaintext1\n%s",plaintext1);
  //plaintext[31]='1';
  if (debug) printf("\nplaintext2\n%s",plaintext2);
  aes_128(km,plaintext2,dkey1);
  //connect dkey0 and dkey1 to kd
  strcat(kd,dkey0); 
  strcat(kd,dkey1); 
  if (debug) printf("\nkd: %s\n",kd);
  //use Hardware HMAC calc. 
  xor_strings(rtx, repeater, in_hmac, 16);
  strcat(in_hmac,kd);
  strcat(in_hmac,"0000000000000000");
  if (debug) printf("in_hmac: %s\n",in_hmac);
  //if (hmac_sha256(in_hmac,H)){
    //try software HMAC calculator if HW doesn't work
    hmac(rtx,kd,H,repeater); 
  //}

  if (debug) printf("\nH: %s",H);
  printf("\n****************************** Send H ******************************************\n");
  //send AKE_Send_H_prime to the transmitter
  AKE_Send_H_prime(H, sockfd);

  printf("\n************************ start locality check **********************************\n");
  //wait for LC_init message

  receive_mess(rn,sockfd);
  //check if message id is correct, EXIT if wrong!   
  if (rn[1]!='9'){
    printf("Wrong message or protocol aborted by transmitter! Expected message id: 9! EXIT!\n");
    close(sockfd);
    return 0;
  }
  do{

    if (debug) printf("rn:\n%s\n",rn);
    for (i=0;i<17;i++){		//FOR TEST ONLY!!!
      rn[i]=rn[i+2];
    }
    strcat(rn,kd);
    strcat(rn,rrx);
   // hmac_sha256(rn,L);     //use Hardware HMAC calc. 

  //  if (hmac_sha256(rn,L)){
    //try software HMAC calculator if HW doesn't work
    //calculate L = HMAC-SHA256(rn,kd XOR rrx)
    // XOR kd with rrx
    //pad rrx with zeros to 256 bits
    rn[16]='\0';  //undo the concatenation before
    strcat(rrx_pad,rrx);
    xor_strings(rrx_pad, kd, key_l, 64);
    hmac(rn,key_l,L,no_repeater);
 // }  


    if (debug) printf("Calculated L: \n%s",L);
    //send L to transmitter
    if (debug) printf("\n************************** send L to transmitter *******************************\n");
    LC_Send_L_prime(L,sockfd);

    receive_mess(Ed_riv,sockfd);
    if (Ed_riv[1]=='9'){
      for (i=0;i<19;i++) rn[i]=Ed_riv[i]; 
      for (i=0;i<48;i++) rrx_pad[i]='0';
      rrx_pad[48]='\0';    
    }
  } while (Ed_riv[1]=='9');  //repeat until a valid message to start SKE is received

   //check if message id is correct, EXIT if wrong!
  if (Ed_riv[1]!='b'){
    printf("Wrong message or protocol aborted by transmitter! Expected message id: b! EXIT!\n");
    return 0;
  }
  printf("Locality check successful!\n");
  printf("\n************************* start Session key exchange ***************************\n");
  //wait for SKE_Send_Eks
  if (debug) printf("\nEd_riv    : %s\n",Ed_riv);
  //get Edkey_ks out of Ed_riv
  for (i=0;i<32;i++){
    Edkey_ks[i]=Ed_riv[i+2];
  }
  Edkey_ks[32]='\0';
  //get riv out of Ed_riv
  for (i=0;i<16;i++){
    riv[i]=Ed_riv[i+34];
  }
  riv[16]='\0';
  if (debug) printf("\nriv       : %s\n",riv);
  //calculate dkey2
  for (i=0;i<17;i++){
    plaintext3[i]=rtx[i];
  }
  strcat(plaintext3,"0000000000000002"); 
  if (debug) printf("Plaintext3: %s\n",plaintext3);
  rn[16] = '\0'; //truncate rn
  if (debug) printf("rn und km: %s %s \n",rn,km);
  xor_strings(rn,km,key2,32);
  if (debug) printf("key2      : %s\n",key2);
  aes_128(key2,plaintext3,dkey2);  
  if (debug) printf("dkey2     : %s\n",dkey2);
  xor_strings(dkey2, rrx, xor_val, 64);
  xor_strings(Edkey_ks, xor_val, ks, 32);
  if (debug) printf("\n*************************** THE SESSION KEY ks *********************************\n");
  if (debug) printf("************************ %s **********************",ks);
  if (debug) printf("\n********************************************************************************\n");
  //xor session key with lc128
  xor_strings(ks, lc128, session_key,32);
  if (debug) printf("\n****************** Session key XORed with  global constant *********************\n");
  if (debug) printf("************************** %s ********************",session_key);
  if (debug) printf("\n********************************************************************************\n");
  printf("\n****************** HDCP Session key exchange successful! ***********************\n");


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


