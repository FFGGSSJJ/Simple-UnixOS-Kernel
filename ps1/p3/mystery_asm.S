
/*
 * tab:2
 *
 * mystery_asm.S - Assembly mystery
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
 * Filename:      mystery_asm.S
 * History:
 *    AH    1    Sun Aug 30 2020
 *        First written.
 */

.data

.text
.global mystery_asm

# mystery (Assembly)
# 
# Registers:
#   eax - Return Value
#   ebx - x
#   ecx - y
#   edi - used for computation

mystery_asm:
  # setup stackframe
  pushl %ebp
  movl %esp, %ebp

  pushl %ebx
  pushl %edi

  xorl %eax, %eax

  movl 8(%ebp), %ebx
  movl 12(%ebp), %ecx

  cmpl $0, %ebx
  jle invalid_input

op_loop:
  movl %ebx, (%ecx, %eax, 4)
  incl %eax 

  cmpl $1, %ebx
  je finish

  movl %ebx, %edi
  shrl $1, %edi
  jb op2

op1:
  movl %edi, %ebx
  
  jmp op_loop

op2:
  movl %ebx, %edi
  imul $3, %edi
  incl %edi

  movl %edi, %ebx
  jmp op_loop

invalid_input:
  movl $-1, %eax

finish:
  popl %edi
  popl %ebx

  leave
  ret
