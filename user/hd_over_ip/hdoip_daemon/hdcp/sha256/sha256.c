#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))

#define SHA256_SCR(i)                         \
{                                             \
    sha256_w[k][i] =  SHA256_F4(sha256_w[k][i -  2]) + sha256_w[k][i -  7]  \
          + SHA256_F3(sha256_w[k][i - 15]) + sha256_w[k][i - 16]; \
}


/********************** Functions sha256 and HMAC_sha256 *****************/

#include "../bigdig/bigd.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sha256.h"
#include "../protocol/protocol.h"

static int debug = 0;  //set to '1', all debug messages will be displayed

/***************************** sha256 *************************************/
/* function to calculate a sha256 hash out of a hex string
   input: 	in:   the string to be hashed 
   output:	out:  the sha256 hash of the string
*/

void sha256(char* in, char* out){
  //little helpers...
  int i, k, round, rond_n, count; 
  uint32_t sha256_w[3][64];
  float temp;
  char s[9];
  //char in_blocks[round*512/4+1];
  char in_blocks[3*512/4+1];
  //uint32_t sha256_w[round][64];
  char temp_w[9];
  char temp_s_hash[9];
  BIGD leng_to_hex;
  leng_to_hex = bdNew();

  //initial hash value
  uint32_t sha256_h0[8] =
            {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
             0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

  //working variables
  uint32_t sha256_a_to_h[8];
  uint32_t t1, t2;

  //constants
  uint32_t sha256_constants[64] =
            {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  
  if(debug) printf("in character : %s\n",in);
  //Calculate the necessary 512bit blocks
  //The last block has to be padded with zeros. The last few bits are reserved to specify the 
  //in leght.

  //calculate number of rounds
  temp = 4*strlen(in); //bits
  temp = temp + 1 + 64;	//bits
  temp = temp / 512;
  //round up
  round = (int)temp + 1; 
  if(debug)printf("Number of rounds: %d \n",round);

  //calculate the last few bits in hex (the in lenght)
  bdSetShort(leng_to_hex, 4*strlen(in));
  bdConvToHex(leng_to_hex, s, 9);
  if(debug)printf("in lenght in Hex: %s\n", s);
  if(debug)printf("Number of chars: %i \n", (int)strlen(s));

  //make the complete string 
  for (i=0;i<round*512/4;i++){
    if (i < (int)strlen(in)){   
      in_blocks[i] = in[i];
    }
    if (i == (int)strlen(in)){
      in_blocks[i]='8';
    }
    if (i > (int)strlen(in)){
      in_blocks[i]='0' ; 
    }
  }
  //add the lenght bits in the end
  for (i=0;i<(int)strlen(s);i++){
    in_blocks[round*512/4-i-1] = s[strlen(s)-i-1];
  }
  //print the complete in block
  for (i=0;i<(int)(round*512/4);i++){
    if (debug) printf("%c",in_blocks[i]);
  }
  if (debug) printf("\n");
  temp_w[8]='\0';
  //now, put the string into hex blocks à 32bit
  for (rond_n=0;rond_n<round;rond_n++){
    for (count=0;count<16;count++){
      for (i=0;i<8;i++){
        temp_w[i]=in_blocks[i+(8*count)+(128*rond_n)];
      }
    sha256_w[rond_n][count] = strtoul(temp_w, NULL, 16); 
    if (debug) printf("sha256_w: %08x\n", sha256_w[rond_n][count]);
    
    }
  }
  if (debug) printf("Start Hash calculation...\n");
  for (k=0;k<round;k++){
    for (i=16;i<64;i++){
      SHA256_SCR(i);
      if (debug) printf("sha256_w: %08x\n", sha256_w[k][i]);
    }
    //initialize working variables
    for (i=0;i<8;i++){
      sha256_a_to_h[i] = sha256_h0[i];
    }
    //calculate the matrix
    for (i=0;i<64;i++){
      t1 = sha256_a_to_h[7] + SHA256_F2(sha256_a_to_h[4]) + CH(sha256_a_to_h[4], sha256_a_to_h[5], sha256_a_to_h[6])
      + sha256_constants[i] + sha256_w[k][i];
      t2 = SHA256_F1(sha256_a_to_h[0]) + MAJ(sha256_a_to_h[0], sha256_a_to_h[1], sha256_a_to_h[2]);
      sha256_a_to_h[7] = sha256_a_to_h[6];
      sha256_a_to_h[6] = sha256_a_to_h[5];
      sha256_a_to_h[5] = sha256_a_to_h[4];
      sha256_a_to_h[4] = sha256_a_to_h[3] + t1;
      sha256_a_to_h[3] = sha256_a_to_h[2];
      sha256_a_to_h[2] = sha256_a_to_h[1];
      sha256_a_to_h[1] = sha256_a_to_h[0];
      sha256_a_to_h[0] = t1+t2;
    }
    //hash value after round k
    for (i=0;i<8;i++){
      sha256_h0[i] = sha256_h0[i] + sha256_a_to_h[i]; 	
      if(debug)printf("HashResult: %08x \n", sha256_h0[i]);
    }
  }
  if (debug) printf("The final hash256 Value:\n");
  out[0]='\0';
  temp_s_hash[8]='\0';
  for (i=0;i<8;i++){
    //sprintf(temp_s_hash, "%08lx", sha256_h0[i]);
    sprintf(temp_s_hash, "%08x", sha256_h0[i]);
    strcat(out, temp_s_hash);
    if (debug) printf("%s\n", out);	
  }
  if (debug) printf("%s\n", out);
  /* Free all objects we made */
  bdFree(&leng_to_hex);
}
/************************** HMAC-SHA256 ************************************/
/*fuction to calculate HMAC-SHA256
  input:  rtx XOR REPEATER (plaintext, 16 chars) 
	  kd (key, 128 chars)
	  repeater (if repeater = true: "01" else "00") (2 chars)
  output: H (HMAC, 64 chars)*/

void hmac(char* rtx, char* kd, char* H, char* repeater){
  int i;
  char kd_in[129];
  char inner_k[146];
  char outer_k[193]; //wrong: 161
  char rtx_new[17];
  char xor36[129] = "36363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636363636";
  char xor5c[129] = "5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c";

  for (i=0;i<65;i++){
    kd_in[i]=kd[i];
  }
  //pad key to 128bytes
  strcat(kd_in, "0000000000000000000000000000000000000000000000000000000000000000");
  if(debug) printf("paded kd: %s\n",kd_in);
  //XOR rtx and repeater
  xor_strings(rtx, repeater, rtx_new, sizeof(rtx_new)-1);

  if(debug) printf("Input (new rtx): %s\n",rtx_new);  //xor_val to hex
  //XOR key with 0x36... to generate inner key

  xor_strings(kd_in, xor36, inner_k, 128);

  if(debug) printf("Inner key: %s\n", inner_k);
  //XOR key with 0x5c... to generate outer key

  xor_strings(xor5c, kd_in, outer_k, 128);
  //inner_key || rtx_new
  strcat(inner_k, rtx_new);
  //inner_k[145] = '\0';
  if(debug) printf("Complete input: %s\n", inner_k); 
  //calculate first hash
  sha256(inner_k, H); 
  if(debug) printf("Hash1: %s\n", H); 
  //generate input with inner key
  strcat(outer_k, H);
  //calculate second hash
  sha256(outer_k, H);
  if(debug) printf("Hash2: %s\n", H);
}
