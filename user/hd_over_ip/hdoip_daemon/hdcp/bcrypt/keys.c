/* ====================================================================
 * Copyright (c) 2002 Johnny Shelley.  All rights reserved.
 *
 * Bcrypt is licensed under the BSD software license. See the file 
 * called 'LICENSE' that you should have received with this software
 * for details
 * ====================================================================
 */

#include "includes.h"
#include "defines.h"
#include "functions.h"
#include "hoi_drv_user.h"

char * getkey(int type){
  char *key, *key2, *ch;
  char key_str[33]="";
  char temp[9];
  int i;
  uint32_t keys_from_HW[4];

#ifndef WIN32	/* Win32 doesn't have termios.h */
  struct termios term, termsave;

  tcgetattr(fileno(stdin), &termsave);
  term = termsave;
  term.c_lflag &= ~ (ECHO | ECHOE | ECHOK | ECHONL);
  tcsetattr(fileno(stdin), TCSANOW, &term);
#endif

  if ((key = malloc(MAXKEYBYTES + 2)) == NULL)
    memerror();

  memset(key, 0, MAXKEYBYTES + 2);

  //fprintf(stderr, "Encryption key:");
  //fgets(key, MAXKEYBYTES + 1, stdin);

  /* blowfish requires 32 bits, I want 64. deal it is still tacked on */
 /* while (strlen(key) < 9 && type == ENCRYPT) {	
    fprintf(stderr, "Key must be at least 8 characters\n");
    memset(key, 0, MAXKEYBYTES + 2);
    fprintf(stderr, "Encryption key:");
    fgets(key, MAXKEYBYTES + 1, stdin);
  }*/

  // Read keys to decrypt flash from HW

  hoi_drv_hdcp_get_key(keys_from_HW);
  //convert keys to string
  for (i=3;i>-1;i--){
	  sprintf(temp,"%08x",keys_from_HW[i]);
	  strcat(key_str, temp);
  }
  //printf("THE KEY: %s", key_str); //SECRET VALUE, SHOW ONLY TO DEBUG
   for (i=0;i<33;i++){
   *(key + i)=key_str[i];
   }

 /* if (memchr(key, (char) 10, MAXKEYBYTES + 1) == NULL) {
    while (fread(overflow, 1, 1, stdin) > 0) {
      if (memcmp(overflow, "\n", 1) == 0)
        break;
    }
  }*/

  if (type == ENCRYPT) {
    if ((key2 = malloc(MAXKEYBYTES + 2)) == NULL)
      memerror();

    memset(key2, 0, MAXKEYBYTES + 2);
    fprintf(stderr, "\nAgain:");
    fgets(key2, MAXKEYBYTES + 1, stdin);
  
    if (strcmp(key, key2)) {
      fprintf(stderr, "\nKeys don't match!\n");
#ifndef WIN32	/* Win32 doesn't have termios.h */
      tcsetattr(fileno(stdin), TCSANOW, &termsave);
#endif
      exit(1);
    }
    memset(key2, 0, strlen(key2));
    free(key2);
  }

  if ((ch = memchr(key, (char) 10, strlen(key))) != NULL)
    memset(ch, 0, 1);

#ifndef WIN32	/* Win32 doesn't have termios.h */
  tcsetattr(fileno(stdin), TCSANOW, &termsave);
#endif

  fprintf(stderr, "\n");

  return(key);
}

void mutateKey(char **key, char **key2) {
  uInt32 L, R, l, r;
  BLOWFISH_CTX ctx;
  char *newkey, *newkey2;
  int i, j;
  j = sizeof(uInt32);

  Blowfish_Init(&ctx, *key, strlen(*key));

  memcpy(&L, *key, j);
  memcpy(&R, *key+j, j);
  memset(*key, 0, MAXKEYBYTES + 1);

  l = L;
  r = R;

  if ((*key2 = malloc(MAXKEYBYTES + 1)) == NULL)
    memerror();
  if ((newkey = malloc(MAXKEYBYTES + 1)) == NULL)
    memerror();
  if ((newkey2 = malloc(MAXKEYBYTES + 1)) == NULL)
    memerror();

  memset(*key2, 0, MAXKEYBYTES + 1);
  memset(newkey, 0, MAXKEYBYTES + 1);
  memset(newkey2, 0, MAXKEYBYTES + 1);

  for (i=0; i < MAXKEYBYTES; i+=(j*2)) {
    Blowfish_Encrypt(&ctx, &L, &R);
    memcpy(newkey+i, &L, j);
    memcpy(newkey+i+j, &R, j);
  }

  for (i=0; i < MAXKEYBYTES; i+=(j*2)) {
    l = swapEndian(l);
    r = swapEndian(r);

    Blowfish_Encrypt(&ctx, &l, &r);

    l = swapEndian(l);
    r = swapEndian(r);

    memcpy(newkey2+i, &l, j);
    memcpy(newkey2+i+j, &r, j);
  }

  memcpy(*key, newkey, MAXKEYBYTES);
  memcpy(*key2, newkey2, MAXKEYBYTES);
  free(newkey);
}
