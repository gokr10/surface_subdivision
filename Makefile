CC = g++
ifeq ($(shell sw_vers 2>/dev/null | grep Mac | awk '{ print $$2}'),Mac)
	CFLAGS = -g -DGL_GLEXT_PROTOTYPES -I./include/ -I/usr/X11/include -DOSX -O2
	LDFLAGS = -framework GLUT -framework OpenGL \
    	-L"/System/Library/Frameworks/OpenGL.framework/Libraries" \
    	-lGL -lGLU -lm -lstdc++
else
	CFLAGS = -g -DGL_GLEXT_PROTOTYPES -Iglut-3.7.6-bin -O2
	LDFLAGS = -lglut -lGLU
endif
	
RM = /bin/rm -f 
all: main 
main: subdivide.o
	$(CC) $(CFLAGS) -o subdivide subdivide.o $(LDFLAGS)
subdivide.o: subdivide.cpp
	$(CC) $(CFLAGS) -c subdivide.cpp -o subdivide.o
clean: 
	$(RM) *.o subdivide
 


