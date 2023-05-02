OBJDIR = objs
OBJS = $(addprefix $(OBJDIR)/,main.o audio_player.o supportLib.o pbPlots.o speech_to_text.o)
GTKCFLAGS = $( pkg-config --cflags gtk4 ) -std=c99 -pedantic-errors
GTKLIBS = $( pkg-config --libs gtk4 )
PALIBS = libportaudio.a -lrt -lm -lasound -ljack -pthread
GTKCFLAGS = $(shell pkg-config --cflags gtk4 )
GTKLIBS = $(shell pkg-config --libs gtk4 )
PLOTLIBS = -lm 

.PHONY: clean

main: $(OBJS)
	gcc $^ -o main $(PALIBS) $(GTKLIBS) $(PLOTLIBS) -lcurl

$(OBJDIR)/main.o: src/main.c
	gcc -c -I include $< -o $@ $(GTKCFLAGS)

$(OBJDIR)/audio_player.o: include/audio_player.c include/audio_player.h 
	gcc -c $< -o $@

$(OBJDIR)/speech_to_text.o: include/speech_to_text.c include/speech_to_text.h 
	gcc -c $< -o $@

$(OBJDIR)/pbPlots.o: include/pbPlots.c include/pbPlots.h 
	gcc -c $< -o $@ -std=c99 -O3 -march=native

$(OBJDIR)/supportLib.o: include/supportLib.c include/supportLib.h 
	gcc -c $< -o $@ -std=c99 -O3 -march=native

clean:
	rm $(OBJS) -v
