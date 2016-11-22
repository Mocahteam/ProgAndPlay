SRC = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJ = $(SRC:.cpp=.o) # replaces the .cpp from SRC with .o
EXE = parser.exe

CC = g++
CFLAGS = -Wall
SPRING_VERSION = spring-0.82.5.1
REP_TRACES = ../../../SpringPP/$(SPRING_VERSION)/rts/lib/pp/traces
REP_CONT = ../../../SpringPP/$(SPRING_VERSION)/cont/traces/data
LDFLAGS = -I../../../SpringPP/$(SPRING_VERSION)/mingwlibs/include
RM = rm

%.o: %.cpp         # combined w/ next line will compile recently changed .c files
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY : all     # .PHONY ignores files named all
all: $(EXE)      # all is dependent on $(EXE) to be complete

$(EXE): $(OBJ)   # $(EXE) is dependent on all of the files in $(OBJ) to exist
	$(CC) $(OBJ) -o $@

install: $(REP_CONT)/params.json $(REP_CONT)/feedbacks.xml $(REP_TRACES)/TracesParser.cpp $(REP_TRACES)/TracesParser.h $(REP_TRACES)/TracesAnalyser.cpp $(REP_TRACES)/TracesAnalyser.h $(REP_TRACES)/Trace.cpp $(REP_TRACES)/Trace.h $(REP_TRACES)/Sequence.cpp $(REP_TRACES)/Sequence.h $(REP_TRACES)/Call.cpp $(REP_TRACES)/Call.h $(REP_TRACES)/CallDef.h $(REP_TRACES)/Event.cpp $(REP_TRACES)/Event.h $(REP_TRACES)/EventDef.h

$(REP_CONT)/params.json: ./params.json
	cp ./params.json $(REP_CONT)
	
$(REP_CONT)/feedbacks.xml: ./feedbacks.xml
	cp ./feedbacks.xml $(REP_CONT)

$(REP_TRACES)/TracesParser.cpp: src/TracesParser.cpp
	cp src/TracesParser.cpp $(REP_TRACES)

$(REP_TRACES)/TracesParser.h: src/TracesParser.h
	cp src/TracesParser.h $(REP_TRACES)
	
$(REP_TRACES)/TracesAnalyser.cpp: src/TracesAnalyser.cpp
	cp src/TracesAnalyser.cpp $(REP_TRACES)

$(REP_TRACES)/TracesAnalyser.h: src/TracesAnalyser.h
	cp src/TracesAnalyser.h $(REP_TRACES)
	
$(REP_TRACES)/Trace.cpp: src/Trace.cpp
	cp src/Trace.cpp $(REP_TRACES)
	
$(REP_TRACES)/Trace.h: src/Trace.h
	cp src/Trace.h $(REP_TRACES)
	
$(REP_TRACES)/Sequence.cpp: src/Sequence.cpp
	cp src/Sequence.cpp $(REP_TRACES)
	
$(REP_TRACES)/Sequence.h: src/Sequence.h
	cp src/Sequence.h $(REP_TRACES)
	
$(REP_TRACES)/Call.cpp: src/Call.cpp
	cp src/Call.cpp $(REP_TRACES)
	
$(REP_TRACES)/Call.h: src/Call.h
	cp src/Call.h $(REP_TRACES)
	
$(REP_TRACES)/CallDef.h: src/CallDef.h
	cp src/CallDef.h $(REP_TRACES)
	
$(REP_TRACES)/Event.cpp: src/Event.cpp
	cp src/Event.cpp $(REP_TRACES)
	
$(REP_TRACES)/Event.h: src/Event.h
	cp src/Event.h $(REP_TRACES)
	
$(REP_TRACES)/EventDef.h: src/EventDef.h
	cp src/EventDef.h $(REP_TRACES)

.PHONY : clean   # .PHONY ignores files named clean
clean:
	$(RM) $(OBJ) $(EXE)