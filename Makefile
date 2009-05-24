
CAT	= cat
CP	= cp
DIFF	= diff
ECHO	= echo
INSTALL	= install -c -o root -g wheel
LN	= ln
MKDIR	= mkdir
RM	= rm
SLEEP	= sleep

all:	build
build:	samefile samelink samearchive-lite samearchive
test:	test-samefile test-samelink test-samearchive-lite \
	test-samearchive
install:
	$(INSTALL) -m 0755 samefile samelink samearchive samearchive-lite \
		$(DESTDIR)$(PREFIX)/bin/
	$(INSTALL) -m 0644 samefile.1.gz samelink.1.gz samearchive.1.gz \
		samearchive-lite.1.gz $(DESTDIR)$(PREFIX)/man/man1/

clean:
	@${RM} -rf *.o *core test test2 test.out \
		samefile samelink samearchive-lite samearchive

# ----------------------------------------------------------------------

test-samefile:	test-build samefile
	@${ECHO} "Testing samefile"
	@${ECHO} 'samefile -A:' > test.out
	@${CAT} test.lst | ./samefile -A >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -Z:' >> test.out
	@${CAT} test.lst | ./samefile -Z >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -At:' >> test.out
	@${CAT} test.lst | ./samefile -At >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -Zt:' >> test.out
	@${CAT} test.lst | ./samefile -Zt >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -L:' >> test.out
	@${CAT} test.lst | ./samefile -L >> test.out
	@${ECHO} >> test.out

	@${ECHO} 'samefile -iA:' >> test.out
	@${CAT} test.lst | ./samefile -iA >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -iZ:' >> test.out
	@${CAT} test.lst | ./samefile -iZ >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -iAt:' >> test.out
	@${CAT} test.lst | ./samefile -iAt >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -iZt:' >> test.out
	@${CAT} test.lst | ./samefile -iZt >> test.out
	@${ECHO} >> test.out
	@${ECHO} 'samefile -iL:' >> test.out
	@${CAT} test.lst | ./samefile -iL >> test.out
	@${ECHO} >> test.out

	@${ECHO} 'samefile -ix:' >> test.out
	@${CAT} test.lst | ./samefile -ix >> test.out
	@${DIFF} -u test.out test.samefile

test-samelink: test-build samelink
	@${ECHO} "Testing samelink"
	@${ECHO} 'samelink -A:' > test.out
	@${CAT} test.lst | ./samefile -A | ./samelink -nvA >> test.out 2>&1
	@${ECHO} >> test.out
	@${ECHO} 'samelink -Z:' >> test.out
	@${CAT} test.lst | ./samefile -Z | ./samelink -nvZ >> test.out 2>&1
	@${ECHO} >> test.out
	@${ECHO} 'samelink -At:' >> test.out
	@${CAT} test.lst | ./samefile -At | ./samelink -nvAt >> test.out 2>&1
	@${ECHO} >> test.out
	@${ECHO} 'samelink -Zt:' >> test.out
	@${CAT} test.lst | ./samefile -Zt | ./samelink -nvZt >> test.out 2>&1
	@${ECHO} >> test.out
	@${ECHO} 'samelink -L:' >> test.out
	@${CAT} test.lst | ./samefile -L | ./samelink -nvL >> test.out 2>&1
	@${DIFF} -u test.out test.samelink

test-samearchive-lite: test-build samearchive-lite
	@${ECHO} "Testing samearchive-lite"
	@${CAT} test.lst | ./samearchive-lite test test2 > test.out
	@${DIFF} -u test.out test.samearchive-lite

test-samearchive: test-build samearchive
	@${ECHO} "Testing samearchive"
	@${CAT} test.lst test2.lst | ./samearchive -i test test2 > test.out
	@${DIFF} -u test.out test.samearchive

test-build:
	-@${RM} -rf test test2
	@${MKDIR} test test2
	@${ECHO} "1234567890" > test.out
	@${CP} test.out test/file1a
	@${SLEEP} 1;
	@${CP} test.out test/file3a
	@${SLEEP} 1;
	@${CP} test.out test/file2a
	@${LN} test/file2a test/file2c
	@${LN} test/file2a test/file2b
	@${LN} test/file1a test/file1b
	@${SLEEP} 1;
	@${CP} test.out test2/file1a
	@${SLEEP} 1;
	@${CP} test.out test2/file3a
	@${SLEEP} 1;
	@${CP} test.out test2/file2a
	@${SLEEP} 1;
	@${CP} test.out test2/file2c
	@${SLEEP} 1;
	@${CP} test.out test2/file2b
	@${SLEEP} 1;
	@${CP} test.out test2/file1b

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
