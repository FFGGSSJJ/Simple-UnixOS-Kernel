make clean
make dep
make 2>&1 | tee makeresult
gdb bootimg