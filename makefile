compileFlag:= -w -g -O0
inc:=-I /opt/dmdbms/bin/include 
lin:= -L /opt/dmdbms/bin -ldmapi
object_bid=sched_bid.o psched.o common.o dmdb.o inifile.o
object_dingdianliang = sched_mwh.o  psched.o common.o dmdb.o inifile.o
all:bid_sched dingdianliang
dingdianliang:$(object_dingdianliang)
	g++ -o dingdianliang $(object_dingdianliang) $(compileFlag) $(lin)
bid_sched:$(object_bid)
	g++ -o bid_sched $(object_bid) $(compileFlag) $(lin)
sched_mwh.o:sched_mwh.cxx psched.h common.h
	g++ -o sched_mwh.o -c sched_mwh.cxx $(compileFlag) $(inc)
sched_bid.o:sched_bid.cxx psched.h common.h
	g++ -o sched_bid.o -c sched_bid.cxx $(compileFlag) $(inc)
psched.o:psched.cpp psched.h common.h
	g++ -o psched.o -c psched.cpp $(compileFlag)
common.o:common.cpp common.h
	g++ -o common.o -c common.cpp $(compileFlag)
dmdb.o:dmdb.cpp dmdb.h
	g++ -o dmdb.o -c dmdb.cpp $(compileFlag) $(inc)
inifile.o:inifile.cpp inifile.h
	g++ -o inifile.o -c inifile.cpp $(compileFlag)
clean:
	-rm $(object_bid) $(dingdianliang) bid_sched dingdianliang
