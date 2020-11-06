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
		@echo "Creating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(TSTSDIR)/%.c
		@echo "Creating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

#-------------------------------------------------------------------------#

all: clean main run

main: $(ODIR)/main.o $(ODIR)/mySpec.o $(ODIR)/myHash.o $(ODIR)/myMatches.o $(ODIR)/functs.o
		@echo "Creating main"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

run:	bin/main
		time ./bin/main -o matches_log

#-------------------------------------------------------------------------#

tests: MatchesList_test mySpec_test
run_tests: run_MatchesList_test run_mySpec_test

MatchesList_test: $(ODIR)/MatchesList_test.o $(ODIR)/myMatches.o $(ODIR)/mySpec.o
		@echo "Creating MatchesList_test"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

run_MatchesList_test:	bin/MatchesList_test
						time ./bin/MatchesList_test

mySpec_test: $(ODIR)/mySpec_test.o $(ODIR)/mySpec.o $(ODIR)/myMatches.o
		@echo "Creating mySpec_test"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

run_mySpec_test:	bin/mySpec_test
					time ./bin/mySpec_test

#-------------------------------------------------------------------------#

clean:
		@echo "Cleaning up..."
		$(RM) $(ODIR)/*
		$(RM) $(BDIR)/*
		rm -r -f Outputs
