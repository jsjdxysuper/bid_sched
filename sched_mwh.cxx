//sched_mwh.cxx by wanggt
////////////////////////////////////////////////////////////
#include"common.h"
#include"psched.h"
#include "dmdb.h"
#include <iostream>
#include<map>
#include <stdlib.h>
#include"operate_config.h"
using namespace::std;
//定义为时段数+1
#define SDNUM 96
#define SQLSTRLEN 2048
string userNameStr;
string passwordStr;
string ipStr;
string logPathStr;

CDmDb *dmdb;
double fhycGloble[SDNUM+1];//
double llxjhGloble[SDNUM+1];//联络线计划
double xnyycGloble[SDNUM+1];//新能源预测
double sdjhGloble[SDNUM+1];//水调计划
double ddljhInputFl[SDNUM+1];//定电量输入负荷
const char Config[] = "db.ini";
/**
 * 读取数据库配置
 */
void readConfig()
{

	// 读配置文件，失败返回一个 异常类对象
	try {
		// Code that could throw an exception
		ConfigHandle.init(Config);
	}
	catch (operatorconfig::File_not_found &filenf) {
		std::cerr << "**Warring: Attemped to open no exist file <" << filenf.filename << ">." << std::endl;
		exit(CONFIG_FILE_NOTFOUND);
	}
	userNameStr = ConfigHandle.read("userName",userNameStr);
	passwordStr = ConfigHandle.read("password",passwordStr);
	ipStr = ConfigHandle.read("ip",ipStr);
	logPathStr = ConfigHandle.read("logPath",logPathStr);

}

/**
 * 读取算法所需参数
 */
void readAlgorithmPara()
{
	string paraSql = "SELECT CSID,CSSZ FROM HLJJHDB.HLJJHDB.XTCS "
				"WHERE CSLX='DDLJH'";
	long long retRedNum = dmdb->exec_select(paraSql.c_str(), 2, "读取算法参数数据");

	map<string,string> paraMap;
	for(int i=0;i<retRedNum;i++)
	{
		string typeStr = dmdb->get_select_data(i,0).c_str();
		string valStr = dmdb->get_select_data(i,1);
		paraMap[typeStr]= valStr;
	}

	mwStep=atof(paraMap["DDLJH_mwStep"].c_str());
	pntNum=atoi(paraMap["DDLJH_pntNum"].c_str());
	rampSd=atoi(paraMap["DDLJH_rampSd"].c_str());
	sdnum =atoi(paraMap["DDLJH_sdnum"].c_str());
	sd1   =atoi(paraMap["DDLJH_sd1"].c_str());
}

/**
 *读取负荷预测
 */
void readFhyc(string strDate)
{
	long long retRedNum = -1;
	string fhycSql="select sd,sz from HLJJHDB.HLJJHDB.FHYC \
			where rq='"+strDate+"' order by sd";

	retRedNum = dmdb->exec_select(fhycSql.c_str(), 2, "读取负荷预测数据");
	if(retRedNum!=SDNUM){
		cout<<"####负荷预测数据有问题"<<endl;
		exit(LOAD_FORCAST_DATA_ERROR);//负荷预测数据有问题
	}
	printf("id              name\n");
	for(int i=0;i<retRedNum;i++)
	{
		string sdStr = dmdb->get_select_data(i,0).c_str();
		int sdInt = atoi(sdStr.c_str());
		string valStr = dmdb->get_select_data(i,1);
		double valDouble = atof(valStr.c_str());
		fhycGloble[sdInt]= valDouble;
	}

	string llxjhSql="select sd,sz from HLJJHDB.HLJJHDB.LLXJH \
			where rq='"+strDate+"' order by sd";

	retRedNum = dmdb->exec_select(llxjhSql.c_str(), 2, "读取联络线计划数据");
	if(retRedNum!=SDNUM){
		cout<<"####联络线计划数据有问题"<<endl;
		exit(INTERCONECTION_DATA_ERROR);//联络线计划数据有问题
	}
	for(int i=0;i<retRedNum;i++)
	{
		string sdStr = dmdb->get_select_data(i,0).c_str();
		int sdInt = atoi(sdStr.c_str());
		string valStr = dmdb->get_select_data(i,1);
		double valDouble = atof(valStr.c_str());
		llxjhGloble[sdInt]= valDouble;
	}


	string xnyycSql = "select sd,sum(sz) from HLJJHDB.HLJJHDB.XNYYC \
			where rq='"+strDate+"' group by sd order by sd";


	retRedNum = dmdb->exec_select(xnyycSql.c_str(), 2, "读取新能源预测数据");
	if(retRedNum==SDNUM||retRedNum==0){
		for(int i=0;i<retRedNum;i++)
		{
			string sdStr = dmdb->get_select_data(i,0).c_str();
			int sdInt = atoi(sdStr.c_str());
			string valStr = dmdb->get_select_data(i,1);
			double valDouble = atof(valStr.c_str());
			xnyycGloble[sdInt]= valDouble;
		}
	}else{
		cout<<"####新能源预测数据有问题"<<endl;
		exit(-NEW_ENERGY_DATA_ERRO);//新能源预测数据有问题
	}

	string sdjhSql = "select sd,sum(sz) from HLJJHDB.HLJJHDB.SDJH \
              where rq='"+strDate+"' group by sd order by sd ";

	retRedNum = dmdb->exec_select(sdjhSql.c_str(), 2, "读取水电计划数据");
	if(retRedNum==SDNUM||retRedNum==0){
		for(int i=0;i<retRedNum;i++)
		{
			string sdStr = dmdb->get_select_data(i,0).c_str();
			int sdInt = atoi(sdStr.c_str());
			string valStr = dmdb->get_select_data(i,1);
			double valDouble = atof(valStr.c_str());
			sdjhGloble[sdInt]= valDouble;
		}
	}else{
		cout<<"水电计划数据有问题"<<endl;
		exit(WATER_SCHED_DATA);//新能源预测数据有问题
	}

	for(int i=1;i<=SDNUM;i++){
		ddljhInputFl[i] =fhycGloble[i]+llxjhGloble[i]-xnyycGloble[i]-sdjhGloble[i];
		wload0[i]=ddljhInputFl[i];
		wload [i]=ddljhInputFl[i];
	}
}

/**
 *读取日定电量
 */
void readRDDL()
{
	long long retRedNum =-1;


	string maxRqInRddl = "select max(rq) from HLJJHDB.HLJJHDB.RDDL";
	retRedNum = dmdb->exec_select(maxRqInRddl.c_str(), 1, "读取日定电量日期");
	if(retRedNum<=0){
		cout<<"####日定电量日期有问题"<<endl;
		exit(-104);
	}
	string maxRq = dmdb->get_select_data(0,0);

	string jzRddlSql = "select sid,sname,rddl,glsx,glxx from HLJJHDB.HLJJHDB.RDDL \
	         where rq='"+maxRq+"'";
	retRedNum = dmdb->exec_select(jzRddlSql.c_str(), 5, "读取日定电量数据");
	if(retRedNum<=0){
		cout<<"####机组日定电量日期有问题"<<endl;
		exit(-105);
	}

	fixbasData=NULL;
	fixbasNum =0;
	for(int i=0;i<retRedNum;i++){
		string sidStr = dmdb->get_select_data(i,0);
		string snameStr = dmdb->get_select_data(i,1);
		string rddlStr = dmdb->get_select_data(i,2);
		string glsxStr = dmdb->get_select_data(i,3);
		string glxxStr = dmdb->get_select_data(i,4);

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

	reverse(fixbasData);
}

void readAllDataFromDb(string strDate)
{
	readAlgorithmPara();
	readFhyc(strDate);
	readRDDL();

}

/**
 * 写入结果
 */
void writeToDb(string strDate)
{

	string deletSql = "DELETE FROM HLJJHDB.HLJJHDB.DDLJH WHERE RQ='"+strDate+"'";
	int deleteNum = dmdb->exec_sql(deletSql.c_str());
	cout<<"删除"<<deleteNum<<"条记录"<<endl;

	char insertDdljhSql[SQLSTRLEN];
	struct fixstr *xp=fixbasData;
	long insertNum = 0;
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
			int ret = dmdb->exec_sql(insertDdljhSql);
			if(ret!=0)
			{
				cout<<"####插入数据出错"<<endl;
				exit(106);
			}else
				insertNum++;
		}
		xp=xp->next;

	}
	cout<<"插入"<<insertNum<<"条数据"<<endl;
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

}
int main(long argc,char **argv)
{
//	int sd = sj2sd("23:45",96);
	string strDate = argv[1];
	readConfig();

	dmdb = new CDmDb(userNameStr.c_str(),passwordStr.c_str(),ipStr.c_str(),logPathStr.c_str());

	int initRet = dmdb->init_database();
	if(initRet!=0)
	{
		cout<<"####数据库初始化失败"<<endl;
		exit(99);
	}

	algrithm(strDate);

	delete(dmdb);
	printf("####成功!\n");
	return 0;
}
//end of file
