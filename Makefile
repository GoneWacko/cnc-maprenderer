.PHONY: all clean

EXECUTABLE := cnc-maprenderer
OBJS := cnc-maprenderer.o map.o renderer.o shp.o output.o ini/ini.o ini/parser.o ini/tokenizer.o vfs/vfs.o vfs/mixarchive.o vfs/dirarchive.o

override CFLAGS += -Wall -g -std=c99 `pkg-config --cflags glib-2.0`
override LDFLAGS += `pkg-config --libs glib-2.0` /usr/lib/pnglite.o -lz

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)

clean:
	rm -vf $(EXECUTABLE) $(OBJS)
