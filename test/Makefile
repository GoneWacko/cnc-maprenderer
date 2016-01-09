.PHONY: all clean

ALL_TESTS := vfs_test

vfs_test_OBJS := vfs_test.o ../vfs/vfs.o ../vfs/mixarchive.o ../vfs/memfile.o

CFLAGS := -Wall -g -std=c99 `pkg-config --cflags glib-2.0`
LDFLAGS := `pkg-config --libs glib-2.0`

all: $(ALL_TESTS)

vfs_test: $(vfs_test_OBJS)

clean:
	rm -vf $(ALL_TESTS) $(vfs_test_OBJS)
