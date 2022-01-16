GCC = g++       

WHERE=/home/shimada/NEWAGE/LTARS/Decoder0.2_cmake

INCLUDE=$(WHERE)/source//include
LIBS = -L/opt/FRCvme/usr/lib -lvui
CLIB = -lnsl -lm -lc
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs --cflags)

RLIB =   -I${ROOTSYS}/include $(ROOTLIBS) $(ROOTGLIBS) -I${INCLUDE}

LD_PATH =$(WHERE)/bin
SRC=$(WHERE)/source/


Event:
	rootcint -f ${SRC}/$(@)/$(@)Dict.cxx -c ${INCLUDE}/$(@).h ${INCLUDE}/$(@)LinkDef.h
	$(CXX) ${RLIB} ${CLIB} `root-config --cflags --libs` -shared ${SRC}/Event/$(@)Dict.cxx ${SRC}/Event/$(@).cxx -o lib$(@).so -fPIC
	mv ./lib$(@).so ${LD_PATH}



clean:
	rm -f *.o
	rm -f *~

