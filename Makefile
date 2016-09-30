CXX=clang++
CXXFLAGS += -g#-O3 

#############INCLUDES
BASIC_ROOTINC=$(shell root-config --cflags)
CXXINCS +=$(BASIC_ROOTINC)

LARLITE_INCDIR=$(shell larlite-config --includes)
CXXINCS +=$(LARLITE_INCDIR)

OPENCV_INCDIR=/usr/local/include
CXXINCS +=-I$(OPENCV_INCDIR)

#############LIBRARIES
BASIC_ROOTLIBS=$(shell root-config --libs)
CXXLIBS += $(BASIC_ROOTLIBS)

LARLITE_LIBDIR=$(shell larlite-config --libs)
CXXLIBS += $(LARLITE_LIBDIR)

OPENCV_LIBDIR=/usr/local/lib
CXXLIBS +=-L$(OPENCV_LIBDIR)

OPENCV_LIBS=-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc
CXXLIBS += $(OPENCV_LIBS)

SFILE=wire2png.cxx
OFILE=wire2png.o
BFILE=wire2png

all: 
	$(CXX) $(CXXFLAGS) $(CXXINCS) -c $(SFILE) -o $(OFILE)
	$(CXX) $(CXXLIBS) -o $(BFILE) $(OFILE) 

clean:
	rm -rf $(OFILE) $(BFILE)
