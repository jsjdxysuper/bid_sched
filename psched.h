//sched.h by wanggt
////////////////////////////////////////////////////////////
#ifndef _PSCHED_H
#define _PSCHED_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<time.h>
#define  FIXBAS  1 //type
#define  FIXCRV  2
#define  FIXMTH  3
#define  HYDRO   4
#define  PLANT   5
////////////////////////////////////////////////////////////

struct fixstr//fixed mwh
{
	struct fixstr *next;
	char   descr[100];
	char   id   [100];
	long   i;
	double puf;
	double mwh0;      //mwh given
	double mwh;       //mwh
	double mwmax[100];//mw max
	double mwmin[100];//mw min
	double mwdn [100];//space/down
	double mwup [100];//space/up
	double mw   [100];//sched/mw
};
////////////////////////////////////////////////////////////

struct micstr//micro-inc
{
	struct micstr *next;
	char   descr[100];
	char   id   [100];
	long   i;
	double mwmax;
	double mwmin;
	double mwdn [100];//space/down
	double mwup [100];//space/up
	double mw   [100];//sched/mw
	double mwh;
	double wt;//weight
	double agcmax;
	double agcmin;
	long   agcStat[100];//status/agc
	long   stat   [100];//status
	long   rampFlag;    //ramp flag
	double rampdn [100];//ramp curve
	double rampup [100];
	PNTSTR pnt    [100];//(mw,price)
	long   pntNum;
}; 
////////////////////////////////////////////////////////////
//function prototypes
double mp1fun(double mw,double mw2,double mp0);
double mp2fun(double mw,double mw2,double mp0);
double mp3fun(double mw,double mw2,double mp0);
double micSched(double *mw,double *wload,long sd);
double micmx_fun();
double micmn_fun();
double mwhfun(double *mw,long sdnum);
double mwfun (double mwh,long sdnum);
////////////////////////////////////////////////////////////

bool isrampUp(long *stat,long sd);
bool isrampDn(long *stat,long sd);
long possd (double *wload,double *wgen,double *mwmax,long sd1,long sdnum);
long possd2(double *wload,double *pump,double mwmax,long sd1,long sdnum);
long typefun(char *descr);
long typefun(char *descr,long type);
////////////////////////////////////////////////////////////

void bid_datadump();
void bid_dataprep();
void bid_readdat ();
void bid_report  ();
void bid_sched   ();
void bid_start(char *case_str);
void filter(double *v,long n);
void hsegfun(double *sched,double *maxSched,double *minSched,double wload);
void hsegNext(double &mic,struct micstr *micData);
void maxSched_fun(double *maxSched,double *minSched,double mic,long sd);
////////////////////////////////////////////////////////////

void rampFlag_fun(struct micstr *micData,long sd);
void rampSched();
void rampup_crv(double *mwmin,double *ramp,long sd1);
void rampdn_crv(double *mwmin,double *ramp,long sd1);
void rampup_fun(double *mwmin,long sdnum);
void rampdn_fun(double *mwmin,long sdnum);
void read_global(FILE *fp);
void read_fixbas(FILE *fp);
void read_pnt   (FILE *fp,PNTSTR *pnt);
void read_mic   (FILE *fp);
void read_wload (FILE *fp);
void report_syssum(FILE *fp);
void report(FILE *fp,struct fixstr *_fixData);
void report(FILE *fp,struct micstr *micData);
////////////////////////////////////////////////////////////

void sched_dataprep();
void sched_datadump();
void sched_readdat();
void sched_report ();
void sched_gross  ();
void sched_fun(struct fixstr *_fixData);
void sched_start(char *case_str);
void update(struct micstr *micData,double *mw,long sd);
void unitSched (double *wgen,double *wload,long sd1,long sdnum,struct fixstr *fp);
void unitSched2(double *pump,double *wload,long sd1,long sdnum);
////////////////////////////////////////////////////////////
//globals
extern struct fixstr *fixbasData;
extern struct fixstr *fixcrvData;
extern struct fixstr *fixmthData;
extern struct fixstr *hydroData;
extern struct fixstr *plantData;
extern struct micstr *micData;
////////////////////////////////////////////////////////////

extern long fixbasNum;
extern long fixcrvNum;
extern long fixmthNum;
extern long hydroNum;
extern long plantNum;
extern long pntNum;
extern long micNum;
extern long sd1;
extern long sdnum;
extern long rampSd;
extern long rampSd_hydro;
extern long rampSd_plant;
extern long rampSd_unit;
////////////////////////////////////////////////////////////

extern double grossFixbas[100];
extern double grossFixcrv[100];
extern double grossFixmth[100];
extern double grossHydro [100];
extern double grossPlant [100];
extern double grossFix   [100];//fixed-mwh
extern double grossMic   [100];//micro-inc
extern double wload0     [100];//system loads
extern double wload      [100];
extern double micPrice   [100];
extern double mwStep;
////////////////////////////////////////////////////////////

#endif
