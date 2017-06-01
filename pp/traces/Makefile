SRC = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)
OBJ = $(SRC:.cpp=.o) # replaces the .cpp from SRC with .o
EXE = parser.exe

CC = g++
CFLAGS = -Wall

REP_TRACES = ../../$(SPRING_VERSION)/rts/lib/pp/traces
REP_CONT = ../../$(SPRING_VERSION)/cont/traces/data
ifeq ($(OS),Windows_NT)
LDFLAGS = -I../../$(SPRING_VERSION)/mingwlibs/include
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

install: $(REP_CONT)/params.json $(REP_CONT)/feedbacks.xml $(REP_TRACES)/TracesParser.cpp $(REP_TRACES)/TracesParser.h $(REP_TRACES)/TracesAnalyser.cpp $(REP_TRACES)/TracesAnalyser.h $(REP_TRACES)/Trace.cpp $(REP_TRACES)/Trace.h $(REP_TRACES)/TraceConstantList.h $(REP_TRACES)/Sequence.cpp $(REP_TRACES)/Sequence.h $(REP_TRACES)/Call.cpp $(REP_TRACES)/Call.h $(REP_TRACES)/CallDef.h $(REP_TRACES)/Event.cpp $(REP_TRACES)/Event.h $(REP_TRACES)/EventDef.h

$(REP_CONT)/params.json: ./example/params.json
	cp ./example/params.json $(REP_CONT)

$(REP_CONT)/feedbacks.xml: ./example/feedbacks.xml
	cp ./example/feedbacks.xml $(REP_CONT)

$(REP_TRACES)/TracesParser.cpp: TracesParser.cpp
	cp TracesParser.cpp $(REP_TRACES)

$(REP_TRACES)/TracesParser.h: TracesParser.h
	cp TracesParser.h $(REP_TRACES)

$(REP_TRACES)/TracesAnalyser.cpp: TracesAnalyser.cpp
	cp TracesAnalyser.cpp $(REP_TRACES)

$(REP_TRACES)/TracesAnalyser.h: TracesAnalyser.h
	cp TracesAnalyser.h $(REP_TRACES)

$(REP_TRACES)/Trace.cpp: Trace.cpp
	cp Trace.cpp $(REP_TRACES)

$(REP_TRACES)/Trace.h: Trace.h
	cp Trace.h $(REP_TRACES)

$(REP_TRACES)/TraceConstantList.h: TraceConstantList.h
	cp TraceConstantList.h $(REP_TRACES)

$(REP_TRACES)/Sequence.cpp: Sequence.cpp
	cp Sequence.cpp $(REP_TRACES)

$(REP_TRACES)/Sequence.h: Sequence.h
	cp Sequence.h $(REP_TRACES)

$(REP_TRACES)/Call.cpp: Call.cpp
	cp Call.cpp $(REP_TRACES)

$(REP_TRACES)/Call.h: Call.h
	cp Call.h $(REP_TRACES)

$(REP_TRACES)/CallDef.h: CallDef.h
	cp CallDef.h $(REP_TRACES)

$(REP_TRACES)/Event.cpp: Event.cpp
	cp Event.cpp $(REP_TRACES)

$(REP_TRACES)/Event.h: Event.h
	cp Event.h $(REP_TRACES)

$(REP_TRACES)/EventDef.h: EventDef.h
	cp EventDef.h $(REP_TRACES)

clean:
	$(RM) -f $(OBJ) $(EXE)
