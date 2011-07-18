OBJECTS=cr-view.o
OUTPUT=cr-view
LDLIBS=-lX11 -lglut -lGLU -lGL -lm -lXext -lXmu

$(OUTPUT): $(OBJECTS)
	cc $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(OUTPUT) $(OBJECTS)

clean:
	rm -f *.o cr-view
