PREFIX = /usr
MANPREFIX = $(PREFIX)/local/share/man/man1/sysfetch.1
ARTPREFIX = $(PREFIX)/share/sysfetch
BINPREFIX = $(PREFIX)/bin/sysfetch
CFLAGS = -Wall

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

sysfetch: sysfetch.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm sysfetch sysfetch.o

install:
	cp -f sysfetch $(BINREFIX)
	chmod 755 $(BINPREFIX)
	mkdir -p $(MANPREFIX) $(ARTPREFIX)
	cp -f ./sysfetch.1 $(MANPREFIX)
	chmod 644 $(MANPREFIX)
	mkdir -p $(ARTPREFIX)
	cp -r ./art/* $(ARTPREFIX)
	chmod 644 $(ARTPREFIX)/*

uninstall:
	rm -f $(BINREFIX)
	rm -f $(MANPREFIX)
	rm -rf $(ARTPREFIX)
