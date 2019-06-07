loser: md5.o mp1.o
	g++ md5.o mp1.o -o loser
md5.o: md5.cpp md5.h
	g++ md5.cpp -c
mp1.o: mp1.cpp md5.h
	g++ mp1.cpp -c
clean:
	rm -rf md5.o mp1.o loser