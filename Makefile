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


$(ODIR)/%.o: $(SRCDIR)/%.c
		@echo "Creating object " $@
		$(CC) $(CCFLAGS) -c -o $@ $<

#all: main mySpec myMatches myHash

main: $(ODIR)/main.o $(ODIR)/mySpec.o $(ODIR)/myHash.o
		@echo "Creating main"
		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

#mySpec: $(ODIR)/mySpec.o $(ODIR)/main.o
#		@echo "Creating mySpec"
#		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
#myMatches: $(ODIR)/myMatches.o $(ODIR)/main.o $(ODIR)/mySpec.o
#		@echo "Creating myMatches"
#		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^
#myHash: $(ODIR)/myHash.o $(ODIR)/main.o $(ODIR)/mySpec.o
#		@echo "Creating myHash"
#		$(CC) $(CCFLAGS) -o $(BDIR)/$@ $^

clean:
		@echo "Cleaning up..."
		$(RM) $(ODIR)/*
		$(RM) $(BDIR)/*