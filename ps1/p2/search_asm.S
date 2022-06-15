/*
 * tab:2
 *
 * search_asm.S - Implementation of Assembly Recursive DFS
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
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Author:        Aamir Hasan
 * Version:       1
 * Creation Date: Fri Aug 30 2020
 * Filename:      search_asm.S
 * History:
 *    AS    1    Fri Aug 30 2020
 *        First written.
 */

.data
  KEY   = 0
  LEFT  = 4
  RIGHT = 8

.text
.global search_asm

# Search ASM (Assembly)
# Searches for an element in a BST
# Declaration is in search.h
#
# Registers:
#   eax - Return Value and Store the value of root
#   ebx - Store the value of key
search_asm:
  pushl %ebp
  movl %esp, %ebp 

  #--- YOUR CODE HERE ---
  pushl %ebx
  movl 8(%ebp), %eax    # move 'root' into EAX
  cmpl $0, %eax         # check if 'root' is NULL
  je RES1
  movl 12(%ebp), %ebx   # move 'key' into EBX
  cmpl (%eax), %ebx     # compare M[root] and key
  je RES0  # key == M[root]
  jl LEFT_NODE  # key <  M[root]

RIGHT_NODE:
  pushl %ebx
  pushl 8(%eax)   # push *(root+8) == root -> right
  call search_asm
  addl $8, %esp
  jmp FUNC_RET

LEFT_NODE:
  pushl %ebx
  pushl 4(%eax)   # push *(root+4) == root -> left
  call search_asm
  addl $8, %esp
  jmp FUNC_RET

RES0:
  movl $0, %eax
  jmp FUNC_RET
RES1:
  movl $-1, %eax
FUNC_RET:
  popl %ebx
  #----------------------
  leave
  ret
