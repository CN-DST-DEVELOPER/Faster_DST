CC= gcc
LIBS= -std=c99 -std=c++11 -ldl -lm -lcrypto -lstdc++
RM= rm -f
MAKE = make

include ./FunctionLocater/Makefile

preload: main.o Cache.o $(FunctionLocater)
	$(CC) $(CFLAGS) -o libpreload.so $? $(LIBS)

main.o: main.cpp $(FunctionLocater_H) Cache.h
	$(CC) -c $(CFLAGS) -o $@ $< $(LIBS)

Cache.o: Cache.cpp Cache.h
	$(CC) -c $(CFLAGS) -o $@ $< $(LIBS)

clean: FunctionLocater_clean
	$(RM) ./*.o

.PHONY: all clean