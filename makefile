compileFlag:= -w -g -O0
inc:=-I /opt/dmdbms/bin/include 
lin:= -L /opt/dmdbms/bin -ldmapi
object_bid=sched_bid.o psched.o common.o dmdb.o operate_config.o
object_dingdianliang = sched_mwh.o  psched.o common.o dmdb.o operate_config.o
object_gen288Point = gen288Point.o psched.o common.o dmdb.o  operate_config.o sched_bid.o
all:bid_sched dingdianliang test testdb
testdb:testdb.cpp
	g++ -o testdb testdb.cpp dmdb.cpp $(inc) $(lin)
test:test.cpp
	g++ -o test test.cpp
gen288Point:$(object_gen288Point)
	g++ -o gen288Point $(object_gen288Point)  $(inc) $(compileFlag)  $(lin)
gen288Point.o:gen288Point.cpp common.h psched.h operate_config.h dmdb.h sched_bid.h
	g++ -o gen288Point.o -c gen288Point.cpp  $(inc) $(compileFlag)
dingdianliang:$(object_dingdianliang)
	g++ -o dingdianliang $(object_dingdianliang) $(compileFlag) $(lin)
bid_sched:$(object_bid)
	g++ -o bid_sched $(object_bid) $(compileFlag) $(lin)
sched_mwh.o:sched_mwh.cxx psched.h common.h sched_mwh.h
	g++ -o sched_mwh.o -c sched_mwh.cxx $(compileFlag) $(inc)
sched_bid.o:sched_bid.cxx psched.h common.h sched_bid.h
	g++ -o sched_bid.o -c sched_bid.cxx $(compileFlag) $(inc)
psched.o:psched.cpp psched.h common.h
	g++ -o psched.o -c psched.cpp $(compileFlag)
common.o:common.cpp common.h
	g++ -o common.o -c common.cpp $(compileFlag)
dmdb.o:dmdb.cpp dmdb.h
	g++ -o dmdb.o -c dmdb.cpp $(compileFlag) $(inc)
operate_config.o:operate_config.cpp operate_config.h
	g++ -o operate_config.o -c operate_config.cpp $(compileFlag)
clean:
	-rm $(object_bid) $(dingdianliang) $(gen288Point) gen288Point bid_sched dingdianliang
