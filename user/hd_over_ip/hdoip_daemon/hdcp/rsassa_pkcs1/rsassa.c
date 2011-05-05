/***********************************************************************
SIGNATURE VERIFICATION OPERATION: RSASSA-PKCS1-v1_5-VERIFY((n,e),M,S)
	Input (n,e) 	signers RSA public key
	M		message whose signature is to be verified
	S		signature to be verified
	return value: '1' if valid				      */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../sha256/sha256.h"
#include "../bigdig/bigd.h"
#include "rsassa.h"
#include "../protocol/protocol.h"

static int debug = 0;  	//set to '1', all debug messages will be displayed

int rsassa_verify(char* n, char* e, char* M, char* S){
  BIGD n_bd,e_bd,res,sig,message;
  n_bd = bdNew();
  e_bd = bdNew();
  res = bdNew();
  sig = bdNew();
  message = bdNew();

 //this part of EM is static because the Public Key Certificate in HDCP has always the same size	
  char EM[769] ="0001ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff003031300d060960864801650304020105000420";
  char mess[277]; 
  char hash[65];  
  int boolean;

  //kpub_dcp (3072-bit public key)
  bdConvFromHex(n_bd,n);
  if(debug)printf("n: \n");
  if(debug)bdPrint(n_bd, BD_PRINT_NL);
  bdConvFromHex(e_bd,e); //exponent
  if(debug)printf("e: \n");
  if(debug)bdPrint(e_bd, BD_PRINT_NL);
  //signature to check in hex (input variable)
  bdConvFromHex(sig, S);
  if(debug)printf("sig: \n");
  if(debug)bdPrint(sig, BD_PRINT_NL);
  //message whose signature has to be verified
  bdConvFromHex(message, M);
  if(debug)printf("message: \n");
  if(debug)bdPrint(message, BD_PRINT_NL); 

  //RSAVP1: res = RSAVP1((n,e),s)
  //(n,e) = public key
  //res=s^e mod n 
  bdModExp(res, sig, e_bd, n_bd);
  if(debug)printf("Sig: \n");
  if(debug)bdPrint(res, BD_PRINT_NL);

  //EMSA-PKCS1-v1_5-ENCODE (M, emLen)
  /*M		message to be encoded
  emLen		intendet lenght in octets of the encoded message
  EM (output)	encoded message
  1:	H=Hash(M) */
  bdConvToHex(message, mess, 277);
  add_leading_zeros(mess,sizeof(mess));
  if(debug)printf("Message string:%s\n",mess);
  sha256(mess, hash);
  strcat(EM, hash);
  if(debug)printf("Final EM:%s\n",EM);	
  bdConvFromHex(message, EM);
  if (debug) printf("Final EM Bigdig:\n");
  if (debug) bdPrint(message, BD_PRINT_NL);
  boolean = bdIsEqual(message, res);
  if (debug) printf("Strings are equal? (1=true): %i\n",boolean);
 
  /* Free all objects we made */
  bdFree(&n_bd);
  bdFree(&e_bd);
  bdFree(&res);
  bdFree(&sig);
  bdFree(&message);

  return boolean;
}
