# Create build directory, setup the .gitignore
# file, then configure and build the project.

mkdir build/ && cd build/
echo "*" >> .gitignore
cmake .. && make -j${nproc}
