/* this is a collection of all the functions needed for rsaes-oaep encryption/decryption
regarding to HDCP */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../sha256/sha256.h"
#include "../bigdig/bigd.h"
#include "rsaes.h"
#include "../protocol/protocol.h"

static int debug = 0;  		   //set to '1', all debug messages will be displayed


/************************* RSAES_OAEP-ENCRYPT ****************************************/
/* RSAES_OAEP-ENCRYPT((n,e),M,L) function
   Input:	n,e:	RSA public key
		M:	message to be encrypted
		L:	empty string
   Output:	C:	ciphertext, an octet string of 1024 bits
*/
void rsaes_encrypt(char* kpubrx,char* kpubrx_e,char* message, char* ciphertext, char* seed){
  BIGD kpub_rx_n_bd, kpub_rx_e_bd, EM_bd, c;
  kpub_rx_n_bd = bdNew();
  kpub_rx_e_bd = bdNew();
  c = bdNew();
  EM_bd = bdNew();

  int i;
  char T[191];
  char seedmask[65]; 
  char maskedseed[65];
  char EM[257]="00";
  //hash of nothing (this value is constant if message lenght is constant to):
  char s_part1[191]="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b8550000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";
  strcat(s_part1, message);  //add to a final DB string
  
  for (i=0;i<190;i++){
    T[i]= '0';
  }
  T[190] ='\0';
  mgf1(seed, T);
  if (debug) printf("TE: %s\n", T);
  if (debug) printf("dbmask: \n");
  xor_strings(s_part1, T, T, 190);

  if (debug) printf("the hex string = %s\n", T);
 //seedMask = MGF(maskedDB,hLen)
  for (i=0;i<64;i++){
    seedmask[i]= '0';
  }
  seedmask[64] ='\0';
  mgf1(T, seedmask);
  if (debug) printf("seedmask: %s\n", seedmask);
  xor_strings(seedmask, seed, maskedseed, strlen(seed));

  if (debug) printf("maskedseed: %s\n", maskedseed);
  //EM = 0x00 || maskedseed || maskeddb
  //char EM[129]="00";
  strcat(EM, maskedseed);  //add to a final string
  strcat(EM, T);
  if (debug) printf("EM: %s\n", EM); 
  bdConvFromHex(EM_bd, EM);
  //RSA encryption
  //RSAEP: c=RSAEP((n,e),OS2IP(EM))
  //Receiver public key
  bdConvFromHex(kpub_rx_n_bd, kpubrx); 
  bdConvFromHex(kpub_rx_e_bd, kpubrx_e);  //exponent
  //m=s^e mod n 
  bdModExp(c, EM_bd, kpub_rx_e_bd, kpub_rx_n_bd);
  if (debug) printf("\nFinal Ekpub(km):\n");
  if (debug) bdPrint(c, BD_PRINT_NL); 
  bdConvToHex(c, ciphertext, 257);
  add_leading_zeros(ciphertext,257);
  if (debug) printf("Final Ekpub(km) string:%s\n",ciphertext);

  /* free memory */
  bdFree(&kpub_rx_n_bd); 
  bdFree(&kpub_rx_e_bd);
  bdFree(&EM_bd);
  bdFree(&c);
}

/************************** RSAES_OAEP-DECRYPT ****************************************/
/* RSAES_OAEP-DECRYPT(K,C,L) 
   function to decrypt an RSAES-OAEP ciphertext
   Input:	K:	receivers private key
		C:	ciphertext to be decrypted
		L:	empty string
   Output:	M:	decrypted output message
*/
void rsaes_decrypt(char* ciphertext, char* km, char* p, char* q, char* dp, char* dq, char* qInv){ //km has to be 32 char array
  BIGD p_bd, q_bd, dp_bd, dq_bd, qInv_bd, m1, m2, c_text, h;
  p_bd = bdNew();        q_bd = bdNew();           dp_bd = bdNew();
  qInv_bd = bdNew();     m1 = bdNew();             m2 = bdNew();
  h = bdNew();           c_text = bdNew();         dq_bd = bdNew();

  int i;
  char EM[257];
  char y[3];
  char maskedSeed[65];
  char maskeddb[191];
  char seedMask[]="0000000000000000000000000000000000000000000000000000000000000000"; //initialize to the right lenght
  char seed[]="0000000000000000000000000000000000000000000000000000000000000000";
  char dbmask[191];
  char db_char[191];
  /*receiver private key*/
  bdConvFromHex(p_bd, p);
  bdConvFromHex(q_bd, q);
  bdConvFromHex(dp_bd, dp);
  bdConvFromHex(dq_bd, dq);
  bdConvFromHex(qInv_bd, qInv);
  /*ciphertext to be decrypted*/
  bdConvFromHex(c_text, ciphertext);
  if(debug)printf("ciphertext to be decrypted: \n");
  if(debug) bdPrint(c_text, BD_PRINT_NL);
  /*RSADP: m=s^e mod n */
  if(debug)printf("c_text \n");  
  if(debug)bdPrint(c_text, BD_PRINT_NL);
  if(debug)printf("dp_bd \n");  
  if(debug)bdPrint(dp_bd, BD_PRINT_NL);
  if(debug)printf("p_bd \n");  
  if(debug)bdPrint(p_bd, BD_PRINT_NL);

  bdModExp(m1, c_text, dp_bd, p_bd);
  bdModExp(m2, c_text, dq_bd, q_bd);
  if(debug)printf("m1: \n");
  if(debug)bdPrint(m1, BD_PRINT_NL);
  if(debug)printf("m2: \n");
  if(debug)bdPrint(m2, BD_PRINT_NL);
  if (bdCompare(m1, m2) < 0)
  bdAdd(m1, m1, p_bd);
  bdSubtract(m1, m1, m2);
  if(debug)printf("After sub: \n");
  if(debug)bdPrint(m1, BD_PRINT_NL);
  bdModMult(h, m1, qInv_bd, p_bd);
  bdMultiply(m1, q_bd, h);
  bdAdd(h, m2, m1);
  if(debug)printf("Output of RSADP: \n");
  if(debug)bdPrint(h, BD_PRINT_NL);
  /*EME-OAEP decoding*/
  bdConvToHex(h, EM, sizeof(EM));
  add_leading_zeros(EM, sizeof(EM));
  if(debug)printf("the hex string = %s\n", EM);
  /*seperate EM to Y, maskedSeed and maskedDB*/
  for (i=0;i<2;i++){
    y[i] = EM[i];
  }
  y[2] = '\0';
  for (i=0;i<64;i++){
    maskedSeed[i] = EM[i+2];
  }
  maskedSeed[64] = '\0';
  for (i=0;i<190;i++){
    maskeddb[i] = EM[i+64+2];
  }
  maskeddb[190] = '\0';
  if(debug)printf("y = %s\n", y);
  if(debug)printf("maskedSeed = %s\n", maskedSeed);
  if(debug)printf("maskeddb = %s\n", maskeddb);
  //seedMask = MGF(maskedDB,seedmask)
  mgf1(maskeddb, seedMask);
  if(debug)printf("maskeddb1 = %s\n", maskeddb);
  if(debug)printf("seedMask = %s\n", seedMask);
  xor_strings(maskedSeed, seedMask, seed, strlen(seed));

  if(debug)printf("seed = %s\n", seed);
  if(debug)printf("maskeddb2 = %s\n", maskeddb);
  for (i=0;i<190;i++){
    dbmask[i] = '0';
  }
  dbmask[190] = '\0';
  mgf1(seed, dbmask); /*let dbMask=MGF(seed,k-hLen-1) */
  xor_strings(dbmask, maskeddb, db_char, sizeof(db_char)-1);

  for (i=0;i<32;i++){
    km[31-i] = db_char[strlen(db_char)-1-i]; 
  }
  km[32]='\0';
  if (debug) printf("km: %s\n", km); 

  /* free memory */
  bdFree(&p_bd); 
  bdFree(&q_bd);
  bdFree(&dp_bd);
  bdFree(&dq_bd);
  bdFree(&qInv_bd);
  bdFree(&m1);
  bdFree(&m2);
  bdFree(&c_text);
  bdFree(&h);
}
/************************************ MGF1 ********************************************/
/*MGF1 is a mask generation function based on the sha256 function*/
/*Input: seed, Output: T
'T' has to be a string with the desired lenght
Be aware that the lenght of 'T' is an input too
*/
void mgf1(char* seed, char* T){
  unsigned int counter,i,j,k;
  char out[65]; 
  char temp_c[]="00000000";
  char seed_count[250]; // (strlen(seed)+9)!<250

  /*calculate number of loops*/
  if (strlen(T)%64 == 0)
    counter=(int)(strlen(T)/64);     //round up
  else
    counter=(int)((strlen(T)/64)+1); //64bytes is the lenght of the sha256 output
  /*seed extendet with counter val*/
  for (i=0;i<(strlen(seed)+8);i++){  //with 0 initialize
    seed_count[i]='0';
  }
  seed_count[strlen(seed)+8] = '\0';
  for (i=0;i<strlen(seed);i++){
    seed_count[i]=seed[i];
  }
  /*concatenate the counter val at the end*/
  for (i=0;i<counter;i++){
    sprintf(temp_c, "%08x", i); 
    //if (debug) printf("%s\n",temp_c);
    for(k=0;k<8;k++){
      seed_count[strlen(seed)+k]=temp_c[k];
    }
    //if (debug) printf("seedcount: %s\n", seed_count);
    sha256(seed_count, out); 
    for (j=0;j<64;j++){
      if ((j+64*i)<strlen(T)) 	     //stop if the end of the output string is reached  
        T[j+64*i]=out[j];
    }
  
  }
}


