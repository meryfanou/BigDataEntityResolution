.PHONY: clean
CC = gcc
CCFLAGS = -Wall -g3
RM = rm -f

BDIR = bin
SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c)
IDIR = include
DEPS = $(wildcard $(IDIR)/*.h)
ODIR = build
TSTSDIR = tests
TSTS = $(wildcard $(TSTSDIR)/*.c)


$(ODIR)/%.o: $(SRCDIR)/%.c
		@echo "\nCreating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(TSTSDIR)/%.c
		@echo "\nCreating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

#-------------------------------------------------------------------------#

all: clean tests main run

main: $(ODIR)/main.o $(ODIR)/mySpec.o $(ODIR)/myHash.o $(ODIR)/myMatches.o $(ODIR)/functs.o
		@echo "\nCreating main"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

run:	bin/main
		./bin/main -o matches_log

valgrind: main
		  valgrind ./bin/main -o matches_log

#-------------------------------------------------------------------------#

tests: myMatchesList_test mySpec_test myHash_test

myMatchesList_test: $(ODIR)/MatchesList_test.o $(ODIR)/myMatches.o $(ODIR)/mySpec.o
					@echo "\nCreating myMatchesList_test"
					$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
					@echo "\nRunning Unit Testing for myMatches"
					./bin/myMatchesList_test

mySpec_test: $(ODIR)/mySpec_test.o $(ODIR)/mySpec.o $(ODIR)/myMatches.o
			 @echo "\nCreating mySpec_test"
			 $(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
			 @echo "\nRunning Unit Testing for mySpec"
			 ./bin/mySpec_test

myHash_test: $(ODIR)/myHash_test.o $(ODIR)/myHash.o $(ODIR)/mySpec.o
			 @echo "\nCreating myHash_test"
			 $(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
			 @echo "\nRunning Unit Testing for myhash"
			 ./bin/myHash_test

#-------------------------------------------------------------------------#

clean:
		@echo "\nCleaning up..."
		$(RM) $(ODIR)/*
		$(RM) $(BDIR)/*
		#$(RM) $(TSTSDIR)/*.o
		rm -r -f Outputs
