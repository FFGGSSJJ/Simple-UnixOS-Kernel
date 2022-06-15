/*
 * tab:2
 *
 * main.c - I/O and BST problem setup
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
 * Creation Date: Fri Aug 30 2020
 * Filename:      main.c
 * History:
 *    AS    1    Fri Aug 30 2020
 *        First written.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "search.h"

node_t* import_bst(char* fname, uint32_t* find) {
  FILE* f = fopen(fname, "r");
  uint32_t i, key, num_elements;
  node_t* root = NULL;

  fscanf(f, "%d %d\n", &num_elements, find);
  
  for (i = 0; i < num_elements; i++) {
    fscanf(f, "%d\n", &key);
    root = insert(root, key);
  }

  fclose(f);
  return root;
}

int main(int argc, char** argv) {
  if(argc < 2) {
    printf("Usage: ./search <input_maze>\n");
    return -1;
  }
  node_t* root = NULL;
  uint32_t find = 0;
  
  root = import_bst(argv[1], &find);
  printf("Reading Tree:\n");
  print(root);

  printf("\nBeginning C Search\n");
  printf("search_c find %d: %d\n", find, search_c(root, find));


  printf("Beginning ASM Search\n");
  printf("search_asm find %d: %d\n", find, search_asm(root, find));
  
  delete(root);
  return 0;
}
