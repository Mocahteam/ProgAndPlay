SRC = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJ = $(SRC:.cpp=.o) # replaces the .cpp from SRC with .o
EXE = MyParserTest.exe

CC = g++
CFLAGS = -Wall -g -std=c++11
#CFLAGS = -Wall -g

ifeq ($(OS),Windows_NT)
LDFLAGS = -I./include
else
LDFLAGS = -fpermissive
endif
RM = rm

%.o: %.cpp        # combined w/ next line will compile recently changed .c files
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY : all clean    # .PHONY ignores files named all and clean
all: $(EXE)      # all is dependent on $(EXE) to be complete

$(EXE): $(OBJ)  # $(EXE) is dependent on all of the files in $(OBJ) to exist
	$(CC) $(OBJ) -o $@

clean:
	$(RM) -f $(OBJ) $(EXE)
