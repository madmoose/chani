SRCS=$(wildcard src/**/*.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

TOPSRCS=$(wildcard src/*.cpp)
TOPOBJS=$(subst .cpp,.o,$(TOPSRCS))

CPPFLAGS=-I src --std=c++17 -O2

all: chani

chani: src/chani.o $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

depend: .depend

.depend: $(SRCS) $(TOPSRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) $(TOPOBJS)

distclean: clean
	$(RM) chani *~ .depend

include .depend
