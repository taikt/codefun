IDIR=./include
CPPFLAGS=-I$(IDIR) -g
#if c source file, use CFLAGS, otherwise CPPFLAGS for c++
#LDFLAGS=-lctest -lcpptest
LDFLAGS=-lpthread
CXX=g++
CC=gcc

OBJ = looper.o

app.out : $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)
# $@ = app
# $^ = $(OBJ)

#.o : .cpp $(IDIR)/h1.h
.o : .cpp $(IDIR)
	$(CXX) -c $< $(CPPFLAGS)
# $< = .cpp (first reprerequisite, i.e test.cpp)

clean:
	rm *.o app.out

print-%  : ; @echo $* = $($*)


# make -f makefile1.mk
# make -f makefile1.mk clean
