CXX=g++
CXXFLAGS += -O3 #-g

#############INCLUDES
BASIC_ROOTINC=$(shell root-config --cflags)
CXXINCS +=$(BASIC_ROOTINC)

LARLITE_INCDIR=$(shell larlite-config --includes)
CXXINCS +=$(LARLITE_INCDIR)

OPENCV_INCDIR=/uboone/app/users/vgenty/opencv/include
CXXINCS +=-I$(OPENCV_INCDIR)

#############LIBRARIES
BASIC_ROOTLIBS=$(shell root-config --libs)
CXXLIBS += $(BASIC_ROOTLIBS)

LARLITE_LIBDIR=$(shell larlite-config --libs)
CXXLIBS += $(LARLITE_LIBDIR)

OPENCV_LIBDIR=/uboone/app/users/vgenty/opencv/lib
CXXLIBS +=-L$(OPENCV_LIBDIR)

OPENCV_LIBS=-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc
CXXLIBS += $(OPENCV_LIBS)

SWFILE=wire2png.cxx
OWFILE=wire2png.o
BWFILE=wire2png
SDFILE=digit2png.cxx
ODFILE=digit2png.o
BDFILE=digit2png

all: 
	$(CXX) $(CXXFLAGS) $(CXXINCS) -c $(SWFILE) -o $(OWFILE)
	$(CXX) $(CXXLIBS) -o $(BWFILE) $(OWFILE) 
	$(CXX) $(CXXFLAGS) $(CXXINCS) -c $(SDFILE) -o $(ODFILE)
	$(CXX) $(CXXLIBS) -o $(BDFILE) $(ODFILE) 

clean:
	rm -rf $(OWFILE) $(BWFILE)
	rm -rf $(ODFILE) $(BDFILE)

