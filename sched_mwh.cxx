//sched_mwh.cxx by wanggt
////////////////////////////////////////////////////////////
#include"common.h"
#include"psched.h"
#include "dmdb.h"
#include <iostream>
#include<map>
#include"inifile.h"
using namespace::std;
//定义为时段数+1
#define SDNUM 96
string userNameStr;
string passwordStr;
string ipStr;
string logPathStr;
string sd2sj[]={"00:15","00:30","00:45",
		"01:00","01:15","01:30","01:45",
		"02:00","02:15","02:30","02:45",
		"03:00","03:15","03:30","03:45",
		"04:00","04:15","04:30","04:45",
		"05:00","05:15","05:30","05:45",
		"06:00","06:15","06:30","06:45",
		"07:00","07:15","07:30","07:45",
		"08:00","08:15","08:30","08:45",
		"09:00","09:15","09:30","09:45",
		"10:00","10:15","10:30","10:45",
		"11:00","11:15","11:30","11:45",
		"12:00","12:15","12:30","12:45",
		"13:00","13:15","13:30","13:45",
		"14:00","14:15","14:30","14:45",
		"15:00","15:15","15:30","15:45",
		"16:00","16:15","16:30","16:45",
		"17:00","17:15","17:30","17:45",
		"18:00","18:15","18:30","18:45",
		"19:00","19:15","19:30","19:45",
		"20:00","20:15","20:30","20:45",
		"21:00","21:15","21:30","21:45",
		"22:00","22:15","22:30","22:45",
		"23:00","23:15","23:30","23:45",
		"24:00"
};

////////////////////////////////////////////////////////////
void readConfig()
{
	CIniFile iniFile("./db.ini");
	userNameStr = iniFile.GetStrValue("dmdb","userName");
	passwordStr =  iniFile.GetStrValue("dmdb","password");
	ipStr =  iniFile.GetStrValue("dmdb","ip");
	logPathStr = iniFile.GetStrValue("dmdb","logPath");
}
void readAllDataFromDb(string strDate)
{
	double fhycGloble[SDNUM+1];//
	double llxjhGloble[SDNUM+1];//联络线计划
	double xnyycGloble[SDNUM+1];//新能源预测
	double sdjhGloble[SDNUM+1];//水调计划
	double ddljhInputFl[SDNUM+1];//定电量输入负荷

	CDmDb dmdb(userNameStr.c_str(),passwordStr.c_str(),ipStr.c_str(),logPathStr.c_str());

	int initRet = dmdb.init_database();

	string paraSql = "select TYPEC,SVALUE from HLJJHDB.HLJJHDB.ALGPARA \
	        where TYPEA='FDJH' AND TYPEB='DDLJH'";
	long long retRedNum = dmdb.exec_select(paraSql.c_str(), 2, "测试");

	map<string,string> paraMap;
	for(int i=0;i<retRedNum;i++)
	{
		string typeStr = dmdb.get_select_data(i,0).c_str();
		string valStr = dmdb.get_select_data(i,1);
		paraMap[typeStr]= valStr;
	}

	mwStep=atof(paraMap["mwStep"].c_str());
	pntNum=atoi(paraMap["pntNum"].c_str());
	rampSd=atoi(paraMap["rampSd"].c_str());
	sdnum =atoi(paraMap["sdnum"].c_str());
	sd1   =atoi(paraMap["sd1"].c_str());
//===============================================读取配置参数============================
	string fhycSql="select sd,sz from HLJJHDB.HLJJHDB.FHYC \
			where rq='"+strDate+"' order by sd";

	retRedNum = dmdb.exec_select(fhycSql.c_str(), 2, "测试");
	if(retRedNum!=SDNUM){
		cout<<"负荷预测数据有问题"<<endl;
		exit(-100);//负荷预测数据有问题
	}
	printf("id              name\n");
	for(int i=0;i<retRedNum;i++)
	{
		string sdStr = dmdb.get_select_data(i,0).c_str();
		int sdInt = atoi(sdStr.c_str());
		string valStr = dmdb.get_select_data(i,1);
		double valDouble = atof(valStr.c_str());
		fhycGloble[sdInt]= valDouble;
	}

	string llxjhSql="select sd,sz from HLJJHDB.HLJJHDB.LLXJH \
			where rq='"+strDate+"' order by sd";

	retRedNum = dmdb.exec_select(llxjhSql.c_str(), 2, "测试");
	if(retRedNum!=SDNUM){
		cout<<"联络线计划数据有问题"<<endl;
		exit(-101);//联络线计划数据有问题
	}
	for(int i=0;i<retRedNum;i++)
	{
		string sdStr = dmdb.get_select_data(i,0).c_str();
		int sdInt = atoi(sdStr.c_str());
		string valStr = dmdb.get_select_data(i,1);
		double valDouble = atof(valStr.c_str());
		llxjhGloble[sdInt]= valDouble;
	}


	string xnyycSql = "select sd,sum(sz) from HLJJHDB.HLJJHDB.XNYYC \
			where rq='"+strDate+"' group by sd order by sd";


	retRedNum = dmdb.exec_select(xnyycSql.c_str(), 2, "测试");
	if(retRedNum==SDNUM||retRedNum==0){
		for(int i=0;i<retRedNum;i++)
		{
			string sdStr = dmdb.get_select_data(i,0).c_str();
			int sdInt = atoi(sdStr.c_str());
			string valStr = dmdb.get_select_data(i,1);
			double valDouble = atof(valStr.c_str());
			xnyycGloble[sdInt]= valDouble;
		}
	}else{
		cout<<"新能源预测数据有问题"<<endl;
		exit(-102);//新能源预测数据有问题
	}

	string sdjhSql = "select sd,sum(sz) from HLJJHDB.HLJJHDB.SDJH \
              where rq='"+strDate+"' group by sd order by sd ";

	retRedNum = dmdb.exec_select(sdjhSql.c_str(), 2, "测试");
	if(retRedNum==SDNUM||retRedNum==0){
		for(int i=0;i<retRedNum;i++)
		{
			string sdStr = dmdb.get_select_data(i,0).c_str();
			int sdInt = atoi(sdStr.c_str());
			string valStr = dmdb.get_select_data(i,1);
			double valDouble = atof(valStr.c_str());
			sdjhGloble[sdInt]= valDouble;
		}
	}else{
		cout<<"水调计划数据有问题"<<endl;
		exit(-103);//新能源预测数据有问题
	}

	for(int i=1;i<=SDNUM;i++){
		ddljhInputFl[i] =fhycGloble[i]+llxjhGloble[i]-xnyycGloble[i]-sdjhGloble[i];
		wload0[i]=ddljhInputFl[i];
		wload [i]=ddljhInputFl[i];
	}
//=================================================负荷预测设置完毕===========================

	string maxRqInRddl = "select max(rq) from HLJJHDB.HLJJHDB.RDDL";
	retRedNum = dmdb.exec_select(maxRqInRddl.c_str(), 2, "测试");
	if(retRedNum<=0){
		cout<<"日定电量日期有问题"<<endl;
		exit(-104);
	}
	string maxRq = dmdb.get_select_data(0,0);

	string jzRddlSql = "select sid,sname,rddl,glsx,glxx from HLJJHDB.HLJJHDB.RDDL \
	         where rq='"+maxRq+"'";
	retRedNum = dmdb.exec_select(jzRddlSql.c_str(), 5, "测试");
	if(retRedNum<=0){
		cout<<"机组日定电量日期有问题"<<endl;
		exit(-105);
	}

	fixbasData=NULL;
	fixbasNum =0;
	for(int i=0;i<retRedNum;i++){
		string sidStr = dmdb.get_select_data(i,0);
		string snameStr = dmdb.get_select_data(i,1);
		string rddlStr = dmdb.get_select_data(i,2);
		string glsxStr = dmdb.get_select_data(i,3);
		string glxxStr = dmdb.get_select_data(i,4);

		struct fixstr *xp=(struct fixstr *)malloc(sizeof(struct fixstr));
		if(xp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
		memset(xp,0,sizeof(struct fixstr));
		xp->i       =i+1;
		sprintf(xp->id,"%s",sidStr.c_str());
		sprintf(xp->descr,"%s",snameStr.c_str());
		xp->mwmax[0]=atof(glsxStr.c_str());
		xp->mwmin[0]=atof(glxxStr.c_str());
		xp->mwh0    =atof(rddlStr.c_str());
		xp->next    =fixbasData;
		fixbasData=xp;
		fixbasNum++;
	}

    ////////////////////////////////////////////////////////////

	reverse(fixbasData);

}

void writeToDb(string strDate)
{
	CDmDb dmdb(userNameStr.c_str(),passwordStr.c_str(),ipStr.c_str(),logPathStr.c_str());

	int initRet = dmdb.init_database();
	string deletSql = "DELETE FROM HLJJHDB.HLJJHDB.DDLJH WHERE RQ='"+strDate+"'";
	int deleteNum = dmdb.exec_sql(deletSql.c_str());
	cout<<"删除"<<deleteNum<<"条记录"<<endl;

	char insertDdljhSql[2048];
	struct fixstr *xp=fixbasData;
	for(int i=0;i<fixbasNum;i++)
	{
		string sid = xp->id;
		string sname = xp->descr;

		for(int j=0;j<SDNUM;j++){
//			insertDdljhSql = "INSERT INTO HLJJHDB.HLJJHDB.DDLJH(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) "
//					"VALUES('"+strDate+"','"+sd2sj[i]+"','"+(j+1)+"','"+sid+"',,,,,,)";
			sprintf(insertDdljhSql,"INSERT INTO HLJJHDB.HLJJHDB.DDLJH(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) "
					"VALUES('%s','%s','%d','%s','%s','%f','%f','%f','%f','%f')",
					strDate.c_str(),sd2sj[j].c_str(),j+1,sid.c_str(),sname.c_str(),xp->mwmax[j+1],
					xp->mwmin[j+1],xp->mwup[j+1],xp->mwdn[j+1],xp->mw[j+1]);
			//printf("%s\n",insertDdljhSql);
			int ret = dmdb.exec_sql(insertDdljhSql);
			if(ret!=0)
			{

			}
		}
		xp=xp->next;

	}
}
void algrithm(string strDate)
{
	readConfig();
	sched_start("sched01");
	//sched_readdat();
	readAllDataFromDb(strDate);
	sched_dataprep();

	////////////////////////////////////////////////////////////

	sched_fun(fixbasData);

	sched_gross();
	//sched_report();
	writeToDb(strDate);
	printf("\n\nSuccess!\n");
}
int main(long argc,char **argv)
{
	algrithm("2017-07-17");


	//iniFile.
	////////////////////////////////////////////////////////////

	return 0;
}
//end of file
