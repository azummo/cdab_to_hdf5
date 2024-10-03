OBJS = cdab_to_hdf5.o eos_hdf5.o
SRCS = $(EVB_OBJS:.o=.cpp)

INCDIR = include
INCLUDE = -I$(INCDIR) -I/usr/include/hdf5/serial/
CFLAGS =
LFLAGS =
LIBS = 

CC = h5c++ -g

CXXFLAGS = $(CFLAGS) $(LFLAGS) $(INCLUDE) $(LIBS) -g

all: cdab_to_hdf5

cdab_to_hdf5: $(OBJS)
	$(CC) -o $@ $(OBJS) $(CXXFLAGS)

clean: 
	-$(RM) core *.o cdab_to_hdf5
