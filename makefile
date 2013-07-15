GPP=g++ -m32
GCC=gcc -m32

OUTFILE="bin/mysql.so"

COMPILE_FLAGS=-c -O3 -w -fPIC -DLINUX -Wall -Isrc/SDK/amx/ -Isrc/ -fpermissive
BOOST_LIB_DIR=./src/boost_lib

all:
	@echo Compiling..
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/date_time/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/system/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/thread/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) $(BOOST_LIB_DIR)/thread/pthread/*.cpp
	@ $(GCC) $(COMPILE_FLAGS) src/SDK/amx/*.c
	@ $(GPP) $(COMPILE_FLAGS) src/SDK/*.cpp
	@ $(GPP) $(COMPILE_FLAGS) src/*.cpp
	@echo Linking..
	@ $(GPP) -O2 -fshort-wchar -shared -o $(OUTFILE) *.o -lmysqlclient_r -pthread -lrt
	@echo Done. Binary file is now located in \"$(OUTFILE)\".
	@ rm -f *.o

clean:
	@ rm -f *.o
	@echo Cleaning done.
