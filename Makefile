TIME_LIB = ../time
COORDINATE_LIB = ../coordinates
CPPXML_LIB = ../cppxml
DATA_LIB = ../data
OBJS = orbit_server.o
EXE = orbit_server
CFLAGS += -g --std=c++11 -I${TIME_LIB} -I${COORDINATE_LIB} -I${CPPXML_LIB} -I${DATA_LIB}
LFLAGS = -lboost_system -lboost_thread

.cpp.o:
	g++ ${CFLAGS} -c $< -o $@


all: ${OBJS}
	g++ ${OBJS} ${LFLAGS} -o ${EXE}


clean:
	rm -f *~ ${EXE} ${OBJS} core
