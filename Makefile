PREFIX = /usr
MANPREFIX = $(PREFIX)/local/share/man/man1
ARTPREFIX = $(PREFIX)/share/sysfetch
BINPREFIX = $(PREFIX)/bin
CFLAGS = -Wall

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

sysfetch: sysfetch.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm sysfetch sysfetch.o

install:
	cp -f sysfetch $(BINREFIX)/sysfetch
	chmod 755 $(BINPREFIX)/sysfetch
	mkdir -p $(MANPREFIX) $(ARTPREFIX)
	cp -f ./sysfetch.1 $(MANPREFIX)
	chmod 644 $(MANPREFIX)/sysfetch.1
	mkdir -p $(ARTPREFIX)
	cp -r ./art/* $(ARTPREFIX)
	chmod 644 $(ARTPREFIX)/*

uninstall:
	rm -f $(BINREFIX)/sysfetch
	rm -f $(MANPREFIX)/sysfetch.1
	rm -rf $(ARTPREFIX)
