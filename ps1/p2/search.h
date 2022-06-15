/*
 * tab:2
 *
 * search.h - Search function & helper function declarations
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
 * Filename:      search.c
 * History:
 *    AS    1    Fri Aug 30 2020
 *        First written.
 */

#ifndef SEARCH_H
#define SEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct node_t {
	uint32_t key;
	struct node_t* left;
	struct node_t* right;
} node_t;

// create_node
// 		Creates a new node with the associated key and returns a pointed to the node
extern node_t* create_node(uint32_t key);

// print
// 		Prints the pre order traversal of the tree
extern void print(node_t* node);

// delete
// 		Deletes the tree. Frees all memory used by the tree
extern void delete(node_t* node);

// insert
// 		Inserts an element into the tree at its right place
extern node_t* insert(node_t* root, uint32_t key);

// search_c
//   searches for an element in the BST recursively
//   Returns:
//     0 - found element
//     -1 - element not found
extern int32_t search_c(node_t* root, uint32_t key);

// search_asm
//   Same as search_c
// TODO: Implement this function in x86 Assembly in search_asm.S
extern int32_t search_asm(node_t* root, uint32_t key);

#endif
