# Toggle the following line/comment under windows
LIBS =  -lglut -lGLU -lGL -lGLEW -lm -lgdal
#LIBS =  -lglut32 -lGLU32 -lopengl32 -lglew32 -lm

CFLAGS = -Wall -O3 -mfpmath=sse -msse2 #-march=native -mtune=native
CXXFLAGS = -Wall -O3 -mfpmath=sse -msse2 #-march=native -mtune=native
CPPFLAGS = -I$(INCDIR) -I/include -I.
LDFLAGS = -L/usr/lib -L/lib
LDLIBS = $(LIBS)  
CC = g++
CPP = g++
OBJDIR = obj

CIBLE = dsmviewer
SRCS =  main.cc camera.cc gdal-helper.cc
DEPS = main.h camera.h gdal-helper.h

_OBJS = $(SRCS:.cc=.o)
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.cc $(DEPS)
	$(CPP) -c -o $@ $< $(CPPFLAGS) $(LIBS)

$(CIBLE): $(OBJS) $(DEPS)
	$(CPP) $(LDFLAGS) -o $(CIBLE) $(OBJS) $(LIBS)

.PHONY:
	clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(CIBLE)

