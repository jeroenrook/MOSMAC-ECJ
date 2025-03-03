cd code/m4ri-20130416
make clean
./configure
make -j

cd ../..
mkdir binary
cd binary

cmake -DCMAKE_BUILD_TYPE=Release -DNOSTATS=ON -DNOZLIB=ON -DSTATICCOMPILE=ON -DNOTESTS=ON ../code/
make -j VERBOSE=1

rm -rf CM*
rm -rf cm*
rm CPackConfig.cmake  CPackSourceConfig.cmake Makefile
cd ../code/m4ri-20130416
make clean
