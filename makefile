GPP=g++ -m32
GCC=gcc -m32

OUTFILE="bin/mysql.so"

COMPILE_FLAGS=-c -O3 -fPIC -w -DLINUX -Wall -Isrc/SDK/amx/ -Isrc/ -fpermissive
BOOST_LIB_DIR=./src/boost_lib/

all:
	@echo Compiling..
	@ $(GCC) $(COMPILE_FLAGS) src/SDK/amx/*.c
	@ $(GPP) $(COMPILE_FLAGS) src/SDK/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) src/*.cpp
	@echo Linking..
	@ $(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o -lmysqlclient_r -pthread -lrt $(BOOST_LIB_DIR)libboost_thread.a $(BOOST_LIB_DIR)libboost_system.a $(BOOST_LIB_DIR)libboost_date_time.a
	@echo Done. Binary file is now located in \"$(OUTFILE)\".
	@ rm -f *.o

clean:
	@ rm -f *.o
	@echo Cleaning done.
