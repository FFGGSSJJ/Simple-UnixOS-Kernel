# Problem Set 1 - ECE391 Spring 2022

### Logistics
Problem Set 1 is due **Monday 1/31 at 11:59:59 PM** in the master branch. Only one
person per group should have a `partners.txt` with **all** the netids of your partners on separate lines.
An example `partners.txt` would look like this if `naiyinj2` was submitting the group's solution.
> naiyinj2
>
> henryh2
>
> xiangl5
>
> yamsani2


Note that if your netid is part of more than one `partners.txt`, you will recieve a 0 for the problem set.

**You can attempt this problem set on the Class VM (devel) or on any EWS Linux computer.**

### Problem 1: GNU Debugger (GDB) (5 pt)
Please write the command(s) you should use to achieve the following tasks in GDB.
1. Show the value of variable "test" in hex format.
2. Show the top four bytes on your stack word-by-word, e.g. it should look something like this "0x0102 0x0304", NOT "0x01020304".
3. Check all register values.
4. Set a breakpoint at "ece.c" line 391.
5. Connect to the test\_(no)debug vm in the lab setup.

Please write your solution in p1\_soln.txt with answers to each question on a separate line. For example, your p1\_soln.txt should be of the form
> answer 1
>
> answer 2 
>
> ...
>
> answer 5

### Problem 2: C to Assembly Translation (10 pt)
Write x86 assembly code for the body of the `search` function found in `search.c`. Make sure to set up and tear down the stack frame as well as save and restore any callee/caller-saved registers yourself (if you use them). Assume caller-saved registers are saved prior to the `search_asm` function being called for the first time. Include comments (but don't overdo it!) in your assembly code to show the correspondence between the C code and your x86 code.

Also note that:
1. The `search_asm` function in `search_asm.S` is partially filled out for you, your job is to complete the function.
2. Please make sure your code and comments are easy to read. We reserve the right to take points off if your answer is too hard to follow.
3. You must synthesize your answer without the help of a computer. For example, you may not write the C code and then use the compiler to disassemble it. If you are caught doing this, you will receive a 0.
4. You must **translate** your code, a functionally equivalent algorithm with a different structure will recieve a 0.
5. You must write your solution in `p2/search_asm.S` and submit it through gitlab.

To build the code (no debug flag):
`$ make clean && make`

To run the code:
`$ ./search ./input_small.txt`

To build the code (debug flag):
`$ make clean && make debug`

To run the code (debug):
`$ gdb --args ./search ./input_small.txt`

### Problem 3: Assembly to C Translation (10 pt)
Write a C function equivalent to the x86 assembly function, `mystery_asm` found in `mystery_asm.S`.

1. Please make sure your code and comments are easy to read. We reserve the right to take points off if your answer is too hard to follow.
2. You must **translate** your code, a functionally equivalent algorithm with a different structure will recieve a 0.
3. You must write your solution in `p3/mystery.c` and submit it through gitlab.

To build the code (no debug flag):
`$ make clean && make`

To run the code:
`$ ./mystery ./input_1.txt`

To build the code (debug flag):
`$ make clean && make debug`

To run the code (debug):
`$ gdb --args ./mystery ./input_1.txt`

