//sched_bid.cxx by wanggt, 2015/4/29
////////////////////////////////////////////////////////////
#include"common.h"
#include"psched.h"
#include "dmdb.h"
#include <iostream>
#include<map>
#include<vector>
#include"operate_config.h"
using namespace::std;
//定义为时段数+1
#define SDNUM 96
#define SQLSTRLEN 2048
#define paraSize 5
string userNameStr;
string passwordStr;
string ipStr;
string logPathStr;

CDmDb *dmdb;
double fhycxzGloble[SDNUM+1];//负荷预测修正
double llxjhGloble[SDNUM+1];//联络线计划
double xnyycGloble[SDNUM+1];//新能源预测
double sdjhGloble[SDNUM+1];//水调计划
double ddljhGlobel[SDNUM+1];//定电量输入负荷
double dqxjhGlobel[SDNUM+1];//定曲线计划
double bidFhInput[SDNUM+1];//竞价负荷输入
const char Config[] = "db.ini";
void convertAndSetKtj(string startDateTime,string endDateTime,string strDate,long ktj[],int ktjnum);
////////////////////////////////////////////////////////////

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
		exit(98);
	}
	userNameStr = ConfigHandle.read("userName",userNameStr);
	passwordStr = ConfigHandle.read("password",passwordStr);
	ipStr = ConfigHandle.read("ip",ipStr);
	logPathStr = ConfigHandle.read("logPath",logPathStr);
}

/**
 * 读取算法配置参数
 */
void readAlgorithmPara()
{

	string paraSql = "SELECT CSID,CSSZ FROM HLJJHDB.HLJJHDB.XTCS "
				"WHERE CSLX='BID'";
	long long retRedNum = dmdb->exec_select(paraSql.c_str(), 2, "读取算法参数");

	map<string,string> paraMap;
	for(int i=0;i<retRedNum;i++)
	{
		string typeStr = dmdb->get_select_data(i,0).c_str();
		string valStr = dmdb->get_select_data(i,1);
		paraMap[typeStr]= valStr;
	}
	if(paraMap.size()<paraSize)
	{
		cout<<"####数据库中算法参数数量小于需求"<<endl;
		exit(211);
	}
	mwStep=atof(paraMap["BID_mwStep"].c_str());
	pntNum=atoi(paraMap["BID_pntNum"].c_str());
	rampSd=atoi(paraMap["BID_rampSd"].c_str());
	sdnum =atoi(paraMap["BID_sdnum"].c_str());
	sd1   =atoi(paraMap["BID_sd1"].c_str());

	g_maxsdSpan = atoi(paraMap["BID_maxsdSpan"].c_str());
	g_tol = atof(paraMap["BID_tol"].c_str());
	g_balanceInitPowerRatio = atof(paraMap["BID_balanceInitPowerRatio"].c_str());

}

/**
 * 获取算法输入：负荷率
 */
void readFhl(string strDate)
{
	string fhycxzSql="select sd,sz from HLJJHDB.HLJJHDB.FHYCXZ \
			where rq='"+strDate+"' order by sd";

	long retRedNum = -1;
	retRedNum = dmdb->exec_select(fhycxzSql.c_str(), 2, "读取负荷预测修正数据");
	if(retRedNum!=SDNUM){
		cout<<"####负荷预测数据有问题"<<endl;
		exit(-200);//负荷预测数据有问题
	}
	printf("id              name\n");
	for(int i=0;i<retRedNum;i++)
	{
		string sdStr = dmdb->get_select_data(i,0).c_str();
		int sdInt = atoi(sdStr.c_str());
		string valStr = dmdb->get_select_data(i,1);
		double valDouble = atof(valStr.c_str());
		fhycxzGloble[sdInt]= valDouble;
	}

	string llxjhSql="select sd,sz from HLJJHDB.HLJJHDB.LLXJH \
			where rq='"+strDate+"' order by sd";

	retRedNum = dmdb->exec_select(llxjhSql.c_str(), 2, "读取联络线计划数据");
	if(retRedNum!=SDNUM){
		cout<<"####联络线计划数据有问题"<<endl;
		exit(-201);//联络线计划数据有问题
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
		exit(-202);//新能源预测数据有问题
	}

	string sdjhSql = "select sd,sum(sz) from HLJJHDB.HLJJHDB.SDJH \
              where rq='"+strDate+"' group by sd order by sd ";

	retRedNum = dmdb->exec_select(sdjhSql.c_str(), 2, "测试");
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
		cout<<"####水调计划数据有问题"<<endl;
		exit(-203);//新能源预测数据有问题
	}

	string ddljhSql = "SELECT SD,SUM(GL) FROM HLJJHDB.HLJJHDB.DDLJH "
			"WHERE RQ='"+strDate+"' GROUP BY SD";

	retRedNum = dmdb->exec_select(ddljhSql.c_str(), 2, "读取定电量发电计划");
	if(retRedNum==SDNUM||retRedNum==0){
		for(int i=0;i<retRedNum;i++)
		{
			string sdStr = dmdb->get_select_data(i,0).c_str();
			int sdInt = atoi(sdStr.c_str());
			string valStr = dmdb->get_select_data(i,1);
			double valDouble = atof(valStr.c_str());
			ddljhGlobel[sdInt]= valDouble;
		}
	}else{
		cout<<"####定电量发电计划数据有问题"<<endl;
		exit(-204);//新能源预测数据有问题
	}

	string dqxjhSql = "SELECT SD,SUM(GL) FROM HLJJHDB.HLJJHDB.DQXJH "
			"WHERE RQ='"+strDate+"' GROUP BY SD";

	retRedNum = dmdb->exec_select(dqxjhSql.c_str(), 2, "读取定曲线发电计划");
	if(retRedNum==SDNUM||retRedNum==0){
		for(int i=0;i<retRedNum;i++)
		{
			string sdStr = dmdb->get_select_data(i,0).c_str();
			int sdInt = atoi(sdStr.c_str());
			string valStr = dmdb->get_select_data(i,1);
			double valDouble = atof(valStr.c_str());
			dqxjhGlobel[sdInt]= valDouble;
		}
	}else{
		cout<<"####定曲线发电计划数据有问题"<<endl;
		exit(-205);//新能源预测数据有问题
	}


	for(int i=1;i<=SDNUM;i++){
		bidFhInput[i] =fhycxzGloble[i]+llxjhGloble[i]-xnyycGloble[i]-sdjhGloble[i]-ddljhGlobel[i]-dqxjhGlobel[i];
		wload0[i]=bidFhInput[i];
		wload [i]=bidFhInput[i];
	}

}

/**
 *读取权重系数
 */
void readQzxs(string strDate)
{
	long retRedNum = 0;

	string maxRqInQzxs = "select max(rq) from HLJJHDB.HLJJHDB.QZXS WHERE RQ<='"+strDate+"'";
	retRedNum = dmdb->exec_select(maxRqInQzxs.c_str(), 1, "读取权重系数最大日期");
	if(retRedNum<=0){
		cout<<"####权重系数日期有问题"<<endl;
		exit(-206);
	}
	string maxRq = dmdb->get_select_data(0,0);
	//取出定曲线和平衡机组
	string jzRddlSql = "select sid,sname,qzxs from HLJJHDB.HLJJHDB.QZXS "
				"where rq='"+maxRq+"' and SID NOT IN "
				"(select DISTINCT(SID) from HLJJHDB.HLJJHDB.DQXJH "
						"WHERE RQ='"+strDate+"' union "
						"SELECT DISTINCT(SID) FROM HLJJHDB.HLJJHDB.PHJ "
						"WHERE RQ=(SELECT MAX(RQ) FROM HLJJHDB.HLJJHDB.PHJ WHERE RQ<='"+strDate+"'))";
	retRedNum = dmdb->exec_select(jzRddlSql.c_str(), 3, "读取权重系数");
	if(retRedNum<=0){
		cout<<"####权重系数有问题"<<endl;
		exit(-207);
	}

	micData=NULL;
	micNum =0;
	for(int i=0;i<retRedNum;i++)
	{
		struct micstr *mp=(struct micstr *)malloc(sizeof(struct micstr));
		if(mp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }

		string sidStr = dmdb->get_select_data(i,0);
		string snameStr = dmdb->get_select_data(i,1);
		string qzxsStr = dmdb->get_select_data(i,2);


		memset(mp,0,sizeof(struct micstr));
		mp->i     =i;
		sprintf(mp->id,"%s",sidStr.c_str());
		sprintf(mp->descr,"%s",snameStr.c_str());

		mp->wt = atof(qzxsStr.c_str());
		mp->next  =micData;
		micData   =mp;
		micNum++;
	}
	reverse(micData);
}

/**
 * 设置竞价参数
 */
void setBid()
{
	//把此程序的竞价固定
	struct micstr *mp = micData;
	for(int i=0;i<micNum;i++)
	{
		mp->pntNum=pntNum;
		mp->pnt[1].x = mp->mwmin;
		mp->pnt[1].y = 1;
		mp->pnt[2].x = mp->mwmax;
		mp->pnt[2].y = 1;

		double everyStep = mp->mwmin/rampSd;
		for(int j=1;j<=rampSd;j++)
		{
			mp->rampdn[j] = mp->mwmin-everyStep*(j-1);
			mp->rampup[j] = everyStep*j;
		}
		mp = mp->next;
	}
}

/**
 * 读取开停机信息
 */
void readKtj(string strDate)
{
	long retRedNum = 0;

//////////////////////////////////////////此处应该有开停机///////////////////////、
	string ktjSql = "select  sid,sname,qssj,zzsj from HLJJHDB.HLJJHDB.JZKT "
			"where substr(qssj,0,10)<='"+strDate+"' and substr(zzsj,0,10)>='"+strDate+"'";
	retRedNum = dmdb->exec_select(ktjSql.c_str(), 4, "读取停机信息");

	//默认设置所有机组开机
	struct micstr *mp = micData;
	for(int i=0;i<micNum;i++)
	{
		for(int j=1;j<=SDNUM;j++)
		{
			mp->stat[j] = 1;
		}
		mp = mp->next;
	}

	for(int i=0;i<retRedNum;i++)
	{
		string sidStr = dmdb->get_select_data(i,0);
		string snameStr = dmdb->get_select_data(i,1);
		string qssjStr = dmdb->get_select_data(i,2);
		string zzsjStr = dmdb->get_select_data(i,3);

		struct micstr *mp = micData;
		for(int j=0;j<micNum;j++)
		{
			if(strcmp(sidStr.c_str(),mp->id)==0)
			{

				convertAndSetKtj(qssjStr,zzsjStr,strDate,mp->stat,SDNUM);
				cout<<"id:"<<mp->id<<",name:"<<mp->descr<<": ";
				for(int i=1;i<=SDNUM;i++)
				{
					cout<<mp->stat[i]<<" ";
				}
				cout<<"endl"<<endl;
			}
			mp = mp->next;
		}
	}
}

/**
 * 向机组结构中注入功率限值
 */
void readGlxz(string strDate)
{
	long retRedNum = 0;

	string maxRqInGlxz = "select max(rq) from HLJJHDB.HLJJHDB.GLXZ where rq<='"+strDate+"'";
	retRedNum = dmdb->exec_select(maxRqInGlxz.c_str(), 1, "读取功率限值最大日期");
	if(retRedNum<=0){
		cout<<"####功率限值最大日期有问题"<<endl;
		exit(-208);
	}
	string maxRq = dmdb->get_select_data(0,0);

	string glxzSql = "select sid,sname,glsx,glxx from HLJJHDB.HLJJHDB.GLXZ "
			"where rq='"+maxRq+"'";

	retRedNum = dmdb->exec_select(glxzSql.c_str(), 4, "读取功率限值");
	if(retRedNum<=0){
		cout<<"####功率限值有问题"<<endl;
		exit(-209);
	}


	for(int i=0;i<retRedNum;i++)
	{
		string sidStr = dmdb->get_select_data(i,0);
		string snameStr = dmdb->get_select_data(i,1);
		string glsxStr = dmdb->get_select_data(i,2);
		string glxxStr = dmdb->get_select_data(i,3);

		double glsxDouble = atof(glsxStr.c_str());
		double glxxDouble = atof(glxxStr.c_str());
		struct micstr *mp = micData;
		while(mp!=NULL)
		{
			if(strcmp(mp->id,sidStr.c_str())==0)
			{
				mp->mwmin = glxxDouble;
				mp->mwmax = glsxDouble;
			}
			mp = mp->next;
		}
	}
}
void readBalanceGen(string strDate)
{
	vector<vector<string> >balanceGenId;
	string phjMaxRqSql = "SELECT MAX(RQ) FROM HLJJHDB.HLJJHDB.PHJ WHERE RQ<='"+strDate+"'";
	dmdb->exec_select(phjMaxRqSql.c_str(),1,"balance gen max date");
	string phjMaxDateStr = dmdb->get_select_data(0,0);
	string glxzMaxRqSql = "SELECT MAX(RQ) FROM HLJJHDB.HLJJHDB.GLXZ WHERE RQ<='"+strDate+"'";
	dmdb->exec_select(glxzMaxRqSql.c_str(),1,"power limit max date");
	string glxzMaxDateStr = dmdb->get_select_data(0,0);
	string balanceSql = "SELECT PHJ.SID,GLXZ.SNAME,GLXZ.GLSX,GLXZ.GLXX FROM HLJJHDB.HLJJHDB.PHJ PHJ,HLJJHDB.HLJJHDB.GLXZ GLXZ "
			"WHERE PHJ.SID=GLXZ.SID AND PHJ.RQ='"+phjMaxDateStr+"' AND GLXZ.RQ='"+glxzMaxDateStr+"'";
	long rowNum = dmdb->exec_select(balanceSql.c_str(),4,"balance gen select");

	for(int i=0;i<rowNum;i++)
	{
		struct micstr *mp=(struct micstr *)malloc(sizeof(struct micstr));
		if(mp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }

		string sidStr = dmdb->get_select_data(i,0);
		string snameStr = dmdb->get_select_data(i,1);
		string glsxStr = dmdb->get_select_data(i,2);
		string glxxStr = dmdb->get_select_data(i,3);


		memset(mp,0,sizeof(struct micstr));
		mp->i     =i;
		sprintf(mp->id,"%s",sidStr.c_str());
		sprintf(mp->descr,"%s",snameStr.c_str());


		mp->mwmax = atof(glsxStr.c_str());
		mp->mwmin = atof(glxxStr.c_str());
		mp->next  =balanceMicData;
		balanceMicData   =mp;
		balanceNum++;
	}
	reverse(balanceMicData);

	initBalanceGen();//sub balance init power
}
/**
 * 读取算法所需数据，并填写全局变量
 */
void readAllDataFromDb(string strDate)
{

	readAlgorithmPara();
	readFhl(strDate);
	readQzxs(strDate);
	readGlxz(strDate);
	setBid();
	readKtj(strDate);
	readBalanceGen(strDate);

}

/**
 * 将结果存入数据库
 */
void writeToDb(string strDate)
{

	string deleteRjzjh = "delete from HLJJHDB.HLJJHDB.RJZJH "
			"where rq='"+strDate+"'";
	string deleteRjzjh288 = "delete from HLJJHDB.HLJJHDB.RJZJH288 "
			"where rq='"+strDate+"'";
	int deleteNum = dmdb->exec_sql(deleteRjzjh.c_str());
	cout<<"删除"<<deleteNum<<"条记录"<<endl;

	deleteNum = dmdb->exec_sql(deleteRjzjh288.c_str());
	cout<<"删除"<<deleteNum<<"条记录"<<endl;

	char insertSqlStr[SQLSTRLEN];

	long numInsert96 = 0;
	long numInsert288 = 0;
	double *mw288 = (double *)malloc(sizeof(double)*sdnum*3);
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		for(int i=1;i<=sdnum;i++)
		{

			sprintf(insertSqlStr,"INSERT INTO HLJJHDB.HLJJHDB.RJZJH(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) VALUES( "
					"'%s','%s','%d','%s','%s','%f','%f','%f','%f','%f')",	strDate.c_str(),sd2sj[i-1].c_str(),i,mp->id,
					mp->descr,mp->mwmax,mp->mwmin,mp->mwup[i],
					mp->mwdn[i],mp->mw[i]);
			printf(insertSqlStr);
			int ret = dmdb->exec_sql(insertSqlStr);
			if(ret!=0)
			{
				cout<<"插入数据出错，错误代码："<<ret<<endl;
				exit(106);
			}else
				numInsert96++;
		}
		memset(mw288,0,sizeof(double)*sdnum*3);
		mpfun(mw288,mp->mw,sdnum);
		for(int i=sd1;i<=sdnum*3;i++)
		{
			sprintf(insertSqlStr,"INSERT INTO HLJJHDB.HLJJHDB.RJZJH288(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) VALUES( "
					"'%s','%s','%d','%s','%s','%f','%f','%f','%f','%f')",	strDate.c_str(),sd2sj288[i-1].c_str(),i,mp->id,
					mp->descr,mp->mwmax,mp->mwmin,mp->mwmax-mw288[i-1],
					mw288[i-1]-mp->mwmin,mw288[i-1]);
			printf(insertSqlStr);
			int ret = dmdb->exec_sql(insertSqlStr);
			if(ret!=0)
			{
				cout<<"插入数据出错，错误代码："<<ret<<endl;
				exit(106);
			}else
				numInsert288++;
		}
		mp=mp->next;
	}

	mp=balanceMicData;
	while(NULL!=mp)
	{
		for(int i=sd1;i<=sdnum;i++)
		{
			sprintf(insertSqlStr,"INSERT INTO HLJJHDB.HLJJHDB.RJZJH(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) VALUES( "
					"'%s','%s','%d','%s','%s','%f','%f','%f','%f','%f')",	strDate.c_str(),sd2sj[i-1].c_str(),i,mp->id,
					mp->descr,mp->mwmax,mp->mwmin,mp->mwup[i],
					mp->mwdn[i],mp->mw[i]);
			printf(insertSqlStr);
			int ret = dmdb->exec_sql(insertSqlStr);
			if(ret!=0)
			{
				cout<<"插入数据出错，错误代码："<<ret<<endl;
				exit(106);
			}else
				numInsert96++;
		}

		memset(mw288,0,sizeof(double)*sdnum*3);
		mpfun(mw288,mp->mw,sdnum);
		for(int i=sd1;i<=sdnum*3;i++)
		{
			sprintf(insertSqlStr,"INSERT INTO HLJJHDB.HLJJHDB.RJZJH288(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) VALUES( "
					"'%s','%s','%d','%s','%s','%f','%f','%f','%f','%f')",	strDate.c_str(),sd2sj288[i-1].c_str(),i,mp->id,
					mp->descr,mp->mwmax,mp->mwmin,mp->mwmax-mw288[i-1],
					mw288[i-1]-mp->mwmin,mw288[i-1]);
			printf(insertSqlStr);
			int ret = dmdb->exec_sql(insertSqlStr);
			if(ret!=0)
			{
				cout<<"插入数据出错，错误代码："<<ret<<endl;
				exit(106);
			}else
				numInsert288++;
		}

		mp = mp->next;
	}
	cout<<"96点计划插入"<<numInsert96<<"条数据"<<endl;
	cout<<"288点计划插入"<<numInsert288<<"条数据"<<endl;
}



/**
 * 将时间信息转化为开停机数组
 * 例如：
 * startDateTime:2017-07-17 03:00
 * endDateTime:2017-07-30 24:00
 * 2017-07-17
 * ktjnum为96
 * *****0位置空闲不用
 * 1到7为开机设置为1
 * 8到96为关机设置为0
 */
void convertAndSetKtj(string startDateTime,string endDateTime,string strDate,long ktj[],int ktjnum)
{
	string startDate = startDateTime.substr(0,10);
	string startTime = startDateTime.substr(11,5);

	string endDate = endDateTime.substr(0,10);
	string endTime = endDateTime.substr(11,5);

	int pointStart = sj2sd(startTime,ktjnum);
	int pointEnd = sj2sd(endTime,ktjnum);
	//初始化
	for(int i=1;i<=ktjnum;i++){
		ktj[i]=1;
	}
	//停机包括在内
	if(startDate.compare(strDate)<0&&endDate.compare(strDate)>0)
	{
		for(int i=1;i<=ktjnum;i++){
			ktj[i]=0;
		}
	}else if(startDate.compare(strDate)==0&&endDate.compare(strDate)==0)//这种情况不存在,开机转停机再转开机
	{
		for(int i=pointStart;i<=pointEnd;i++)
		{
			ktj[i] = 0;
		}
	}else if(startDate.compare(strDate)==0)//由开机转停机
	{
		for(int i=pointStart;i<=ktjnum;i++)
			ktj[i] = 0;

	}else if(endDate.compare(strDate)==0)//由停机转开机
	{
		for(int i=1;i<=pointEnd;i++)
			ktj[i] = 0;
	}
}
/**
 * 打印算法输入数据
 */
void printAllDate()
{
	cout<<"fhycxzGloble:";
	for(int i=1;i<SDNUM;i++)
	{
		cout<<i<<":"<<fhycxzGloble[i]<<",";
	}

	cout<<"llxjhGloble:";
	for(int i=1;i<SDNUM;i++)
	{
		cout<<i<<":"<<llxjhGloble[i]<<",";
	}
	cout<<endl;
	cout<<"xnyycGloble:";
	for(int i=1;i<SDNUM;i++)
	{
		cout<<i<<":"<<xnyycGloble[i]<<",";
	}
	cout<<endl;
	cout<<"sdjhGloble:";
	for(int i=1;i<SDNUM;i++)
	{
		cout<<i<<":"<<sdjhGloble[i]<<",";
	}
	cout<<endl;

	cout<<"sdjhGloble:";
	for(int i=1;i<SDNUM;i++)
	{
		cout<<i<<":"<<sdjhGloble[i]<<",";
	}
	cout<<endl;
	cout<<"zuizhong:";
	for(int i=1;i<SDNUM;i++)
	{
		cout<<i<<":"<<wload0[i]<<",";

	}
	cout<<endl;
	struct micstr *mp = micData;
	double clxx = 0;
	while(mp!=NULL)
	{
		cout<<mp->id<<endl;
		cout<<mp->descr<<endl;
		cout<<"wmmin："<<mp->mwmin<<",wmmax:"<<mp->mwmax<<endl;
		cout<<"PNT:"<<mp->pntNum<<",point:("<<mp->pnt[1].x<<","<<mp->pnt[1].y<<"),("<<mp->pnt[2].x<<","<<mp->pnt[2].y<<")"<<endl;
		cout<<"id:"<<mp->id<<",name:"<<mp->descr<<": ";
		for(int i=1;i<=SDNUM;i++)
		{
			cout<<mp->stat[i]<<" ";
		}
		cout<<endl<<"rampdn:";
		for(int i=0;i<=rampSd;i++)
		{
			cout<<mp->rampdn[i]<<" ";
		}
		cout<<endl<<"rampup:";
		for(int i=0;i<=rampSd;i++)
		{
			cout<<mp->rampup[i]<<" ";
		}
		cout<<endl<<"出力：";
		for(int i=0;i<=rampSd;i++)
		{
			cout<<mp->mw[i]<<" ";
		}
		cout<<endl;
		cout<<endl;
		cout<<endl;
		cout<<endl;
		cout<<endl;

		mp = mp->next;
	}
	mp = micData;
	while(mp!=NULL)
	{
		if(mp->stat[2]!=0)
		{

			clxx+=mp->mwmin;
			cout<<mp->descr<<"	"<<mp->mwmin<<endl;
		}
		mp=mp->next;
	}
	cout<<"2时段："<<clxx<<endl;

	cout<<"-----------------------------------------------------"<<endl;;
	cout<<"balance data"<<endl;
	mp=balanceMicData;
	while(NULL!=mp)
	{
		cout<<mp->id<<endl<<mp->descr<<endl;
		cout<<"mwmax:"<<mp->mwmax<<",mwmin:"<<mp->mwmin<<endl;
		for(int i=1;i<sdnum;i++)
		{
			cout<<mp->mw[i]<<",";
		}
		cout<<endl;
		mp = mp->next;
	}
}
int main(long argc,char **argv)
{
	string strDate = argv[1];

	readConfig();
	dmdb = new CDmDb(userNameStr.c_str(),passwordStr.c_str(),ipStr.c_str(),logPathStr.c_str());

	int initRet = dmdb->init_database();
	if(initRet!=0)
	{
		cout<<"####数据库初始化失败"<<endl;
		exit(99);
	}

	bid_start("sched02");
	readAllDataFromDb(strDate);

	printAllDate();
//	bid_readdat ();
	bid_dataprep();
	bid_sched   ();
	compensateLoad();
	writeToDb(strDate.c_str());
	bid_report  ();
	////////////////////////////////////////////////////////////
	delete(dmdb);
	printf("\n\n");
	printf("####算法完成\n");
	return 0;
}
//end of file
