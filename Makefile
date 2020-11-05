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

# LES = $(SRC)/%.c

$(ODIR)/%.o: $(SRCDIR)/%.c
		@echo "Creating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

$(TSTSDIR)/%.o: $(TSTSDIR)/%.c
		@echo "Creating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

all: clean tests_ main run

main: $(ODIR)/main.o $(ODIR)/mySpec.o $(ODIR)/myHash.o $(ODIR)/myMatches.o $(ODIR)/functs.o
		@echo "Creating main"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

# mySpec: $(ODIR)/mySpec.o $(ODIR)/main.o
# 		@echo "Creating mySpec"
# 		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
# myMatches: $(ODIR)/myMatches.o $(ODIR)/main.o $(ODIR)/mySpec.o
# 		@echo "Creating myMatches"
# 		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
# myHash: $(ODIR)/myHash.o $(ODIR)/main.o $(ODIR)/mySpec.o
# 		@echo "Creating myHash"
# 		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

tests_:	
		@echo "Creating tests_run .."
# 		$(CC) $(CCFLAGS) -o run_tests $^

# 		@echo "Running Unit Testing .."
# 		../tests/run_tests

clean:
		@echo "Cleaning up..."
		$(RM) $(ODIR)/*
		$(RM) $(BDIR)/*
		$(RM) $(TSTSDIR)/*.o
		rm -r -f Outputs


run:	bin/main
		time ./bin/main -o matches_log