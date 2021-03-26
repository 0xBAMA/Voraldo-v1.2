mkdir build
cmake -S . -B ./build
cd build
make exe
cp ./bin/exe ..
cd ..
rm -r ./build
