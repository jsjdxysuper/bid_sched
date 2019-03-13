#include<string>
#include<iostream>
#include"operate_config.h"
#include"common.h"
#include"psched.h"
#include"dmdb.h"
using namespace::std;

CDmDb *dmdbl;
#define POINT96 100
#define POINT288 300
#define SQLLEN 4086

#define RJZ "rjz"
#define DDL "ddl"
#define DQX "dqx"


string userNameStr;
string passwordStr;
string ipStr;
string logPathStr;
char Config[]="db.ini";
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

struct FdjhStru
{
	char jzmc[200];
	char jzid[200];
	char rq[15];
	int sd;
	char sj[15];
	double gl[100];
	double sx[100];
	double xx[100];
	double skt[100];
	double xkt[100];
	struct FdjhStru* next;
};
struct FdjhStru *fdjh96PointStrLink;
void readJzjh96Point(string lx,string strDate)
{
	string rjzjhSql;
	if(lx.compare(RJZ)==0)
		rjzjhSql= "select rq,sd,sid,sname,gl,sx,xx,skt,xkt from HLJJHDB.HLJJHDB.RJZJH "
			"where rq='"+strDate+"' order by sid,sd";
	else if(lx.compare(DDL)==0)
		rjzjhSql= "select rq,sd,sid,sname,gl,sx,xx,skt,xkt from HLJJHDB.HLJJHDB.DDLJH "
					"where rq='"+strDate+"' order by sid,sd";
	else if(lx.compare(DQX)==0)
		rjzjhSql = "select rq,sd,sid,sname,gl from HLJJHDB.HLJJHDB.DQXJH "
				"where rq='"+strDate+"' order by sid,sd";

	dmdbl->exec_select(rjzjhSql.c_str(),9,"日机组计划96点");
	long rowNum = dmdbl->get_row_num();
	if(rowNum%96!=0||rowNum/96<1)
	{
		cerr<<"####96点计划数据有问题"<<endl;

		exit(-1);
	}
	fdjh96PointStrLink = (struct FdjhStru*)malloc(sizeof(struct FdjhStru));
    long linkPointNum = 1;

	for(int i=0;i<rowNum/96;i++)
	{
		for(int j=1;j<=96;j++)
		{
			long rowIndex = i*96+j-1;
			sprintf(fdjh96PointStrLink->rq,"%s",strDate.c_str());

			fdjh96PointStrLink->sd = atoi(dmdbl->get_select_data(rowIndex,1).c_str());
			if(j!=fdjh96PointStrLink->sd)
			{
				cerr<<"####96点计划数据有问题"<<endl;
				exit(-1);
			}
			if(j==1)
			{
				sprintf(fdjh96PointStrLink->jzid,"%s",dmdbl->get_select_data(rowIndex,2).c_str());
				sprintf(fdjh96PointStrLink->jzmc,"%s",dmdbl->get_select_data(rowIndex,3).c_str());
			}
//			fdjh96PointStrLink->sx[j] = atof(dmdbl->get_select_data(rowIndex,4).c_str());
//			fdjh96PointStrLink->xx[j] = atof(dmdbl->get_select_data(rowIndex,5).c_str());
//			fdjh96PointStrLink->skt[j] = atof(dmdbl->get_select_data(rowIndex,6).c_str());
//			fdjh96PointStrLink->xkt[j] = atof(dmdbl->get_select_data(rowIndex,7).c_str());
			fdjh96PointStrLink->gl[j] = atof(dmdbl->get_select_data(rowIndex,4).c_str());
		}

		if(i+1==rowNum/96)
			break;
		linkPointNum++;
		struct FdjhStru *point = (struct FdjhStru*)malloc(sizeof(struct FdjhStru));
		point->next = fdjh96PointStrLink;
		fdjh96PointStrLink = point;
	}

	struct FdjhStru*pp = fdjh96PointStrLink;
	while(pp!=NULL)
	{
		cout<<"机组名称："<<pp->jzmc<<",机组id："<<pp->jzid<<",日期："<<pp->rq<<endl;
		cout<<"功率：";
		for(int i=1;i<=96;i++)
		{
			cout<<pp->gl[i]<<",";
		}
		cout<<endl;
		pp = pp->next;
	}
}

void writeJzjh288Point(string lx,string strDate)
{
	string deleteSql;
	if(lx.compare(RJZ)==0)
		deleteSql = "DELETE FROM HLJJHDB.HLJJHDB.RJZJH288 WHERE rq='"+strDate+"'";
	else if(lx.compare(DDL)==0)
		deleteSql = "DELETE FROM HLJJHDB.HLJJHDB.DDLJH288 WHERE rq='"+strDate+"'";
	else if(lx.compare(DQX)==0)
		deleteSql = "DELETE FROM HLJJHDB.HLJJHDB.DQXJH288 WHERE rq='"+strDate+"'";
	int fluRowNum = dmdbl->exec_sql(deleteSql.c_str());

	cout<<"删除日机组计划"<<fluRowNum<<"条"<<endl;


	long insertNum = 0;
	struct FdjhStru *pp = fdjh96PointStrLink;
	while(pp!=NULL)
	{
		double jh288Point[POINT288];
		memset(jh288Point,0,sizeof(double)*POINT288);
		mpfun(jh288Point,pp->gl,96);
		for(int i=1;i<=288;i++)
		{

			stringstream ss;
			if(lx.compare(RJZ)==0)
				ss<<"INSERT INTO HLJJHDB.HLJJHDB.RJZJH288(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) VALUES('"<<strDate<<"','"<<sd2sj288[i-1]<<"','"<<i<<"','"<<pp->jzid<<"','"
					<<pp->jzmc<<"','"<<0<<"','"<<0<<"','"<<0<<"','"<<0<<"','"<<jh288Point[i]<<"')";
			else if(lx.compare(DDL)==0)
				ss<<"INSERT INTO HLJJHDB.HLJJHDB.DDLJH288(RQ,SJ,SD,SID,SNAME,SX,XX,SKT,XKT,GL) VALUES('"<<strDate<<"','"<<sd2sj288[i-1]<<"','"<<i<<"','"<<pp->jzid<<"','"
									<<pp->jzmc<<"','"<<0<<"','"<<0<<"','"<<0<<"','"<<0<<"','"<<jh288Point[i]<<"')";
			else if(lx.compare(DQX)==0)
				ss<<"INSERT INTO HLJJHDB.HLJJHDB.DQXJH288(RQ,SJ,SD,SID,SNAME,GL) VALUES('"<<strDate<<"','"<<sd2sj288[i-1]<<"','"<<i<<"','"<<pp->jzid<<"','"
													<<pp->jzmc<<"','"<<jh288Point[i]<<"')";
			cout<<ss.str()<<endl;
			int ret = dmdbl->exec_sql(ss.str().c_str());
			if(ret==0)
				insertNum++;

		}
		pp = pp->next;
	}
	cout<<"####插入288点计划"<<insertNum<<"条"<<endl;
}
int main(int argc,char *argv[]){
	string lx;
	lx = argv[1];
	if(strcmp(argv[1],DDL)==0)
	{
		cout<<"生成定电量288点计划"<<endl;
	}else if(strcmp(argv[1],"dqx")==0)
	{
		cout<<"生成定曲线288点计划"<<endl;
	}else if(strcmp(argv[1],RJZ)==0)
	{
		cout<<"生成日机组288点计划"<<endl;
	}else
	{
		cerr<<"####输入参数不正确"<<endl;
		return -1;
	}

	readConfig();

	dmdbl = new CDmDb(userNameStr.c_str(),passwordStr.c_str(),ipStr.c_str(),logPathStr.c_str());
	dmdbl->init_database();

	readJzjh96Point(lx,argv[2]);
	writeJzjh288Point(lx,argv[2]);

	struct FdjhStru *pp = fdjh96PointStrLink;
	while(pp!=NULL)
	{
		struct FdjhStru *tp;
		tp = pp;
		pp = pp->next;
		free(tp);
	}
	dmdbl->close_database();
	printf("####算法成功完成!\n");
}
