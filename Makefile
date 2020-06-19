.PHONY: all run

all: run

run: a.out
	./a.out | xxd

bin: a.out
	./a.out | xxd -b

clean:
	rm -rf *.o *.out *.pb.h *.pb.cc

%.pb.cc: %.proto
	protoc --cpp_out=. $<

%.pb.h: %.pb.cc
	true

%.pb.o : %.pb.cc
	g++ -c -o $@ $<

%.o: %.cpp
	g++ -c -o $@ $<

a.out: a.pb.o main.o
	g++ -lprotobuf -lpthread -o $@ main.o a.pb.o
	rm -rf *.o
