.PHONY: clean
CC = gcc
CCFLAGS = -Wall -g3
LIBS = -lm
RM = rm -f

BDIR = bin
SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c)
IDIR = include
DEPS = $(wildcard $(IDIR)/*.h)
ODIR = build
TSTSDIR = tests
TSTS = $(wildcard $(TSTSDIR)/*.c)
ARGS = ""


$(ODIR)/%.o: $(SRCDIR)/%.c
		@echo "\nCreating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(TSTSDIR)/%.c
		@echo "\nCreating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

#-------------------------------------------------------------------------#

all: dirs clean tests main run

#-------------------------------------------------------------------------#

main: $(ODIR)/main.o $(ODIR)/mySpec.o $(ODIR)/myHash.o $(ODIR)/myMatches.o $(ODIR)/functs.o $(ODIR)/boWords.o $(ODIR)/mbh.o $(ODIR)/logistic.o
		@echo "\nCreating main"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^ $(LIBS)

run:	bin/main
        ifeq ($(ARGS),"")
		./bin/main -o matches_log_DEF
        else
		./bin/main $(ARGS)
        endif

valgrind: main
		  valgrind --leak-check=full ./bin/main -o matches_log_DEF

#-------------------------------------------------------------------------#

all_tests:  $(ODIR)/myHash_test.o  $(ODIR)/myMatchesList_test.o $(ODIR)/myMatchesNegsList_test.o $(ODIR)/mySpec_test.o $(ODIR)/myMatches.o $(ODIR)/mySpec.o  $(ODIR)/myHash.o
			# ADD $(ODIR)/logistic_test.o $(ODIR)/logistic.o $(ODIR)/functs.o $(ODIR)/boWords.o $(ODIR)/mbh.o
			@echo "\nCreating Unit Testing Files"
			$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^ $(LIBS)

tests:	dirs all_tests
		@echo "\nRunning Unit Testing.."
		./bin/all_tests

#-------------------------------------------------------------------------#

dirs:
		@echo "\nCreating Dirs.."
		mkdir -p $(BDIR)
		mkdir -p $(ODIR)

#-------------------------------------------------------------------------#

medium_labels: main
		./bin/main -o matches_log_MeLbls -l m

#-------------------------------------------------------------------------#

clean:
		@echo "\nCleaning up..."
		$(RM) $(ODIR)/*
		$(RM) $(BDIR)/*
		rm -r -f Outputs
