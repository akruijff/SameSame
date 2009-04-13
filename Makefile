
INSTALL     = /usr/bin/install -c -o root -g wheel

all:	build

build:		samefile samelink samearchive-lite samearchive
test:		test-samefile test-samelink test-samearchive-lite \
		test-samearchive
install:	install-samefile install-samelink install-samearchive-lite \
			install-samearchive
clean:
	@rm -rf *.o *core find test test2 test.out \
		samefile samelink samearchive-lite samearchive

# ----------------------------------------------------------------------
install-samefile:			test-samefile
	$(INSTALL) -m 0755 samefile $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 samefile.1.gz $(DESTDIR)$(PREFIX)/man/man1/
install-samelink:			test-samelink
	$(INSTALL) -m 0755 samelink $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 samelink.1.gz $(DESTDIR)$(PREFIX)/man/man1/
install-samearchive:		test-samearchive
	$(INSTALL) -m 0755 samearchive $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 samearchive.1.gz $(DESTDIR)$(PREFIX)/man/man1/
install-samearchive-lite:	test-samearchive-lite
	$(INSTALL) -m 0755 samearchive-lite $(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 samearchive-lite.1.gz $(DESTDIR)$(PREFIX)/man/man1/

# ----------------------------------------------------------------------

test-samefile:	test-build samefile
	@echo "Testing samefile"
	@echo 'samefile -A:' > test.out
	@find test | ./samefile -A >> test.out
	@echo >> test.out
	@echo 'samefile -Z:' >> test.out
	@find test | ./samefile -Z >> test.out
	@echo >> test.out
	@echo 'samefile -At:' >> test.out
	@find test | ./samefile -At >> test.out
	@echo >> test.out
	@echo 'samefile -Zt:' >> test.out
	@find test | ./samefile -Zt >> test.out
	@echo >> test.out
	@echo 'samefile -L:' >> test.out
	@find test | ./samefile -L >> test.out
	@echo >> test.out

	@echo 'samefile -iA:' >> test.out
	@find test | ./samefile -iA >> test.out
	@echo >> test.out
	@echo 'samefile -iZ:' >> test.out
	@find test | ./samefile -iZ >> test.out
	@echo >> test.out
	@echo 'samefile -iAt:' >> test.out
	@find test | ./samefile -iAt >> test.out
	@echo >> test.out
	@echo 'samefile -iZt:' >> test.out
	@find test | ./samefile -iZt >> test.out
	@echo >> test.out
	@echo 'samefile -iL:' >> test.out
	@find test | ./samefile -iL >> test.out
	@echo >> test.out

	@echo 'samefile -ix:' >> test.out
	@find test | ./samefile -ix >> test.out
	@diff -u test.out test.samefile

test-samelink: test-build samelink
	@echo "Testing samelink"
	@echo 'samelink -A:' > test.out
	@find test | ./samefile -A | ./samelink -nvA >> test.out 2>&1
	@echo >> test.out
	@echo 'samelink -Z:' >> test.out
	@find test | ./samefile -Z | ./samelink -nvZ >> test.out 2>&1
	@echo >> test.out
	@echo 'samelink -At:' >> test.out
	@find test | ./samefile -At | ./samelink -nvAt >> test.out 2>&1
	@echo >> test.out
	@echo 'samelink -Zt:' >> test.out
	@find test | ./samefile -Zt | ./samelink -nvZt >> test.out 2>&1
	@echo >> test.out
	@echo 'samelink -L:' >> test.out
	@find test | ./samefile -L | ./samelink -nvL >> test.out 2>&1
	@diff -u test.out test.samelink

test-samearchive-lite: test-build samearchive-lite
	@echo "Testing samearchive-lite"
	@find test | ./samearchive-lite test test2 > test.out
	@diff -u test.out test.samearchive-lite

test-samearchive: test-build samearchive
	@echo "Testing samearchive"
	@find test test2 | ./samearchive -i test test2 > test.out
	@diff -u test.out test.samearchive

test-build:
	@-rm -rf test test2
	@mkdir test test2
	@echo "1234567890" > test.out
	@cp test.out test/file1a
	@sleep 1;
	@cp test.out test/file3a
	@sleep 1;
	@cp test.out test/file2a
	@ln test/file2a test/file2c
	@ln test/file2a test/file2b
	@ln test/file1a test/file1b
	@sleep 1;
	@cp test.out test2/file1a
	@sleep 1;
	@cp test.out test2/file3a
	@sleep 1;
	@cp test.out test2/file2a
	@sleep 1;
	@cp test.out test2/file2c
	@sleep 1;
	@cp test.out test2/file2b
	@sleep 1;
	@cp test.out test2/file1b
	@find test > find

# ----------------------------------------------------------------------

samefile: main.o templates.o holder.o sizegroup.o filegroup.o filename.o \
		printhardlinked.o stats.o write2disk.o storage.o \
		matchmatrix.o toolkit.o hash.o
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o samefile samefile.cpp main.o \
		templates.o holder.o sizegroup.o filegroup.o filename.o \
		printhardlinked.o stats.o write2disk.o storage.o \
		matchmatrix.o toolkit.o hash.o
samelink: toolkit.o
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o samelink samelink.cpp \
		toolkit.o
samearchive: main.o templates.o holder.o sizegroup.o filegroup.o filename.o \
		printhardlinked.o stats.o write2disk.o storage.o \
		matchmatrix.o toolkit.o hash.o
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o samearchive samearchive.cpp main.o \
		templates.o holder.o sizegroup.o filegroup.o filename.o \
		printhardlinked.o stats.o write2disk.o storage.o \
		matchmatrix.o toolkit.o hash.o

samearchive-lite: toolkit.o hash.o
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o samearchive-lite samearchive-lite.cpp \
		toolkit.o hash.o

main.o:			main.cpp holder.o stats.o printhardlinked.o \
			write2disk.o storage.o toolkit.o
templates.o:		templates.cpp container.o filename.o filegroup.o \
			sizegroup.o
holder.o:		holder.cpp sizegroup.o matchmatrix.o toolkit.o hash.o
sizegroup.o:		sizegroup.cpp matchmatrix.o filegroup.o storage.o \
			toolkit.o hash.o
stats.o:		stats.cpp
write2disk.o:		write2disk.cpp storage.o
storage.o:		storage.cpp filegroup.o filename.o toolkit.o
filegroup.o:		hash.o toolkit.o filename.o
filename.o:		filename.cpp
container.o:		container.cpp toolkit.o
matchmatrix.o:		matchmatrix.cpp
toolkit.o:		toolkit.cpp
hash.o:			hash.cpp
