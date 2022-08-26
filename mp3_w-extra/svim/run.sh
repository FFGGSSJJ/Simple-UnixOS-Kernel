make
cp ./to_fsdir/svim ../fsdir/
cd ..
./createfs -i fsdir -o student-distrib/filesys_img
cd student-distrib/
sudo ./debug.sh
cd ../svim
gdb ../student-distrib/stubootimg
