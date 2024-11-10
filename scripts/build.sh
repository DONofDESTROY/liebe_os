export PREFIX="/usr/local/i386elfgcc"
export TARGET=i386-elf

export PATH="$PATH:$PREFIX/bin"
cd ..
make all
