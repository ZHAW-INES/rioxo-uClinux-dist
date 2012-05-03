/* ====================================================================
 * Copyright (c) 2002 Johnny Shelley.  All rights reserved.
 *
 * Bcrypt is licensed under the BSD software license. See the file 
 * called 'LICENSE' that you should have received with this software
 * for details
 * ====================================================================
 */

#include "includes.h"
#include "blowfish.h"

/* from wrapbf.c */
unsigned long BFEncrypt(char **input, char *key, unsigned long sz, 
	BCoptions *options);
unsigned long BFDecrypt(char **input, char *key, char *key2,
	unsigned long sz, BCoptions *options);

/* from keys.c */
char * getkey(int type);
void mutateKey(char **key, char **key2);

/* from rwfile.c */
int getremain(unsigned long sz, int dv);
unsigned long padInput(char **input, unsigned long sz);
unsigned long attachKey(char **input, char *key, unsigned long sz);
unsigned long readfile(char *infile, char **input, struct stat statbuf);
unsigned long writefile(char *outfile, char *output, unsigned long sz, 
	BCoptions options, struct stat statbuf);
int deletefile(char *file, BCoptions options, char *key, struct stat statbuf);

/* from wrapzl.c */
//unsigned long docompress(char **input, unsigned long sz);
//unsigned long douncompress(char **input, unsigned long sz, BCoptions options);

/* from main.c */
BCoptions initoptions(BCoptions options);
int usage(char *name);
int memerror();
int parseArgs(int *argc, char **argv, BCoptions *options);
int assignFiles(char *arg, char **infile, char **outfile, struct stat *statbuf,
        BCoptions *options, char *key);
//int main(int argc, char *argv[]);
int main(void);

/* from endian.c */
void getEndian(unsigned char **e);
uInt32 swapEndian(uInt32 value);
int testEndian(char *input);
int swapCompressed(char **input, unsigned long sz);

