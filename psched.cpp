//sched.cpp by wanggt
////////////////////////////////////////////////////////////
#include"common.h"
#include"psched.h"
#include"math.h"

//globals
struct fixstr *fixbasData;
struct fixstr *fixcrvData;
struct fixstr *fixmthData;
struct fixstr *hydroData;
struct fixstr *plantData;
struct micstr *micData;
////////////////////////////////////////////////////////////

long fixbasNum;
long fixcrvNum;
long fixmthNum;
long hydroNum;
long plantNum;
long pntNum;
long micNum;
long sd1;
long sdnum;
long rampSd;
long rampSd_hydro;
long rampSd_plant;
long rampSd_unit;
////////////////////////////////////////////////////////////

double grossFixbas[100];
double grossFixcrv[100];
double grossFixmth[100];
double grossHydro [100];
double grossPlant [100];
double grossFix   [100];//fixed-mwh
double grossMic   [100];//micro-inc
double wload0     [100];//system loads
double wload      [100];
double micPrice   [100];
double mwStep;
////////////////////////////////////////////////////////////
/**
 * use the weight to resched the power
 * mw0 is the init power without weight(justice divide)
 */
void wtSched(double *mw,double *mw0,long micNum,long sd)
{
	double sum_upSpace=0;//wt==1
	double sum_dnSpace=0;
	double sum_dmw = 0.0;//wt!=1,
	struct micstr *mp=micData;
	for(long i=1;i<=micNum;i++,mp=mp->next)
	if(mp->rampFlag==0)////////////////////////////////////不是开停机才能调整
	{
		if(fabs(mp->wt-1.0)<SMLL)
		{
			sum_upSpace+= mp->mwmax-mw0[i];
			sum_dnSpace-= mp->mwmin-mw0[i];
		}
		else//if(fabs(mp->wt-1.0)>SMLL)
		{
			mw[i]=mw0[i]*mp->wt;
			mw[i]=min(mw[i],mp->mwmax);
			mw[i]=max(mw[i],mp->mwmin);
			sum_dmw+= mw[i]-mw0[i];
		}
	}
	////////////////////////////////////////////////////////////

	printf("\n sum_upSpace=%lf",sum_upSpace);
	printf("\n sum_dnSpace=%lf",sum_dnSpace);
	printf("\n sum_dmw    =%lf",sum_dmw);
	mp=micData;
	for(long i=1;i<=micNum;i++,mp=mp->next)
	if(mp->rampFlag==0)/////////////不是开停机才能调整
	{
		if(fabs(mp->wt-1.0)<SMLL)
		{
			if(sum_dmw>SMLL) mw[i]=mw0[i]+sum_dmw*(mp->mwmin-mw0[i])/sum_dnSpace;
			else             mw[i]=mw0[i]-sum_dmw*(mp->mwmax-mw0[i])/sum_upSpace;
		}
	}
	////////////////////////////////////////////////////////////

	return;
}
////////////////////////////////////////////////////////////

void sched_fun(struct fixstr *_fixData)
{
	struct fixstr *fp=_fixData;
	while(fp!=NULL)
	{
		unitSched(fp->mw,wload,sd1,sdnum,fp);
		fp->mwh=mwhfun(fp->mw,sdnum);
		for(long k=1;k<=sdnum;k++)
		{
			fp->mwup[k]=fp->mwmax[k]-fp->mw[k];
			fp->mwdn[k]=fp->mw[k]-fp->mwmin[k];
		}	fp=fp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

/**
 *para:
 *para: mw ouput result,the length of mw[] is micNum
 *para: wload input space
 */
double micSched(double *mw,double *wload,long sd)
{
//wload --- system load
//sd    --- current sd
//mw[]  --- micro-inc sched, returned
//ret   --- micro-inc price
////////////////////////////////////////////////////////////

	rampFlag_fun(micData,sd);
	struct micstr *mp=micData;
	for(long i=1;i<=micNum;i++,mp=mp->next)//-1-ramping sd
	{
		if(mp->rampFlag==1) mw[i]=mp->mw[sd];
		else                mw[i]=mp->mwmin;
	}
	////////////////////////////////////////////////////////////

	double micmx=micmx_fun();
	double micmn=micmn_fun();
	double maxSched[1000];
	double minSched[1000];
//	maxSched_fun(maxSched,minSched,micmx,sd);//-2-max/minSched
	maxSched_fun(maxSched,minSched,micmn,sd);
	if(wload[sd]<minSched[0])
	{
		printf("\nError --- %ld",__LINE__);
		printf("\n wload   =%lf, sd=%ld",wload[sd],sd);
		printf("\n minSched=%lf",minSched[0]);
		exit(0);
	}
	////////////////////////////////////////////////////////////

	double mic=micmn;//init
	while(1)//-3-hseg search
	{
		maxSched_fun(maxSched,minSched,mic,sd);
		printf("\n sd=%ld, mic/price=%lf",sd,mic);
		if(islarge(mic)){ printf("\nWarning --- %ld",__LINE__); break; }
		if(interv(wload[sd],maxSched[0],minSched[0]))
		{
			hsegfun(mw,maxSched,minSched,wload[sd]);
			return mic;
		}
		copy(mw,minSched,micNum);//current hseg
		hsegNext(mic,micData);   //next
	}
	////////////////////////////////////////////////////////////

	return 0;
}

void bid_sched()
{
	rampSched();
	double mw0[1000];
	double mw[1000];
	for(long sd=sd1;sd<=sdnum;sd++)
	{
		micPrice[sd] = micSched(mw0,wload,sd);//mw,price
		wtSched(mw,mw0,micNum,sd);

		grossMic[sd] = sum(mw,micNum);       //mw[0]
		wload   [sd]-= sum(mw,micNum);
		update(micData,mw,sd);
	}
	////////////////////////////////////////////////////////////

	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		mp->mwh=mwhfun(mp->mw,sdnum);
		mp=mp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

void unitSched2(double *pump,double *wload,long sd1,long sdnum)
{
//pump  --- sched of pump, returned
//wload --- system load modified
//sd1   --- sd start
//sdnum --- sd num
////////////////////////////////////////////////////////////

	initVector(pump,sd1,sdnum);
	double mwmax=12;//MW
	double mwh0 =60;//MWh
	while(mwhfun(pump,sdnum)<mwh0)
	{
		long k=possd2(wload,pump,mwmax,sd1,sdnum);
		if(k==NO){ printf("\nWarning --- %ld",__LINE__); break; }

		double mwinc=min(mwStep,mwmax-pump[k]);
		if(mwinc<SMLL){ printf("\nWarning --- %ld",__LINE__); break; }
		pump [k]+= mwinc;
		wload[k]+= mwinc;
	}
	////////////////////////////////////////////////////////////

	return;
}

/**
*
*para:
*wgen fixstr's power
*wload power system load
*sd1 start segment number
*sdsum end segment number
*fp fixstr data link
*/
void unitSched(double *wgen,double *wload,long sd1,long sdnum,struct fixstr *fp)
{
//wgen  --- sched of unit, returned
//wload --- system load modified
//sd1   --- sd start
//sdnum --- sd num
//fp    --- fixed-mwh pointer
////////////////////////////////////////////////////////////

	double mwhmax=mwhfun(fp->mwmax,sdnum);//max energy one day
	double mwhmin=mwhfun(fp->mwmin,sdnum);//min energy one day
	if(!interv(fp->mwh0,mwhmax,mwhmin))
	{
		printf("\nWarning --- %ld",__LINE__);
		printf("\n mwhmax=%lf, mwh0=%lf",mwhmax,fp->mwh0);
		printf("\n mwhmin=%lf",mwhmin);
	}
	////////////////////////////////////////////////////////////

	copy(wgen,fp->mwmin,sdnum);
	subtrfun(wload,wgen,sdnum);
	while(mwhfun(wgen,sdnum)<fp->mwh0)
	{
		long k=possd(wload,wgen,fp->mwmax,sd1,sdnum);
		if(k==NO){ printf("\nWarning --- %ld",__LINE__); break; }

		double mwinc=min(mwStep,fp->mwmax[k]-wgen[k]);
		if(mwinc<SMLL){ printf("\nWarning --- %ld",__LINE__); break; }
		wgen [k]+= mwinc;
		wload[k]-= mwinc;
	}
	////////////////////////////////////////////////////////////

	return;
}

void sched_dataprep()
{
	struct fixstr *fp=fixbasData;
	while(fp!=NULL)
	{
		for(long i=1;i<=sdnum;i++)
		{
			fp->mwmax[i]=fp->mwmax[0];//init
			fp->mwmin[i]=fp->mwmin[0];
		}
		rampdn_fun(fp->mwmax,sdnum);
		rampdn_fun(fp->mwmin,sdnum);
		rampup_fun(fp->mwmax,sdnum);
		rampup_fun(fp->mwmin,sdnum); fp=fp->next;
	}
	////////////////////////////////////////////////////////////

	double wload_max=fmax(wload,sdnum);
	double wload_min=fmin(wload,sdnum);
	printf("\n wload_max=%lf",wload_max);
	printf("\n wload_min=%lf",wload_min);
	////////////////////////////////////////////////////////////

	return;
}

void report_syssum(FILE *fp)
{
	fprintf(fp,"\nSystem Summary");
	fprintf(fp,"\n wload_mwh=%lf",mwhfun(wload0,sdnum));
	fprintf(fp,"\n wload_max=%lf",fmax(wload0,sdnum));
	fprintf(fp,"\n wload_min=%lf",fmin(wload0,sdnum));
	fdrawline(fp,55);
	fprintf(fp,"\n sd |wload0 |wload |wgen |price");
	fdrawline(fp,55);
	for(long i=1;i<=sdnum;i++)
	fprintf(fp,"\n %4ld %lf %lf %lf %lf",i,wload0[i],wload[i],grossMic[i],micPrice[i]);
//	fprintf(fp,"\n %4ld %lf %lf %lf",i,wload0[i],wload[i],grossFixbas[i]);
	fdrawline(fp,55);
	////////////////////////////////////////////////////////////

	return;
}

/**
 * in the price mic,in sd time segment,calculate the max power and the min power
 */
void maxSched_fun(double *maxSched,double *minSched,double mic,long sd)
{
//maxSched - max sched/sum, returned
//minSched - min sched/sum, returned
//mic    --- current price
//sd     --- current sd
////////////////////////////////////////////////////////////

	double micbuf[100];
	struct micstr *mp=micData;
	for(long i=1;i<=micNum;i++,mp=mp->next)
	{
		if(mp->rampFlag==1)
		{
			maxSched[i]=mp->mw[sd]; //-1-ramping sd
			minSched[i]=mp->mw[sd]; continue;
		}
		/////////////////////////////////////////////////////////////
		if(mp->stat[sd]==0)
		{
			maxSched[i] = 0;
			minSched[i] = 0; continue;
		}
		////////////////////////////////////////////////////////////
		long n=mp->pntNum; getpnt_y(micbuf,n,mp->pnt);
		long k=lookup2(mic,micbuf,n);//-2-pos.segment
		if(k==NO)
		{
			if(mic>micbuf[n]) k=n-1;
			if(mic<micbuf[1]) k=1;
		}
		maxSched[i]=mp->pnt[k+1].x; //-3-max/min sched
		minSched[i]=mp->pnt[k].x;
	}
	////////////////////////////////////////////////////////////

	maxSched[0]=sum(maxSched,micNum);//-4-sum.stat
	minSched[0]=sum(minSched,micNum);
	////////////////////////////////////////////////////////////

	return;
}

void bid_dataprep(){}
void bid_readdat()
{
	FILE *fp=fopen(datfile,"r");
	if(fp==NULL){ printf("\nCannot open %s",datfile); exit(0); }
	read_global(fp);
	read_wload (fp);
	read_mic   (fp);
	fclose     (fp);
    ////////////////////////////////////////////////////////////

	return;
}

void bid_report()
{
	FILE *fp=fopen(resfile,"w");
	if(fp==NULL){ printf("\nCannot open %s",resfile); exit(0); }
	getdate_time(end_time);
	fprintf(fp,"\nStart_time=%s",start_time);
	fprintf(fp,"\nEnd_time  =%s",end_time);
	report_syssum(fp);
	report(fp,micData);
	fclose(fp);
	////////////////////////////////////////////////////////////

	return;
}

/**
 * find a delta (micro up) price
 */
void hsegNext(double &mic,struct micstr *micData)
{
	double micNext=LARGE;//init
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		for(long i=1;i<mp->pntNum;i++)
		if(mp->pnt[i].y>mic+SMLL)
		micNext=min(micNext,mp->pnt[i].y);
		mp=mp->next;
	}
	////////////////////////////////////////////////////////////

	mic=micNext;
	return;	
}

/**
 * judge whether ervery gen's sd is a rampUp or rampDn sd
 */
void rampFlag_fun(struct micstr *micData,long sd)
{
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		mp->rampFlag=0;//init
		if(isrampUp(mp->stat,sd)) mp->rampFlag=1;
		if(isrampDn(mp->stat,sd)) mp->rampFlag=1;
		mp=mp->next;
	}
	////////////////////////////////////////////////////////////
	
	return;
}

/**
 * put run->stop curve,stop->run curve inject into mp->mw[]
 */
void rampSched()
{
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		for(long i=1;i<sdnum;i++)
		{
			if(mp->stat[i]==0 && mp->stat[i+1]==1)
			for(long k=1;k<=rampSd;k++)
				mp->mw[i+k-1]=min(mp->mwmin,mp->rampup[k]);
		}
		////////////////////////////////////////////////////////////
		
		for(long i=1;i<=sdnum;i++)
		{
			if(mp->stat[i]==0 && mp->stat[i-1]==1)
			for(long k=1;k<=rampSd;k++)
			mp->mw[i+k-rampSd]=min(mp->mwmin,mp->rampdn[k]);
		}	mp=mp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

/**
 * after sched, update the micstr's mw,mwup,mwdn
 */
void update(struct micstr *micData,double *mw,long sd)
{
	struct micstr *mp=micData;
	for(long i=1;i<=micNum;i++,mp=mp->next)
	{
		mp->mw  [sd]=mw[i];
		mp->mwup[sd]=0;
		mp->mwdn[sd]=0;
		if(!isrampUp(mp->stat,sd))
		if(!isrampDn(mp->stat,sd))
		{
			mp->mwup[sd]=mp->mwmax-mp->mw[sd];
			mp->mwdn[sd]=mp->mw[sd]-mp->mwmin;
		}
	}
	////////////////////////////////////////////////////////////

	return;
}
/**
 *use the :by a ratio of (max power - min power)  to divide the residue space
 */
void hsegfun(double *sched,double *maxSched,double *minSched,double wload)
{
//sched    --- sched returned
//wload    --- system load
//maxSched --- max sched
//minSched --- min sched
////////////////////////////////////////////////////////////

	double wt[1000];
	for(long i=1;i<=micNum;i++)
		wt[i]=maxSched[i]-minSched[i];
	double wtsum=sum(wt,micNum);
	double wtval=(wload-sum(minSched,micNum))/wtsum;
	for(long i=1;i<=micNum;i++)
	sched[i]=minSched[i]+wt[i]*wtval;
	sched[0]=sum(sched,micNum);
	////////////////////////////////////////////////////////////

	return;
}

/**
 *judge whether sd in stop->run curve section,[]close section,ie include border
 */
bool isrampUp(long *stat,long sd)
{
//stat --- status (0/1)
//sd   --- sd
////////////////////////////////////////////////////////////

	for(long i=1;i<sdnum;i++)
	if(stat[i]==0 && stat[i+1]==1)
	if(interv(sd,i,i+rampSd)) return true;
	////////////////////////////////////////////////////////////

	return false;
}

bool isrampDn(long *stat,long sd)
{
	for(long i=1;i<=sdnum;i++)
	if(stat[i]==0 && stat[i-1]==1)
	if(interv(sd,i,i-rampSd)) return true;
	////////////////////////////////////////////////////////////

	return false;
}

void sched_gross()
{
	struct fixstr *fp=fixbasData;
	while(fp!=NULL)
	{
		addVector(grossFixbas,fp->mw,sdnum);
		fp=fp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

void report(FILE *fp,struct fixstr *_fixData)
{
	struct fixstr *xp=_fixData;
	while(xp!=NULL)
	{
		fprintf(fp,"\n num=%ld: %s %s",xp->i,xp->id,xp->descr);
		fprintf(fp,"\n mwh=%lf",xp->mwh); fdrawline(fp,55);
		printf("\n mwh=%lf",xp->mwh);
		fprintf(fp,"\n sd |mwmin |mwdn |mw  |mwup |mwmax |id");
		fdrawline(fp,55);
		for(long i=1;i<=sdnum;i++)
		fprintf(fp,"\n %4ld %lf %lf %lf %lf %lf",i,xp->mwmin[i],xp->mwdn[i],xp->mw[i],xp->mwup[i],xp->mwmax[i]);
		fdrawline(fp,55);
		xp=xp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

void report(FILE *fp,struct micstr *micData)
{
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		fprintf(fp,"\n num=%ld: %s %s",mp->i,mp->id,mp->descr);
		fprintf(fp,"\n mwmax=%lf",mp->mwmax); fdrawline(fp,55);
		printf("\n mwmax=%lf",mp->mwmax);		
		fprintf(fp,"\n sd |mwmax |mwmin |mwup |mwdn |mw  |id");
		fdrawline(fp,55);
		for(long i=1;i<=sdnum;i++)
		fprintf(fp,"\n %4ld %lf %lf %lf %lf %lf",i,mp->mwmax,mp->mwmin,mp->mwup[i],mp->mwdn[i],mp->mw[i]);
		fdrawline(fp,55);
		mp=mp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

void rampup_crv(double *mwmin,double *ramp,long sd1)
{
//mwmin  --- lower limt
//ramp   --- ramp curve
//sd1    --- start sd
////////////////////////////////////////////////////////////

	for(long i=1;i<=rampSd;i++)
	{
		if(ramp[i]>mwmin[0]) break;
		mwmin[sd1-1+i]=ramp[i];
	}
	////////////////////////////////////////////////////////////

	return;
}

void rampdn_crv(double *mwmin,double *ramp,long sd1)
{
	for(long i=1;i<=rampSd;i++)
	{
		if(ramp[i]>mwmin[0]) continue;
		mwmin[sd1-1+i]=ramp[i];
	}
	////////////////////////////////////////////////////////////

	return;
}
 
void rampup_fun(double *mwmin,long sdnum)
{
//mwmin  --- lower limit
//sdnum  --- sd num
////////////////////////////////////////////////////////////

	for(long i=1;i<sdnum;i++)
	if(mwmin[i]<SMLL && mwmin[i+1]>mwStep)
	{
		for(long k=1;k<=rampSd;k++)
			mwmin[i+k]=mwmin[i+rampSd]*k/rampSd;
	}
	////////////////////////////////////////////////////////////

	return;
}

void rampdn_fun(double *mwmin,long sdnum)
{
	for(long i=1;i<=sdnum;i++)
	if(mwmin[i]<SMLL && mwmin[i-1]>mwStep)
	{
		for(long k=1;k<=rampSd;k++)
			mwmin[i-k]=mwmin[i-rampSd]*k/rampSd;
	}
	////////////////////////////////////////////////////////////

	return;
}

long possd2(double *wload,double *pump,double mwmax,long sd1,long sdnum)
{
	double minLoad=LARGE;
	long  imin=NO;
	for(long i=sd1;i<=sdnum;i++)
	{
		if(pump [i]>mwmax-SMLL) continue;
		if(wload[i]<minLoad)
		{
			minLoad=wload[i];
			imin=i;
		}
	}
	////////////////////////////////////////////////////////////

	return imin;
}

/**
 * find the point wgen can rise
 * return the max load index in wload, but not in max wgen index point
 */
long possd(double *wload,double *wgen,double *mwmax,long sd1,long sdnum)
{
	double maxLoad=0;//init
	long  imax=NO;
	for(long i=sd1;i<=sdnum;i++)
	{
		if(wgen [i]>mwmax[i]-SMLL) continue;
		if(wload[i]>maxLoad)
		{
			maxLoad=wload[i];
			imax=i;
		}
	}
	////////////////////////////////////////////////////////////

	return imax;
}

void sched_report()
{
	FILE *fp=fopen(resfile,"w");
	if(fp==NULL){ printf("\nCannot open %s",resfile); exit(0); }
	getdate_time(end_time);
	fprintf(fp,"\nStart_time=%s",start_time);
	fprintf(fp,"\nEnd_time  =%s",end_time);
	report_syssum(fp);
	report(fp,fixbasData);
//	report(fp,fixmthData);
//	report(fp,hydroData);
//	report(fp,plantData);
	fclose(fp);
	////////////////////////////////////////////////////////////

	return;
}

void sched_readdat()
{
	FILE *fp=fopen(datfile,"r");
	if(fp==NULL){ printf("\nCannot open %s",datfile); exit(0); }
	read_global(fp);
	read_fixbas(fp);
	read_wload (fp);
	fclose(fp);
    ////////////////////////////////////////////////////////////

	return;
}

void read_global(FILE *fp)
{
	posdata(fp,"DE GLOBAL/RC");
	while(!feof(fp))
	{
		fgets(buffer,1000,fp);
		if(buffer[0]=='D') break;
		if(strstr(buffer,"mwStep")) mwStep=getfval(buffer);
		if(strstr(buffer,"pntNum")) pntNum=getval(buffer);
		if(strstr(buffer,"rampSd")) rampSd=getval(buffer);
		if(strstr(buffer,"sdnum"))  sdnum =getval(buffer);
		if(strstr(buffer,"sd1"))    sd1   =getval(buffer);
	}
    ////////////////////////////////////////////////////////////

	return;
}

void read_wload(FILE *fp)
{
	posdata(fp,"DE LOADDATA");
	fgets(buffer,1000,fp);
	for(long i=1;i<=sdnum;i++)
	{
		fscanf(fp,"%ld%lf",&ia,&fa);
		wload0[i]=fa;
		wload [i]=fa;
	}
	////////////////////////////////////////////////////////////

	return;
}

void read_fixbas(FILE *fp)
{
	posdata(fp,"DE FIXBAS");
	fgets(buffer,1000,fp);
	fixbasData=NULL;
	fixbasNum =0;
	while(fscanf(fp,"%ld%lf%lf%lf",&ia,&fa,&fb,&fc)==4)
	{
		struct fixstr *xp=(struct fixstr *)malloc(sizeof(struct fixstr));
		if(xp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
		memset(xp,0,sizeof(struct fixstr));
		fgets(buffer,1000,fp);//id
		xp->i       =ia;
		xp->mwmax[0]=fa;
		xp->mwmin[0]=fb;
		xp->mwh0    =fc;
		xp->next    =fixbasData;
		fixbasData=xp;
		fixbasNum++;
	}
    ////////////////////////////////////////////////////////////

	reverse(fixbasData);
	return;
}

void read_mic(FILE *fp)
{
	posdata(fp,"DE MICDATA");
	fgets(buffer,1000,fp);
	micData=NULL;
	micNum =0;
	while(fscanf(fp,"%ld%lf%lf",&ia,&fa,&fb)==3)
	{
		struct micstr *mp=(struct micstr *)malloc(sizeof(struct micstr));
		if(mp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
		memset(mp,0,sizeof(struct micstr));
		mp->i     =ia;
		mp->mwmax =fa;
		mp->mwmin =fb;
		mp->pntNum=pntNum; read_pnt(fp,mp->pnt);
		mp->next  =micData;
		micData   =mp;
		micNum++;
	}
    ////////////////////////////////////////////////////////////

	reverse(micData);
	return;
}

void read_pnt(FILE *fp,PNTSTR *pnt)
{
	for(long i=1;i<=pntNum;i++)
	{
		fscanf(fp,"%lf%lf",&fa,&fb);
		pnt[i].x=fa;//MW
		pnt[i].y=fb;//mic/price
	}
	////////////////////////////////////////////////////////////

	return;
}

double mp1fun(double mw,double mw2,double mp0)
{
//mw  - current sd
//mw2 - next sd
//mp0 - pre-moment
////////////////////////////////////////////////////////////

	double ret=(9*mw+5*mp0-mw2)/13;
	return ret;
}

double mp2fun(double mw,double mw2,double mp0)
{
	double ret=(18*mw-3*mp0-2*mw2)/13;
	return ret;
}

double mp3fun(double mw,double mw2,double mp0)
{
	double ret=(12*mw-2*mp0+3*mw2)/13;
	return ret;
}

void filter(double *v,long n)
{
	for(long i=1;i<n;i++)
	{
		double dv =v[i]-v[i-1];
		double dv2=v[i+1]-v[i];
		if(dv > SMLL)
		if(dv2<-SMLL) v[i]=max(v[i-1],v[i+1]);
		if(dv <-SMLL)
		if(dv2> SMLL) v[i]=min(v[i-1],v[i+1]);
	}
	////////////////////////////////////////////////////////////

	return;
}
/**
 * get the min price
 */
double micmn_fun()
{
	double ret=LARGE;
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		ret=min(ret,mp->pnt[1].y);
		mp=mp->next;
	} 
	////////////////////////////////////////////////////////////

	return ret;
}
/**
 * get the max price
 */
double micmx_fun()
{
	double ret=0;//init
	struct micstr *mp=micData;
	while(mp!=NULL)
	{
		ret=max(ret,mp->pnt[mp->pntNum].y);
		mp=mp->next;
	}
	////////////////////////////////////////////////////////////

	return ret;
}

void bid_start(char *case_str)
{
	getdate_time(start_time);
	sprintf(basfile,"%s.bas",case_str);
	sprintf(datfile,"%s.dat",case_str);
	sprintf(dmpfile,"%s.dmp",case_str);
	sprintf(resfile,"%s.res",case_str);
	////////////////////////////////////////////////////////////

	return;
}

void sched_start(char *case_str)
{
	getdate_time(start_time);
	sprintf(basfile,"%s.bas",case_str);
	sprintf(datfile,"%s.dat",case_str);
	sprintf(dmpfile,"%s.dmp",case_str);
	sprintf(resfile,"%s.res",case_str);
	////////////////////////////////////////////////////////////

	return;
}

long typefun(char *descr,long type)
{
	strcpy(descr,"all energy fixed gens");
	if(type==FIXBAS) strcpy(descr,"fixed energy gen");
	if(type==FIXCRV) strcpy(descr,"fixed curve gen");
	if(type==FIXMTH) strcpy(descr,"month fixed energy");
	if(type==HYDRO)  strcpy(descr,"water gen");
	if(type==PLANT)  strcpy(descr,"fixed energy plant");
	////////////////////////////////////////////////////////////

	return 0;
}

/**
 * get mwh from sdnum and mw[]
 */
double mwhfun(double *mw,long sdnum)
{
	double ret=sum(mw,sdnum)*24/sdnum;
	return ret;
}
//end of file
