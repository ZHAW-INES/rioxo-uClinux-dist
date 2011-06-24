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
** Description : test program for bcrypt
**
** Change History
** Date       |Name         |Modification
**************|*************|**************************************************
** 15.03.2011 | Lukas Itin  |file created
******************************************************************************/
#define LENGHT_CERTRX 1044
#define LENGHT_PRIVATE_KEY 128
#define LENGHT_LC128 32
#define LENGHT_OFFSET 8


#include <stdlib.h>
#include <stdio.h>
#include "b_main.h"
#include "string.h"


/******************************************************************************
****************************** main program ***********************************
******************************************************************************/
int main(void){
  unsigned long sz;
  char *input;
  char *str;
  int i;
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

  /*for (i=0;i<sz;i++){   
    printf("%c", *(input + i));
  }*/

  //copy input to a string
  for (i=0;i<sz;i++){   
    str[i]=*(input+i);
  }

 //  printf("The string: %s\n", str);


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

  return 0;
}



