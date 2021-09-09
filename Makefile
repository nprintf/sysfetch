PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man/man1
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
	cp -f ./sysfetch $(BINPREFIX)
	chmod 755 $(BINPREFIX)
	mkdir -p $(MANPREFIX) $(ARTPREFIX)
	cp -f ./sysfetch.1 $(MANPREFIX)/sysfetch.1
	chmod 644 $(MANPREFIX)/sysfetch.1
	sed "s|ARTPATH|$(ARTPREFIX)|g" < ./sysfetch.1 > $(MANPREFIX)/sysfetch.1
	cp -r ./art/* $(ARTPREFIX)/
	chmod 644 $(ARTPREFIX)/*

uninstall:
	rm -f $(BINPREFIX)
	rm -f $(MANPREFIX)/sysfetch.1
	rm -rf $(ARTPREFIX)
