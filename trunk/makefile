GPP=g++ -m32
GCC=gcc -m32

OUTFILE="bin/mysql.so"

COMPILE_FLAGS=-c -O3 -fPIC -w -DLINUX -Wall -Isrc/SDK/amx/ -Isrc/ -fpermissive

all:
	@echo Compiling..
	@ $(GCC) $(COMPILE_FLAGS) src/SDK/amx/*.c
	@ $(GPP) $(COMPILE_FLAGS) src/SDK/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) src/*.cpp
	@echo Linking..
	@ $(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o -L/usr/lib/mysql -lmysqlclient_r -pthread ./src/boost_lib/libboost_thread.a
	@echo Done. Binary file is now located in \"$(OUTFILE)\".
	@ rm -f *.o

clean:
	@ rm -f *.o
	@echo Cleaning done.
