/*
 * tab:2
 *
 * uitilities.c - Helper functions for BST
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
#include "search.h"

node_t* create_node(uint32_t key) {
  // allocate memeory for node
  node_t* node = malloc(sizeof(node_t));

  // set param values
  node->key = key;
  node->right = NULL;
  node->left = NULL;

  return node;  
}

void print(node_t* node) {
  if (node == NULL) {
    return;
  }

  // print order is Root Left Right
  printf("%d ", node->key);
  print(node->left);
  print(node->right);
}

void delete(node_t* node) {
  if (node == NULL) {
    return;
  }
  // remove children before removing the root
  delete(node->left);
  delete(node->right);

  free(node);
}

node_t* insert(node_t* root, uint32_t key) {
	if (root == NULL) {
		return create_node(key);
	}

	if (key < root->key) {
    // insert in left subtree
		root->left = insert(root->left, key);
	} else {
    // insert in right subtree
		root->right = insert(root->right, key);
	}

	return root;
}
