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

unsigned long docompress(char **input, unsigned long sz) {
  unsigned long newsz;
  char *output;

  newsz = sz + (sz *.1) + 13;
  if ((output = malloc(newsz + 1)) == NULL)
    memerror();

  memset(output, 0, newsz + 1);

  //compress((Bytef *) output, &newsz, (const Bytef *) *input,  sz);

  free(*input);
  if ((*input = malloc(newsz)) == NULL)
    memerror();

  memcpy(*input, output, newsz);

  free(output);

  return(newsz);
}

unsigned long douncompress(char **input, unsigned long sz, BCoptions options) {
  char *output;

  if ((output = malloc(options.origsize + 1)) == NULL)
    memerror();

  memset(output, 0, options.origsize + 1);

  //uncompress((Bytef *) output, (unsigned long *) &options.origsize, 
  //	(const Bytef *) *input, sz);

  free(*input);
  if ((*input = malloc(options.origsize)) == NULL)
    memerror();

  memcpy(*input, output, options.origsize);

  free(output);

  return(options.origsize);
}
