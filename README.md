http://cgi.di.uoa.gr/~mema/courses/k24/homeworks/hw3-spring-2021.pdf


The aim of this project is to get acquainted with the creation of threads and the web sockets.


**Run:**
Makefile: make: separate compilation
          make run: ./travelMonitorClient -m 10 -b 16 -c 10 -s 100000 -i ./big_dir -t 5
          make valgrind: valgrind ./travelMonitorClient -m 10 -b 16 -c 10 -s 100000 -i ./big_dir -t 5
          make clean: Ekkatharisi .o, ektelesimwn kai log files
          make count: Total lines of code
