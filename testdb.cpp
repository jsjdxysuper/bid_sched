#include<iostream>
#include<stdlib.h>
#include"dmdb.h"
using namespace std;
int main(){
	cout<<"Hello"<<endl;
	char userName[]="SYSDBA";
	char password[]="SYSDBA";
	char ip[]="localhost";
	char logPath[]="/home/user/work/log/testdb/log.txt";
	CDmDb dmdb(userName,password,ip,logPath);

	int initRet = dmdb.init_database();
	char genSql[]="select id,name from TEST.TEST.GEN";

	long long retRedNum = dmdb.exec_select(genSql, 2, "测试");
	cout<<"id"<<"      "<<"name"<<endl;
	for(int i=0;i<retRedNum;i++)
	{
		cout<<dmdb.get_select_data(i,0)<<"     "<<dmdb.get_select_data(i,1)<<endl;
	}

	dmdb.close_database();
}
