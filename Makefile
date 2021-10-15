MODULE = ./source
INCLUDE = ./include

OBJS	= travelMonitorClient.o travelMonitorClientHelp.o common.o
OBJS1	= monitorServer.o monitorServerHelp.o bloomfilter.o citizen.o hash.o skiplist.o virus.o common.o
SOURCE	= travelMonitorClient.cpp travelMonitorClientHelp.cpp common.cpp
SOURCE1	= monitorServer.cpp monitorServerHelp.cpp bloomfilter.cpp citizen.cpp hash.cpp skiplist.cpp virus.cpp common.cpp
HEADER	= travelMonitorClientHelp.h monitorServerHelp.h bloomfilter.h citizen.h hash.h skiplist.h virus.h common.h
OUT	= travelMonitorClient
OUT1 = monitorServer
CC	 = g++
FLAGS	 = -g3 -c -Wall -I$(INCLUDE)
LFLAGS	= -lpthread

all: $(OBJS) $(OBJS1)
	$(CC) -g $(OBJS) -o $(OUT)
	$(CC) -g $(OBJS1) -o $(OUT1) $(LFLAGS)

travelMonitorClient.o: $(MODULE)/travelMonitorClient.cpp
	$(CC) $(FLAGS) $(MODULE)/travelMonitorClient.cpp -std=c++11

travelMonitorClientHelp.o: $(MODULE)/travelMonitorClientHelp.cpp
	$(CC) $(FLAGS) $(MODULE)/travelMonitorClientHelp.cpp -std=c++11

monitorServer.o: $(MODULE)/monitorServer.cpp
	$(CC) $(FLAGS) $(MODULE)/monitorServer.cpp -std=c++11

monitorServerHelp.o: $(MODULE)/monitorServerHelp.cpp
	$(CC) $(FLAGS) $(MODULE)/monitorServerHelp.cpp -std=c++11

bloomfilter.o: $(MODULE)/bloomfilter.cpp
	$(CC) $(FLAGS) $(MODULE)/bloomfilter.cpp -std=c++11

citizen.o: $(MODULE)/citizen.cpp
	$(CC) $(FLAGS) $(MODULE)/citizen.cpp -std=c++11

hash.o: $(MODULE)/hash.cpp
	$(CC) $(FLAGS) $(MODULE)/hash.cpp -std=c++11

skiplist.o: $(MODULE)/skiplist.cpp
	$(CC) $(FLAGS) $(MODULE)/skiplist.cpp -std=c++11

virus.o: $(MODULE)/virus.cpp
	$(CC) $(FLAGS) $(MODULE)/virus.cpp -std=c++11

common.o: $(MODULE)/common.cpp
	$(CC) $(FLAGS) $(MODULE)/common.cpp -std=c++11

clean:
	rm -f $(OBJS) $(OUT) $(OBJS1) $(OUT1)
	rm -f ./logs/*

run:
	./travelMonitorClient -m 10 -b 16 -c 10 -s 100000 -i ./big_dir -t 5

valgrind:
	valgrind ./travelMonitorClient -m 10 -b 16 -c 10 -s 100000 -i ./big_dir -t 5

count:
	wc $(MODULE)/*.cpp $(INCLUDE)/*.h