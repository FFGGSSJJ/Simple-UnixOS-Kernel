/*
 * tab:2
 *
 * main.c - I/O and P3 setup
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Aamir Hasan
 * Version:       1
 * Creation Date: Sun Aug 30 2020
 * Filename:      main.c
 * History:
 *    AH    1    Sun Aug 30 2020
 *        First written.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mystery.h"

#define MAX_LENGTH 500

int import(char* infile, int* a) {
  FILE* f = fopen(infile, "r");
  if (f == NULL) return -1;
  fscanf(f, "%d\n", a);
  fclose(f);
  return 0;
}

void print_array(int32_t* array, int32_t size) {
  int32_t i;

  if (size <= 0) {
    printf("\n");
  }

  for (i = 0; i < size && i < MAX_LENGTH; i++) {
    printf("%d ", array[i]);
  }
  printf("\n");

}

int main(int argc, char** argv) {
  if(argc < 2) {
    printf("Usage: ./mystery <input file>\n");
    return -1;
  }
  
  int32_t x = 0;
  int32_t* y = (int32_t*) calloc(MAX_LENGTH, sizeof(int32_t));
  int32_t out = 0;

  import(argv[1], &x);
  printf("Running C Code\n");
  out = mystery_c(x, y);
  printf("Mystery(%d) = %d\n  Mystery Sequence: ", x, out);
  print_array(y, out);

  printf("Running ASM Code\n");

  out = mystery_asm(x, y);
  printf("Mystery(%d) = %d\n  Mystery Sequence: ", x, out);
  print_array(y, out);

  free(y);
  return 0;
}
