PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
CFLAGS = -Wall

%.o: %.c asciiart.h
	$(CC) -c -o $@ $< $(CFLAGS)

sysfetch: sysfetch.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm sysfetch sysfetch.o

install:
	cp -f sysfetch $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/sysfetch
	mkdir -p  $(MANPREFIX)/man1
	cp -f ./sysfetch.1 $(MANPREFIX)/man1/sysfetch.1
	chmod 644 $(MANPREFIX)/man1/sysfetch.1

uninstall:
	rm -f $(PREFIX)/bin/sysfetch
	rm -f $(MANPREFIX)/man1/sysfetch.1
