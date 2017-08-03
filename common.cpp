//common.cpp by wanggt
#include"common.h"
//file/path names
char basfile[100]; //1: base file
char bmpfile[100]; //2: bmp  file
char datfile[100]; //3: data file
char dmpfile[100]; //4: dump file
char logfile[100]; //5: log  file
char matfile[100]; //6: matlab file
char rawfile[100]; //7: psse/raw file
char resfile[100]; //8: result file
char tmpFile[100]; //9: temp file
char datpath[100]; //10: data path
char hispath[100]; //11: history path
char respath[100]; //12: result path
char osipath[100]; //13: osi/monarch
char userpath[100];//14: osi/user
////////////////////////////////////////////////////////////
//debug level/file
char  debug_level;
char  debug_file[100];
FILE *debug_fp;
clock_t clock0;
////////////////////////////////////////////////////////////
//date/time and ret
char start_time[100];
char end_time  [100];
char case_str  [100];
char date_str  [100];
char time_str  [100];
char crrdate   [100];
char crrtime   [100];
double f1,f2,f3,f4,f5,f6,f7,f8,f9;
double fa,fb,fc,fd,fe,ff;
long   ia,ib,ic,id,ie;
long   ret;
////////////////////////////////////////////////////////////
//buffer, token/seps
char buffer[1000];
char buf   [1000];
char seg1  [100];
char seg2  [100];
char seg3  [100];
char seg4  [100];
char seg5  [100];
char seg6  [100];
char seg7  [100];
char seg8  [100];
char seg9  [100];
char *token;
////////////////////////////////////////////////////////////

long lookup2(double v0,double *v,long n)
{
	for(long i=1;i<n;i++)
	if(fabs(v0-v[i])<SMLL) return i;

	for(long i=1;i<n;i++)
	if(interv(v0,v[i],v[i+1])) return i;
	////////////////////////////////////////////////////////////

	return NO;
}

void invfun(double **a,long n)
{
	double **e;
	makeMatrix(e,n+1,n+1);
	unitMatrix(e,n);
	for(long k=1;k<=n;k++)
	{
		long imax=pospp(a,k,n);
		if(imax!=k){ swaprow(a,imax,k); swaprow(e,imax,k); }
		divrow(e,a[k][k],k,n);//akk
		divrow(a,a[k][k],k,n);
		for(long i=1;i<=n;i++)
		{
			double aik=a[i][k];
			if(fabs(aik)<SMLL) continue;
			if(i==k) continue;
			addrow(a,-aik,i,k,n);
			addrow(e,-aik,i,k,n);
		}
	}
	////////////////////////////////////////////////////////////

	copy(a,e,n,n);
	free(e,n+1);
	////////////////////////////////////////////////////////////

	return;
}

void copy(double *v,double *v0,long i0,long n)
{
	memcpy(v,v0+i0,(n+1-i0)*sizeof(double));
	return;
}

void copy(double *v,long i,long i0,long n)
{
	memmove(v+i,v+i0,(n+1-i0)*sizeof(double));
	return;
}

void lufun(struct fspar *lu,struct fspar *s,long n)
{
	double *lbuf=(double *)calloc(n+1,sizeof(double));
	double *ubuf=(double *)calloc(n+1,sizeof(double));
	struct fspar *ucol; makespar(ucol,n+1);
	for(long k=1;k<=n;k++)
	{
		unspar(lbuf,n,lu,k);
		unspar(ubuf,n,ucol,k);
//		if(k%100==0) printf("\nk=%ld: n1,n2=%ld %ld",k,lu[k].n,ucol[k].n);
		for(long j=k;j<=n;j++)//u[]
		{
			double sum=sumrow(ucol,j,lbuf);
			double skj=getval(s,k,j);
			double ukj=skj-sum;
			if(fabs(ukj)<SMLL) continue;
			setval2(ucol,j,k,ukj);
		}
		////////////////////////////////////////////////////////////

		double ukk=getval(ucol,k,k);
		for(long i=k+1;i<=n;i++)//l[]
		{
			double sum=sumrow(lu,i,ubuf);
			double sik=getval(s,i,k);
			double lik=(sik-sum)/ukk;
			if(fabs(lik)<SMLL) continue;
			setval2(lu,i,k,lik);
		}
	}
	////////////////////////////////////////////////////////////

	for(long j=1;j<=n;j++)//lu[]
	for(long i=1;i<=ucol[j].n;i++)
	{
		long    ii=ucol[j].index[i];
		double uij=ucol[j].value[i];
		setval2(lu,ii,j,uij);
	}
	freespar(ucol,n+1);
	free(ubuf);
	free(lbuf);
	////////////////////////////////////////////////////////////

	return;
}

void lufun(double **lu,double **a,long n)
{
	for(long k=1;k<=n;k++)
	{
		for(long j=k;j<=n;j++)//u[]
		{
			double sum=0;
			for(long t=1;t<k;t++) sum+= lu[k][t]*lu[t][j];
			lu[k][j]=a[k][j]-sum;
		}
		for(long i=k+1;i<=n;i++)//l[]
		{
			double sum=0;
			for(long t=1;t<k;t++) sum+= lu[i][t]*lu[t][k];
			lu[i][k]=(a[i][k]-sum)/lu[k][k];
		}
	}
	////////////////////////////////////////////////////////////

	return;
}




void makespar(HSPAR **&h,long m)
{
	h=(HSPAR **)calloc(m,sizeof(HSPAR *));
	if(h!=NULL) memset(h,0,m*sizeof(HSPAR *));
	if(h==NULL)
	{
		printf("\nError --- %ld",__LINE__);
		exit(0);
	}
	////////////////////////////////////////////////////////////

	return;
}

void getpnt_y(double *y,long n,PNTSTR *pnt)
{
	for(long i=0;i<=n;i++) y[i]=pnt[i].y;
	return;
}

long LORfun(PNTSTR point,PNTSTR p1,PNTSTR p2)
{
	double rad1=atan2(p2.y-p1.y,p2.x-p1.x);
	double rad2=atan2(point.y-p1.y,point.x-p1.x);
	double rad =rad2-rad1;
	if(fabs(point.x-p1.x)<SMLL)
	if(fabs(point.y-p1.y)<SMLL) return ON_LINE;
	if(fabs(rad)<SMLL || fabs(fabs(rad)-PI)<SMLL)
	{
		if(interv(point.x,p1.x,p2.x))
		if(interv(point.y,p1.y,p2.y)) return ON_LINE;
		return ON_LINEX;
	}
	////////////////////////////////////////////////////////////

	if(rad>0 && rad< PI)     return ON_LEFT;
	if(rad<0 && rad>-PI)     return ON_RIGHT;
	if(rad> PI && rad< 2*PI) return ON_RIGHT;
	if(rad<-PI && rad>-2*PI) return ON_LEFT;
	////////////////////////////////////////////////////////////

	printf("\nWarning --- %ld",__LINE__);
	return NO;
}

bool inside(PNTSTR point0,PNTSTR *point,long n)
{
	long rnum=0;//init
	long lnum=0;
	for(long i=1;i<=n;i++)
	{
		PNTSTR p1=point[i+0];
		PNTSTR p2=point[i+1];
		if(i==n) p2=point[1];
		long ret=LORfun(point0,p1,p2);
		if(ret==ON_RIGHT) rnum++;
		if(ret==ON_LEFT)  lnum++;
		if(ret==ON_LINE)  return true;
	}
	////////////////////////////////////////////////////////////

	if(rnum>0 && lnum==0) return true;
	if(lnum>0 && rnum==0) return true;
	////////////////////////////////////////////////////////////

	return false;
}





double sum(double **a,long m,long n)
{
	double ret=0;//init
	for(long i=0;i<=m;i++)
	for(long j=0;j<=n;j++) ret+= a[i][j];
	return ret;
}

double sum(const double *a,double x,long n)
{
	double xx =1;//init
	double ret=0;
	for(long i=0;i<=n;i++)
	{
		ret+= xx*a[i];
		xx *= x;
	}
	////////////////////////////////////////////////////////////

    return ret;
}

double rmsfun(double *x,long n)
{
	double sum=0;//init
	for(long i=0;i<n;i++) sum+= x[i]*x[i];
	double ret=sqrt(sum/n);
	return ret;	
}

long cavgStat(double *v,long i0,long n)
{
//v[] - vector
//i0  - index start
//n   - index max (size)
//ret - crossing-avg count
////////////////////////////////////////////////////////////

	double va =avgfun(v,i0,n);
	double v0 =0;//init
	long   ret=0;
	for(long i=i0;i<=n;i++)
	{
		double vi=v[i]-va;
		if(fabs(vi)<SMLL)  continue;
		if(fabs(v0)<SMLL)  v0=vi;
		if(  vi*v0 <SMLL){ v0=vi; ret++; }
	}
	////////////////////////////////////////////////////////////

	return ret;
}

void orset(long *v,long &n,long *v1,long n1,long *v2,long n2)
{
	copy(v,v1,n1); n=n1;
	for(long i=1;i<=n2;i++)
	if(lookup(v2[i],v,n)==NO)
	{
		n++;
		v[n]=v2[i];
	}
	////////////////////////////////////////////////////////////

	return;
}

void andset(long *v,long &n,long *v1,long n1,long *v2,long n2)
{
	n=0;//init
	for(long i=1;i<=n1;i++)
	if(lookup(v1[i],v2,n2)!=NO)
	{
		n++;
		v[n]=v1[i];
	}
	////////////////////////////////////////////////////////////

	return;
}

long pospp(struct fspar *s,long k,long m)
{
	for(long i=k;i<=m;i++)
	{
		double sik=getval(s,i,k);
		if(fabs(sik)>SMLL) return i;
//		if(s[i].n<min){ min=s[i].n; imin=i; }
	}
	////////////////////////////////////////////////////////////

	return 0;
}

long pospp(double **a,long k,long m)
{
	for(long i=k;i<=m;i++)
	{
		double fik=fabs(a[i][k]);
		if(fik>SMLL) return i;
	}
	////////////////////////////////////////////////////////////

	return 0;
}

void subtrfun(double **a,double **subtr,long m,long n)
{
	for(long i=0;i<=m;i++)
	for(long j=0;j<=n;j++) a[i][j]-= subtr[i][j];
	return;
}

void mulspar(FSPAR *s,FSPAR *s1,FSPAR *s2,long m,long l,long n)
{
	double *s1buf=(double *)calloc(l+1,sizeof(double));
	if(s1buf==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
	FSPAR   *s2col;
	makespar(s2col,n+1);
	sparcol (s2col,s2,l); initspar(s,m);
	for(long i=1;i<=m;i++)
	{
		unspar(s1buf,l,s1,i);
		for(long j=1;j<=n;j++)
		{
			double sum=0;//init
			for(long k=1;k<=s2col[j].n;k++)
			{
				long     kk=s2col[j].index[k];
				double s2kj=s2col[j].value[k];
				sum += s2kj*s1buf[kk];
			}	if(fabs(sum)>SMLL) setval2(s,i,j,sum);
		}
	}
	////////////////////////////////////////////////////////////

	freespar(s2col,n+1);
	free    (s1buf);
	////////////////////////////////////////////////////////////

	return;
}



void sparmem(struct fspar *s,long i,long nmax)
{
	double *vp=(double *)realloc(s[i].value,nmax*sizeof(double));
	long   *ip=(long *)  realloc(s[i].index,nmax*sizeof(long));
	if(vp==NULL || ip==NULL)
	{
		printf("\nError - %s %ld",__FILE__,__LINE__);
		exit(0);
	}
	s[i].nmax =nmax;
	s[i].value=vp;
	s[i].index=ip;
	////////////////////////////////////////////////////////////

	return;
}

void sparmem(struct spar *s,long i,long nmax)
{
	long *vp=(long *)realloc(s[i].value,nmax*sizeof(long));
	long *ip=(long *)realloc(s[i].index,nmax*sizeof(long));
	if(vp==NULL || ip==NULL)
	{
		printf("\nError - %s %ld",__FILE__,__LINE__);
		exit(0);
	}
	s[i].nmax =nmax;
	s[i].value=vp;
	s[i].index=ip;
	////////////////////////////////////////////////////////////

	return;
}



void delcol(struct fspar *s,long j0,long m)
{
	for(long i=1;i<=m;i++) delelm_spar(s,i,j0);
	for(long i=1;i<=m;i++)
	for(long j=1;j<=s[i].n;j++)
	{
		long jj = s[i].index[j];
		if(jj>j0) s[i].index[j]--;
	}
	////////////////////////////////////////////////////////////

	return;
}

void mulrow(struct fspar *s,double mul,long i)
{
	for(long j=1;j<=s[i].n;j++)
		s[i].value[j]*= mul;
}

void mulrow(struct hspar **h,double mul,long i)
{
	struct hspar *hp=h[i];
	while(hp!=NULL)
	{
		mulVector(hp->value,mul,hp->n);
		hp=hp->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

void copycol2(struct fspar *s,double *v,long j,long m)
{
	for(long i=0;i<=m;i++)
	{
		if(fabs(v[i])<SMLL) continue;
		setval2(s,i,j,v[i]);
	}
	////////////////////////////////////////////////////////////

	return;
}

double solve(double (*fun)(double),double x0,double x1)
{
//fun() - function
//x0,x1 - init.values
//ret   - solution of fun(x)=0
////////////////////////////////////////////////////////////

	double f0=fun(x0);
	double f1=fun(x1);
	if(fabs(f1)>fabs(f0)){ swap(x0,x1); swap(f0,f1); }
	for(long i=1;i<1000;i++)
	{
		double s=f1/f0;
		double x=x1-((x0-x1)*s)/(1-s);
		double f=fun(x);
		if(fabs(f)<TOL) return x;
		if(fabs(f)>fabs(f1)){ x0=x; f0=f; }
		else{ x0=x1; f0=f1; x1=x; f1=f; }
	}
	////////////////////////////////////////////////////////////

	printf("\nWarning - %s %ld",__FILE__,__LINE__);
	printf("\n solve() failed");
	////////////////////////////////////////////////////////////

	return 0;
}

PNTSTR crosspnt(PNTSTR p11,PNTSTR p12,PNTSTR p21,PNTSTR p22)
{
	double dx1=p12.x-p11.x;
	double dx2=p22.x-p21.x;
	double dy1=p12.y-p11.y;
	double dy2=p22.y-p21.y;
	double s1 =LARGE;//init
	double s2 =LARGE;
	if(fabs(dx1)>SMLL) s1=dy1/dx1;
	if(fabs(dx2)>SMLL) s2=dy2/dx2;
	struct pntstr ret=setpnt(LARGE,LARGE);
	if(fabs(s1-s2)<SMLL) return ret;
	////////////////////////////////////////////////////////////

	double x=(s1*p11.x-s2*p21.x-(p11.y-p21.y))/(s1-s2);
	double y=s1*(x-p11.x)+p11.y;
	if(interv(x,p11.x,p12.x))
	if(interv(x,p21.x,p22.x)) ret.x=x;
	if(interv(y,p11.y,p12.y))
	if(interv(y,p21.y,p22.y)) ret.y=y;
	////////////////////////////////////////////////////////////

	return ret;
}

double PIDfun(double *e,long t,double Kp,double Ti,double Td)
{
//e[] - error signal: 0,1,..t
//t   - current time
//Kp  - proportion coeff
//Ti  - integral
//Td  - differential
////////////////////////////////////////////////////////////

	double   D=0;
	if(t>0)  D=Td*(e[t]-e[t-1]);
	double ret=Kp*(e[t]+sum(e,(long)0,t)/Ti+D);
	return ret;
}

void normal(double *v,long n)
{
	double s=sum(v,n);
	for(long i=1;i<=n;i++) v[i]/= s;
	return;
}

void normal(double *v,long n,double vmin,double vmax)
{
	double dvmm=vmax-vmin;
	if(dvmm<SMLL){ printf("\nError --- %ld",__LINE__); exit(0); }
	for(long i=1;i<=n;i++)
		v[i]=(v[i]-vmin)/dvmm;
	////////////////////////////////////////////////////////////

	return;
}

double sigmfun(double x)
{
	double ret=1/(1+exp(-x));
	return ret;
}

long strchar(char *string,char ch)
{
	long n=strlen(string);
	long ret=0;
	for(long i=0;i<n;i++)
	if(string[i]==ch) ret++;
	////////////////////////////////////////////////////////////

	return ret;
}

void reverse(double *v,long i0,long n)
{
	long nm=(i0+n)/2;
	for(long i=i0;i<=nm;i++)
	{
		double vi=v[i];
		v[i]=v[n-i+i0];
		v[n-i+i0]=vi;
	}
	////////////////////////////////////////////////////////////

	return;
}

void reverse(double *v,long n)
{
	reverse(v,(long)1,n);
	return;
}

double simpfun(double (*fun)(double),double a,double b)
{
//a   - start point
//b   - end point
//fun - function
//ret - simpson integral
////////////////////////////////////////////////////////////
	double h=(b-a)/2;
	double x=a+h;
	double sp =(*fun)(a)+(*fun)(b)+2*(*fun)(x);
	double sum=(sp+2*(*fun)(x))*h/3;
	long n=1;//init
	while(1)
	{
		double sum0=sum;
		double sc=0;
		double x=a-h/2; n*= 2;
		for(long j=1;j<=n;j++){ x+= h; sc+= (*fun)(x); }
		h/= 2; sum=(4*sc+sp)*h/3;
		if(fabs(h)<SMLL) printf("\nWarning --- %ld",__LINE__);
		if(fabs(sum-sum0)<SMLL) break;
		else sp+= 2*sc;
	}
	////////////////////////////////////////////////////////////

	return sum;
}

double normal(double x)
{
	double ret=1/sqrt(2*PI)*exp(-x*x/2);
	return ret;
}

double normal(double v1,double v2)
{
	while(1)
	{
		double f0=frand(0,normal(0));
		double f =frand(v1,v2);
		double ff=normal(f);
		if(f0<ff) return f;
	}
	////////////////////////////////////////////////////////////

	printf("\nWarning --- %ld",__LINE__);
	return 0;
}

double fmaxdv(double *v,double *v0,long n)
{
	double ret=-LARGE;
	for(long i=0;i<=n;i++)
	{
		double dv=v[i]-v0[i];
		ret=max(ret,dv);
	}
	////////////////////////////////////////////////////////////

	return ret;
}

double fmaxdv(double *v,long n)
{
	long imaxdv=posmaxdv(v,n);
	if(imaxdv==0) return 0;
	double ret=v[imaxdv-1]-v[imaxdv];
	return ret;
}

long posmaxdv(double *v,long n)
{
	double maxdv=0;//init
	long  imaxdv=0;
	for(long i=1;i<=n;i++)
	{
		double dv=v[i-1]-v[i];
		if(dv>maxdv){ maxdv=dv; imaxdv=i; }
	}
	////////////////////////////////////////////////////////////

	return imaxdv;
}

void strtrim(char *string)
{
	long n=strlen(string);
	for(long i=n-1;i>=0;i--)
	{
		if(string[i]>0)
		if(isqmark(string[i]) || isspace(string[i]))
		{
			string[i]=NULL;
			continue;
		}	break;
	}
	////////////////////////////////////////////////////////////

	for(long i=0;i<n;i++)
	{
		if(isqmark(string[i])) continue;
		if(isspace(string[i])) continue;
		strcpy(string,string+i);
		break;
	}
	////////////////////////////////////////////////////////////

	return;
}

void timefun(char *time_str,long sec)
{
	long min =sec/60;
	long hr  =min/60;
	long _min=min%60;
	long _sec=sec%60;
	sprintf(time_str,"%ld:%02ld:%02ld",hr,_min,_sec);
	////////////////////////////////////////////////////////////

	return;
}

long timefun(char *time_str)
{
	long hr =gettime_hour(time_str);
	long min=gettime_min(time_str);
	long sec=gettime_sec(time_str);
	long ret=3600*hr+60*min+sec;
	////////////////////////////////////////////////////////////

	return ret;
}

double timefun()
{
	double ret=(double)(clock()-clock0)/CLOCKS_PER_SEC;
	return ret;
}

void getfile(char *file,char *pathfile)
{
	char *cp=strrchr(pathfile,'\\');
	if(cp==NULL) cp=strrchr(pathfile,'/');
	if(cp==NULL) strcpy(file,pathfile);
	else
	strcpy(file,cp+1);
	strcut(file,'.');
	////////////////////////////////////////////////////////////

	return;
}

double getfval(char *string)
{
	char *cp=strstr(string,"=");
	if(cp!=NULL) return atof(cp+1);
	return 0;
}

long getval(char *string)
{
	char *cp=strstr(string,"=");
	if(cp!=NULL) return atol(cp+1);
	return 0;
}

void getval(char *value,char *string)
{
	initstr(value);
	char *cp=strstr(string,"=");
	if(cp!=NULL)
	{
		strcpy(value,cp+1);
		strtrim(value);
	}
	////////////////////////////////////////////////////////////

	return;
}

long datefun0(char *date_str)
{
	long ret =0;//init
	long mth =getdate_mth (date_str);
	long day =getdate_day (date_str);
	long year=getdate_year(date_str);
	for(long m=1;m<mth;m++)
	ret+= mthdaysFun(m,year);
	ret+= day;
	////////////////////////////////////////////////////////////

	return ret;
}

long datefun(char *date_str)
{
	long ret =datefun0(date_str);
	long year=getdate_year(date_str);
	for(long y=1900;y<year;y++)
	{
		if(isleap(y)) ret+= 366;
		else ret+= 365;
	}
	////////////////////////////////////////////////////////////

	return ret;
}

long wdayfun(char *date_str)
{
	long dd =datediff(date_str,"1/1/1900");
	long ret=(1+dd%7)%7;
	return ret;
}

void strseg(char *seg,char *string,long k,long k2)
{
	strncpy(seg,string+(k-1),k2-k+1);
	strcut (seg,k2-k+1);
}

void strseg(char *seg,char *string,long k)
{
	char linebuf[1000];
	char q1str  [100];
	char q2str  [100];
	long qk=0;//init
	strcpy(linebuf,string);
	initqstr(q1str,linebuf);
	initqstr(q2str,linebuf+(strstr(string,q1str)-string)+strlen(q1str));
	token=strtok(linebuf,seps);
	if(token!=NULL)
	if(isqstr(token)){ token=q1str; qk=1; }
	for(long i=1;i<k;i++)
	{
		if(token!=NULL) token=strtok(NULL,seps);
		if(token!=NULL && isqstr(token))
		{
			qk++;
			if(qk==1) token=q1str; 
			if(qk==2) token=q2str;
		}
	}
	////////////////////////////////////////////////////////////

	initstr(seg);
	if(token!=NULL) strcpy(seg,token);
	////////////////////////////////////////////////////////////

	return;
}



void initVector(double *v,long i0,long n)
{
	for(long i=i0;i<=n;i++) v[i]=0;
	return;
}

void initVector(double *v,long n,double v0)
{
	for(long i=0;i<=n;i++) v[i]=v0;
	return;
}

bool issymm(double **a,long n)
{
	for(long i=1;i<=n;i++)
	for(long j=1;j< i;j++)
	{
		double d=a[i][j]-a[j][i];
		if(fabs(d)>SMLL) return false;
	}
	////////////////////////////////////////////////////////////

	return true;
}

bool isspace(char *string)
{
	long n=strlen(string);
	for(long i=0;i<n;i++)
	{
		if(string[i]<0)   return false;
		if(string[i]>255) return false;
		if(!isspace(string[i])) return false;
	}
	////////////////////////////////////////////////////////////

	return true;
}

bool isqmark(char ch)
{
	if(ch=='\"') return true;
	if(ch=='\'') return true;
	return false;
}

bool isfind(char *filename)
{
	FILE *fp=fopen(filename,"r");
	if(fp!=NULL){ fclose(fp); return true; }
	return false;
}

long datediff(char *date_str,char *date0)
{
	long d  =datefun(date_str);
	long d0 =datefun(date0);
	long ret=d-d0;
	return ret;
}

long timediff(char *time_str,char *time0)
{
	long t  =timefun(time_str);
	long t0 =timefun(time0);
	long ret=t-t0;
	return ret;
}

double pow(double x,long n)
{
	double ret=pow(x,(double)n);
	return ret;
}

COMPLEX pow(COMPLEX v,long n)
{
	double  r  =fabs(v);
	double  rad=atan2(v.im,v.re);
	COMPLEX ret=setvalp(pow(r,n),rad*n);
	return  ret;
}

void gausslu(struct fspar *s,double *b,long n)
{
	struct fspar *lu;
	makespar(lu,n+1);
	lufun   (lu,s,n);
	lu_forwd(lu,b,n);
	lu_back (lu,b,n);
	freespar(lu,n+1);
	////////////////////////////////////////////////////////////

	return;
}

long strseg(char *string)
{
	char linebuf[1000];
	char q1str  [100];
	char q2str  [100];
	long ret=0;//init
	strcpy(linebuf,string);
	initqstr(q1str,linebuf);
	initqstr(q2str,linebuf+(strstr(string,q1str)-string)+strlen(q1str));
	token=strtok(linebuf,seps);
	while(token!=NULL)
	{
		ret++;
		token=strtok(NULL,seps);
	}
	////////////////////////////////////////////////////////////

	return ret;
}

long sign(double v)
{
	if(v>0.0) return  1;
	if(v<0.0) return -1; return 0;
}

long sign(char ch)
{
	if(ch=='+') return  1;
	if(ch=='-') return -1;
//	printf("\nWarning --- %ld",__LINE__);
	return 0;
}

double getval(HSPAR **h,long i,long j)
{
	HSPAR *hp=findhp(h,i,j);
	if(hp!=NULL)
	{
		for(long jj=1;jj<=hp->n;jj++)
		if(hp->index[jj]==j)
		return hp->value[jj];
	}
	////////////////////////////////////////////////////////////

	return 0;
}

void delelm_spar(HSPAR **h,long i,long j)
{
	HSPAR *hp=findhp(h,i,j);
	if(hp==NULL) return;
	long k=lookup(j,hp->index,hp->n);
	if(k!=NO)
	{
		delelm (hp->value,k,hp->n);
		delelm (hp->index,k,hp->n); hp->n--;
		sparmem(hp,h,i);
	}
	////////////////////////////////////////////////////////////

	return;
}

HSPAR *findhp(HSPAR **h,long i,long j)
{
	HSPAR *hp=h[i];
	while(hp!=NULL)
	{
		if(hp->nh==hash100(j)) return hp;
		hp=hp->next;
	}
	////////////////////////////////////////////////////////////

	return NULL;
}

void setval(HSPAR **h,long i,long j,double v)
{
	struct hspar *hp=findhp(h,i,j);
	if(hp==NULL)
	{
		HSPAR *hp=(HSPAR *)malloc(sizeof(HSPAR));
		if(hp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
		memset(hp,0,sizeof(HSPAR));
		hp->nh=hash100(j);
		hp->n =1; sparmem(hp,h,i);
		hp->index[hp->n]=j;
		hp->value[hp->n]=v;
		hp->next=h[i];
		h[i]=hp;
	}
	////////////////////////////////////////////////////////////
	else
	{	long k=lookup(j,hp->index,hp->n);
		if(k!=NO){ hp->value[k]=v; return; }
		hp->n++; sparmem(hp,h,i);
		hp->index[hp->n]=j;
		hp->value[hp->n]=v;
	}
	////////////////////////////////////////////////////////////

	return;
}


void setval2(HSPAR **h,long i,long j,double v)
{
	struct hspar *hp=findhp(h,i,j);
	if(hp==NULL)
	{
		HSPAR *hp=(HSPAR *)malloc(sizeof(HSPAR));
		if(hp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
		memset(hp,0,sizeof(HSPAR));
		hp->nh=hash100(j);
		hp->n =1; sparmem(hp,h,i);
		hp->index[hp->n]=j;
		hp->value[hp->n]=v;
		hp->next=h[i];
		h[i]    =hp;
	}
	////////////////////////////////////////////////////////////
	else
	{	hp->n++; sparmem(hp,h,i);
		hp->index[hp->n]=j;
		hp->value[hp->n]=v;
	}
	////////////////////////////////////////////////////////////

	return;
}

long gettime_hour(char *time_str)
{
	char timebuf[100];
	strcpy(timebuf,time_str);
	token=strtok(timebuf,seps_time);
	if(token!=NULL) return atol(token);
	////////////////////////////////////////////////////////////

	return 0;
}

long gettime_min(char *time_str)
{
	char timebuf[100];
	strcpy(timebuf,time_str);
	token=strtok(timebuf,seps_time);
	token=strtok(NULL,seps_time);
	if(token!=NULL) return atol(token);
	////////////////////////////////////////////////////////////

	return 0;
}

long gettime_sec(char *time_str)
{
	char timebuf[100];
	strcpy(timebuf,time_str);
	token=strtok(timebuf,seps_time);
	token=strtok(NULL,seps_time);
	token=strtok(NULL,seps_time);
	if(token!=NULL) return atol(token);
	////////////////////////////////////////////////////////////

	return 0;
}

void sdfun(char *time_str,long sd,long sdnum)
{
	long sdmin=1440/sdnum;
	long min  =max((sd-1),0)*sdmin;
	long hr   =min/60;
	min=min%60;
	sprintf(time_str,"%ld:%02ld",hr,min);
	////////////////////////////////////////////////////////////

	return;
}

long sdfun(char *time_str,long sdnum)
{
	long sdmin=1440/sdnum;
	long sec  =timefun(time_str);
	long min  =sec/60;
	long ret  =min/sdmin+1;
	////////////////////////////////////////////////////////////

	return ret;
}

void strpack(char *string)
{
	char linebuf[1000]="";//init.
	long k=0;
	long n=strlen(string);
	for(long i=0;i<n;i++)
	{
		if(isspace(string[i])) continue;
		if(ispunct(string[i])) continue;
		linebuf[k]=string[i];
		linebuf[k+1]=NULL; k++;
	}
	////////////////////////////////////////////////////////////

	strcpy(string,linebuf);
	return;
}

void strrep(char *string,char *str,char *rep)
{
	char linebuf[1000];
	char *cp=strstr(string,str);
	if(cp!=NULL)
	{
		long n=(long)(cp-string);
		strncpy(linebuf,string,n);
		strcut (linebuf,n);
		strcat (linebuf,rep);
		strcat (linebuf,cp+strlen(str));
		strcpy (string,linebuf);
	}
	////////////////////////////////////////////////////////////

	return;
}

long lookup(long v0,long *v,long n)
{ 
	for(long i=1;i<=n;i++)
	if(v0==v[i]) return i;
	return NO;
}

long lookup(double v0,double *v,long n)
{
	for(long i=1;i<=n;i++)
	if(fabs(v0-v[i])<SMLL) return i;
	return NO;
}

long lookup(long v1,long v2,long *v,long n)
{
	for(long i=1;i<=n;i++)
	{
		if(v1==v[i]) return i;
		if(v2==v[i]) return i;
	}
	////////////////////////////////////////////////////////////

	return NO;
}

void mulspar(CSPAR *s,CSPAR *s1,CSPAR *s2,long m,long l,long n)
{
	COMPLEX *s1buf=(COMPLEX *)calloc(l+1,sizeof(COMPLEX));
	CSPAR   *s2col;
	makespar(s2col,n+1);
	sparcol (s2col,s2,l); initspar(s,m);
	for(long i=1;i<=m;i++)
	{
		unspar(s1buf,l,s1,i);
		for(long j=1;j<=n;j++)
		{
			COMPLEX sum=setval0();
			for(long k=1;k<=s2col[j].n;k++)
			{
				long      kk=s2col[j].index[k];
				COMPLEX s2kj=s2col[j].value[k];
				sum +=  s2kj*s1buf[kk];
			}
			////////////////////////////////////////////////////////////

			if(fabs2(sum)<SMLL2) continue;
			setval2 (s,i,j,sum);
		}
	}
	////////////////////////////////////////////////////////////

	freespar(s2col,n+1);
	free    (s1buf);
	////////////////////////////////////////////////////////////

	return;
}

void mulspar(double *v,FSPAR *s,double *v0,long m,long n)
{
	for(long i=1;i<=m;i++)
	{
		double sum=0;//init.
		for(long j=1;j<=s[i].n;j++)
		{
			long    jj=s[i].index[j];
			double sij=s[i].value[j];
			sum += sij*v0[jj];
		}	v[i] = sum;
	}
	////////////////////////////////////////////////////////////

	return;
}

void lu_forwd(struct fspar *lu,double *b,long n)
{
	for(long i=1;i<=n;i++)
	{
		double sum=0;//init
		for(long j=1;j<=lu[i].n;j++)
		{
			double lij=lu[i].value[j];
			long    jj=lu[i].index[j];
			if(jj<i) sum+= lij*b[jj];
		}	b[i] -=  sum;
	}
	////////////////////////////////////////////////////////////

	return;
}

void lu_back(struct fspar *lu,double *b,long n)
{
	for(long i=n;i>0;i--)
	{
		double sum=0;//init
		for(long j=1;j<=lu[i].n;j++)
		{
			double uij=lu[i].value[j];
			long    jj=lu[i].index[j];
			if(jj>i) sum += uij*b[jj];
		}	b[i]=(b[i]-sum)/getval(lu,i,i);
	}
	////////////////////////////////////////////////////////////

	return;
}

void gausspp(struct fspar *s,double *b,long n)
{
	double *sibuf=(double *)calloc(n+1,sizeof(double));
	if(sibuf==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
	for(long k=1;k<=n;k++)
	{
		long imax=pospp(s,k,n);
		if(imax!=k){ swap(s,imax,k); swap(b,imax,k); }
		for(long i=k+1;i<=n;i++)
		{
			double skk=getval(s,k,k);
			double sik=getval(s,i,k);
			double r  =sik/skk;
			if(fabs(sik)<SMLL) continue;
			unspar (sibuf,k,n,s,i);
			addrow (sibuf, -r,s,k);
			sparfun(s,i,sibuf,n);
			b[i]-= r*b[k];
		}
	}
	////////////////////////////////////////////////////////////

	for(long i=n;i>0;i--)
	{
		double sum=0;//init
		for(long j=1;j<=s[i].n;j++)
		{
			double sij=s[i].value[j];
			long    jj=s[i].index[j];
			if(jj>i) sum+= sij*b[jj];
		}	b[i]=(b[i]-sum)/getval(s,i,i);
	}
	////////////////////////////////////////////////////////////

	free(sibuf);
	return;
}

void gausspp(double **a,double *b,long n)
{
	for(long k=1;k<=n;k++)
	{
		long imax=pospp(a,k,n);
		if(imax!=k){ swaprow(a,imax,k); swap(b,imax,k); }
		for(long i=k+1;i<=n;i++)
		{
			if(fabs(a[k][k])<SMLL){ printf("\nError --- %ld",__LINE__); exit(0); }
			if(fabs(a[i][k])<SMLL) continue;
			double r=a[i][k]/a[k][k];
			b[i]-= r*b[k];
			addrow(a,-r,i,k,n);
		}
	}
	////////////////////////////////////////////////////////////

	for(long i=n;i>0;i--)
	{
		double sum=0;//init
		for(long j=i+1;j<=n;j++) sum+= a[i][j]*b[j];
		b[i]=(b[i]-sum)/a[i][i];
	}
	////////////////////////////////////////////////////////////

	return;
}

void unspar(double **a,struct fspar *s,long m,long n)
{
	initMatrix(a,m,n);
	for(long i=1;i<=m;i++)
	for(long j=1;j<=s[i].n;j++)
	{
		long    jj=s[i].index[j];
		double sij=s[i].value[j];
		a[i][jj]=sij;
	}
	////////////////////////////////////////////////////////////

	return;
}



void unspar(double *v,long k,long n,FSPAR *s,long i)
{
	memset(&v[k],0,(n-k+1)*sizeof(double));
	for(long j=1;j<=s[i].n;j++)
	{
		long    jj=s[i].index[j];
		double sij=s[i].value[j];
		v[jj]= sij;
	}
	////////////////////////////////////////////////////////////

	return;
}

void sparfun(struct fspar *s,long i,double *v,long n)
{
	initrow(s,i);
	for(long j=1;j<=n;j++)
	{
		double sij=v[j];
		if(fabs(sij)<SMLL) continue;
		setval2(s,i,j,sij);
	}
	////////////////////////////////////////////////////////////

	return;
}

void sparfun(struct fspar *s,double **a,long m,long n)
{
	initspar(s,m);
	for(long i=1;i<=m;i++)
	for(long j=1;j<=n;j++)
	{
		double aij=a[i][j];
		if(fabs(aij)<SMLL) continue;
		setval2(s,i,j,aij);
	}
	////////////////////////////////////////////////////////////

	return;
}



void sparmem(HSPAR *hp,HSPAR **h,long i,long nmax)
{
	double *vp=(double *)realloc(hp->value,nmax*sizeof(double));
	long   *ip=(long *)  realloc(hp->index,nmax*sizeof(long));
	if(vp==NULL || ip==NULL)
	{
		printf("\nError - %s %ld",__FILE__,__LINE__);
		exit(0);
	}
	////////////////////////////////////////////////////////////

	hp->nmax =nmax;
	hp->value=vp;
	hp->index=ip;
	////////////////////////////////////////////////////////////

	return;
}



void sparmem(HSPAR *hp,HSPAR **h,long i)
{
	if(hp->n==0)
	{
		free(hp->value);
		free(hp->index);
		delnode(hp,h[i]); return;
	}
	////////////////////////////////////////////////////////////

	long nmax=nmax10(hp->n);
	if(nmax!=hp->nmax)
		sparmem(hp,h,i,nmax);
	////////////////////////////////////////////////////////////

	return;
}

void sparmem(struct cspar *s,long i,long nmax)
{
	COMPLEX *vp=(COMPLEX *)realloc(s[i].value,nmax*sizeof(COMPLEX));
	long    *ip=(long *)   realloc(s[i].index,nmax*sizeof(long));
	if(vp==NULL || ip==NULL)
	{
		printf("\nError - %s %ld",__FILE__,__LINE__);
		exit(0);
	}
	////////////////////////////////////////////////////////////

	s[i].nmax =nmax;
	s[i].value=vp;
	s[i].index=ip;
	////////////////////////////////////////////////////////////

	return;
}

bool posdata(FILE *fp,char *data)
{
	rewind(fp);
	while(!feof(fp))
	{
		if(fgets(buffer,1000,fp)==NULL) break;
		if(strstr(buffer,data)) return true;
	}
	////////////////////////////////////////////////////////////

	return false;
}

bool posdata(FILE *fp,long n)
{	
	long k=0; rewind(fp);
	while(!feof(fp))
	{
		k++;
		if(k==n) return true;
		if(fgets(buffer,1000,fp)==NULL) break;
	}
	////////////////////////////////////////////////////////////

	return false;
}

void read(double **a,long m,long n,char *filename)
{
	FILE *fp=fopen(filename,"r");
	if(fp==NULL){ printf("\nCannot open %s",filename); exit(0); }
	for(long i=1;i<=m;i++)
	for(long j=1;j<=n;j++)
	fscanf(fp,"%lf",&a[i][j]);
	fclose(fp);
	////////////////////////////////////////////////////////////

	return;
}

void read(double *v,long n,char *filename)
{
	FILE *fp=fopen(filename,"r");
	if(fp==NULL){ printf("\nCannot open %s",filename); exit(0); }
	for(long i=1;i<=n;i++)
	fscanf(fp,"%lf",&v[i]);
	fclose(fp);
	////////////////////////////////////////////////////////////

	return;
}

void print(double *v,long n,char *descr)
{
    printf("\n%s:",descr);
    for(long i=0;i<=n;i++)
	{
//		if(i%10==1) printf("\n");
		printf("\n i=%ld: %lf",i,v[i]);
	}
	////////////////////////////////////////////////////////////

    return;
}

void print(char *string,long n)
{
	for(long i=0;i<n;i++)
	{
		char ch=string[i];
		printf("\n %4ld: %4ld 0x%02X %c",i,ch,ch,ch);
	}
	////////////////////////////////////////////////////////////

	return;
}

void print(char *string)
{
	long n=strlen(string);
	for(long i=0;i<n;i++)
	{
		char ch=string[i];
		printf("\n %4ld: %4ld 0x%02X %c",i,ch,ch,ch);
	}
	////////////////////////////////////////////////////////////

	return;
}

void print(long *v,long n,char *descr)
{
	printf("\n%s:",descr);
	for(long i=1;i<=n;i++)
	{
		if(i%10==1) printf("\n");
		printf(" %4ld",v[i]);
	}
	////////////////////////////////////////////////////////////

	return;
}

void print(COMPLEX *v,long n,char *descr)
{
    printf("\n%s:\n",descr);
	for(long i=1;i<=n;i++) printf("  %lf",v[i].re);
	printf("\n");
	for(long i=1;i<=n;i++) printf(" j%lf",v[i].im);
	////////////////////////////////////////////////////////////

	return;
}

void print(COMPLEX **a,long m,long n,char *descr)
{
    printf("\n%s:",descr);
    for(long i=1;i<=m;i++)
	{
		printf("\n");
		for(long j=1;j<=n;j++) printf("  %lf",a[i][j].re);
		printf("\n");
		for(long j=1;j<=n;j++) printf(" j%lf",a[i][j].im);
	}
	////////////////////////////////////////////////////////////

	return;
}

void print(double **a,long m,long n,char *descr)
{
    printf("\n%s:",descr);
    for(long i=1;i<=m;i++)
	{
		printf("\n");
		for(long j=1;j<=n;j++) printf(" %lf",a[i][j]);
	}
	////////////////////////////////////////////////////////////

    return;
}














void operator+= (COMPLEX &v1,COMPLEX v2){ v1=v1+v2; }
void operator-= (COMPLEX &v1,COMPLEX v2){ v1=v1-v2; }
void operator*= (COMPLEX &v1,COMPLEX v2){ v1=v1*v2; }
void operator/= (COMPLEX &v1,COMPLEX v2){ v1=v1/v2; }

COMPLEX operator+(COMPLEX v1,COMPLEX v2)
{
	COMPLEX ret;
	ret.re= v1.re+v2.re;
	ret.im= v1.im+v2.im;
	return ret;
}

COMPLEX operator-(COMPLEX v1,COMPLEX v2)
{
	COMPLEX ret;
	ret.re= v1.re-v2.re;
	ret.im= v1.im-v2.im;
	return ret;
}

COMPLEX operator-(COMPLEX v1)
{
	COMPLEX ret;
	ret.re=-v1.re;
	ret.im=-v1.im;
	return ret;
}

COMPLEX operator*(COMPLEX v1,COMPLEX v2)
{
	COMPLEX ret;
	ret.re= v1.re*v2.re-v1.im*v2.im;
	ret.im= v1.re*v2.im+v1.im*v2.re;
	return ret;
}

COMPLEX operator/(COMPLEX v1,COMPLEX v2)
{
	double ssq=sqsum(v2.re,v2.im);
	COMPLEX v1c2=v1*conj(v2);
	COMPLEX ret;
	ret.re= v1c2.re/ssq;
	ret.im= v1c2.im/ssq;
	////////////////////////////////////////////////////////////

	return ret;
}

double interp(double x,double x1,double y1,double x2,double y2)
{
	double r=(y2-y1)/(x2-x1);
	double ret=y1+r*(x-x1);
	return ret;
}

double interp(double x0,double *x,double *y,long n)
{
	for(long i=0;i<n;i++)
	if(interv(x0,x[i],x[i+1]))
	return interp(x0,x[i],y[i],x[i+1],y[i+1]);
	return 0;
}

double interp(double x1,double x2,double r)
{
	double ret=(1-r)*x1+r*x2;
	return ret;
}

void freespar(HSPAR **h,long m)
{
	for(long i=0;i<m;i++)
	{
		HSPAR *hp=h[i];
		while(hp!=NULL)
		{
			HSPAR *hpnext=hp->next;
			free(hp->value);
			free(hp->index);
			free(hp);
			hp=hpnext;
		}
	}
	////////////////////////////////////////////////////////////

	return;
}







double frand()
{
	double ret=(double)rand()/RAND_MAX;
	return ret;
}

double frand(double v1,double v2)
{
	double ret=v1-(v1-v2)*frand();
	return ret;
}

void srand(){ srand((unsigned)time(NULL)); }
void frand(double *v,long n)
{
	srand();
	for(long i=0;i<=n;i++)
		v[i]=frand();
}

double linelen(PNTSTR p1,PNTSTR p2)
{
	double dx=p1.x-p2.x;
	double dy=p1.y-p2.y;
	double ret=sqrt(dx*dx+dy*dy);
	return ret;
}

double lineq_a0(double x1,double y1,double x2,double y2)
{
	double ret=y1-x1*(y2-y1)/(x2-x1);
	return ret;
}

double lineq_a1(double x1,double y1,double x2,double y2)
{
	double ret=(y2-y1)/(x2-x1);
	return ret;
}

long getdate_mth(char *date_str)
{
	char datebuf[100];
	strcpy(datebuf,date_str);
	token=strtok(datebuf,seps_date);
	if(token!=NULL) return atol(token);
	////////////////////////////////////////////////////////////

	return 0;
}

long getdate_day(char *date_str)
{
	char datebuf[100];
	strcpy(datebuf,date_str);
	token=strtok(datebuf,seps_date);
	token=strtok(NULL,seps_date);
	if(token!=NULL) return atol(token);
	////////////////////////////////////////////////////////////
	
	return 0;
}

long getdate_year(char *date_str)
{
	char datebuf[100];
	strcpy(datebuf,date_str);
	token=strtok(datebuf,seps_date);
	token=strtok(NULL,seps_date);
	token=strtok(NULL,seps_date);
	if(token!=NULL) return atol(token);
	////////////////////////////////////////////////////////////

	return 0;
}

double radfun(double adeg)
{
	double ret=adeg*PI/180;
	return ret;
}

double adegfun(double rad)
{
	double ret=rad/PI*180;
	return ret;
}

double adegfun(COMPLEX v)
{
	double rad=atan2(v.im,v.re);
	double ret=adegfun(rad);
	return ret;
}

bool isqstr(char *string)
{
	long n=strlen(string);
	for(long i=0;i<n;i++)
	if(!isqmark(string[i])) return false;
	if(n==0) return false;
	////////////////////////////////////////////////////////////

	return true;
}

bool islarge(double v)
{
	if(fabs(v)>LARGE/2) return true;
	return false;
}

bool islarge(PNTSTR point)
{
	double x=point.x;
	double y=point.y;
	if(fabs(x)>LARGE/2) return true;
	if(fabs(y)>LARGE/2) return true;
	////////////////////////////////////////////////////////////

	return false;
}

void natural(double *v,long n)
{
	for(long i=0;i<=n;i++) v[i]=i;
	return;
}

void negfun(double *v,long n)
{
	for(long i=0;i<=n;i++) v[i]=-v[i];
	return;
}

void negfun(double *v,double *v0,long n)
{
	for(long i=0;i<=n;i++) v[i]=-v0[i];
	return;
}

void transp(double **at,double **a,long m,long n)
{
	for(long i=1;i<=m;i++)
	for(long j=1;j<=n;j++) at[j][i]=a[i][j];
}

void transp(double **a,long n)
{
	for(long i=1;i<=n;i++)
	for(long j=i+1;j<=n;j++)
	{
		double aij=a[i][j];
		a[i][j]   =a[j][i];
		a[j][i]   =aij;
	}
	////////////////////////////////////////////////////////////

	return;
}

bool isleap(long year)
{
	if(year%100==0)
	if(year%400==0) return true;
	if(year%100!=0)
	if(year%4  ==0) return true;
	////////////////////////////////////////////////////////////

	return false;
}
/**
*judge whether the value of v is in [a,b]
*close space
*/
bool interv(double v,double a,double b)
{
	if(fabs(v-a)<SMLL) return true;
	if(fabs(v-b)<SMLL) return true;
	if(v>a && v<b)     return true;
	if(v>b && v<a)     return true;
	////////////////////////////////////////////////////////////

	return false;
}

COMPLEX setval0()
{
	COMPLEX ret=setval(0,0);
	return ret;
}

COMPLEX setval1()
{
	COMPLEX ret=setval(1,0);
	return ret;
}

COMPLEX conj(COMPLEX v)
{
	COMPLEX ret;
	ret.re= v.re;
	ret.im=-v.im; return ret;
}

COMPLEX setval(double re,double im)
{
	COMPLEX ret;
	ret.re= re;
	ret.im= im; return ret;
}

COMPLEX setvalp(double r,double rad)
{
	COMPLEX ret;
	ret.re= r*cos(rad);
	ret.im= r*sin(rad);
	return ret;
}

PNTSTR setpnt(double x,double y)
{
	PNTSTR ret; ret.x=x; ret.y=y;
	return ret;
}

bool isarg(char *arg,long argc,char **argv)
{
	for(long i=1;i<argc;i++)
	if(!strcmp(arg,argv[i])) return true;
	return false;
}

bool isnull(char *string)
{
	if(string==NULL)    return true;
	if(isspace(string)) return true;
	if(!strcmp(string,"NULL")) return true;
	if(!strcmp(string,"Null")) return true;
	if(!strcmp(string,"null")) return true;
	////////////////////////////////////////////////////////////

	return false;
}

long strncmp(char *str1,char *str2)
{ 
	long n1 =strlen(str1);
	long n2 =strlen(str2);
	long n  =min(n1,n2);
	long ret=strncmp(str1,str2,n);
	////////////////////////////////////////////////////////////

	return ret;
}

double avgfun(const double *v,long n)
{
	double ret=sum(v,n)/n;
	return ret;
}

double avgfun(const double *v,long i0,long n)
{
	double ret=sum(v,i0,n)/(n+1-i0);
	return ret;
}

double msefun(double *v,double *v0,long n)
{
	double sum=0;//init
	for(long i=1;i<=n;i++)
	{
		double dv=v[i]-v0[i];
		sum+= dv*dv;
	}
	////////////////////////////////////////////////////////////

	return sum/n;
}

double msefun(double *v,long n)
{
	double avg=avgfun(v,n);
	double sum=0;
	for(long i=1;i<=n;i++)
	{
		double dv=v[i]-avg;
		sum+= dv*dv;
	}
	////////////////////////////////////////////////////////////

	return sum/n;
}

long melfun(char *mel)
{
	if(!strcmp(mel,">"))  return MT;
	if(!strcmp(mel,"<"))  return LT;
	if(!strcmp(mel,"="))  return EQ;
	if(!strcmp(mel,">=")) return ME;
	if(!strcmp(mel,"<=")) return LE;
	if(!strcmp(mel,"!=")) return NE;
	////////////////////////////////////////////////////////////

	return NE;
}

bool iscond(double v,double v0,long mel)
{
	if(mel==MT && v>v0)      return true;
	if(mel==LT && v<v0)      return true;
	if(mel==ME && v>v0-SMLL) return true;
	if(mel==LE && v<v0+SMLL) return true;
	if(mel==EQ && fabs(v-v0)<SMLL) return true;
	if(mel==NE && fabs(v-v0)>SMLL) return true;
	////////////////////////////////////////////////////////////

	return false;
}

bool ismel(char ch)
{
	if(ch=='>') return true;
	if(ch=='<') return true;
	if(ch=='=') return true;
	////////////////////////////////////////////////////////////

	return false;
}

double sumcol(double **a,long j,long i0,long m)
{
	double ret=0;//init
	for(long i=i0;i<=m;i++) ret+= a[i][j];
	return ret;
}

double sumcol(double **a,long j,long m)
{
	double ret=0;//init
	for(long i=1;i<=m;i++) ret+= a[i][j];
	return ret;
}

double sumcol(double **a,long j,long m,double *v)
{
	double ret=0;//init
	for(long i=1;i<=m;i++) ret+= a[i][j]*v[i];
	return ret;
}

double sumrow(double **a,long i,long j0,long n)
{
	double ret=0;//init
	for(long j=j0;j<=n;j++) ret+= a[i][j];
	return ret;
}

double sumrow(double **a,long i,long n)
{
	double ret=0;//init
	for(long j=1;j<=n;j++) ret+= a[i][j];
	return ret;
}

double sumrow(double **a,long i,long n,double *v)
{
	double ret=0;//init
	for(long j=1;j<=n;j++) ret+= a[i][j]*v[j];
	return ret;
}

double sumrow(struct fspar *s,long i,double *v)
{
	double ret=0;//init
	for(long j=1;j<=s[i].n;j++)
	{
		long    jj=s[i].index[j];
		double sij=s[i].value[j];
		ret+= sij*v[jj];
	}
	////////////////////////////////////////////////////////////

	return ret;
}

double fabssum(double *v,long n)
{
	double ret=0;//init
	for(long i=1;i<=n;i++) ret+= fabs(v[i]);
	return ret;
}

double fabssum(double v1,double v2,double v3)
{
	double ret=fabs(v1)+fabs(v2)+fabs(v3);
	return ret;
}

double fabssum(double v1,double v2)
{
	double ret=fabs(v1)+fabs(v2);
	return ret;
}

double sum(const double *v,long n)
{
	double ret=sum(v,(long)1,n);
	return ret;
}

double sum(const double *v,long i0,long n)
{
	double ret=0;//init
	for(long i=i0;i<=n;i++) ret+= v[i];
	return ret;
}

double sum(const double *v1,double *v2,long n)
{
	double ret=0;//init
	for(long i=1;i<=n;i++)
	{
		double vv=v1[i]*v2[i];
		ret+= vv;
	}
	////////////////////////////////////////////////////////////

	return ret;
}

long mthdaysFun(long mth,long year)
{
	if(mth== 4) return 30;
	if(mth== 6) return 30;
	if(mth== 9) return 30;
	if(mth==11) return 30;
	if(mth== 2)
	{
		if(isleap(year)) return 29;
		else return 28;
	}
	////////////////////////////////////////////////////////////

	return 31;
}

long mthfun(char *month)
{
	if(!strncmp(month,"Jan")) return 1;
	if(!strncmp(month,"Feb")) return 2;
	if(!strncmp(month,"Mar")) return 3;
	if(!strncmp(month,"Apr")) return 4;
	if(!strncmp(month,"May")) return 5;
	if(!strncmp(month,"Jun")) return 6;
	if(!strncmp(month,"Jul")) return 7;
	if(!strncmp(month,"Aug")) return 8;
	if(!strncmp(month,"Sep")) return 9;
	if(!strncmp(month,"Oct")) return 10;
	if(!strncmp(month,"Nov")) return 11;
	if(!strncmp(month,"Dec")) return 12;
	////////////////////////////////////////////////////////////

	return 0;
}

void gettime(char *crrtime)
{
	char _crrdate[100];
	char _crrtime[100];
	getdate_time(_crrdate,_crrtime);
	strcpy(crrtime,_crrtime);
	////////////////////////////////////////////////////////////

	return;
}

void getdate(char *crrdate)
{
	char _crrdate[100];
	char _crrtime[100];
	getdate_time(_crrdate,_crrtime);
	strcpy(crrdate,_crrdate);
	////////////////////////////////////////////////////////////

	return;
}

void getdate_time(char *crrtime)
{
	char _crrdate[100];
	char _crrtime[100];
	getdate_time(_crrdate,_crrtime);
	sprintf(crrtime,"%s %s",_crrdate,_crrtime);
	////////////////////////////////////////////////////////////

	return;
}

void getdate_time(char *crrdate,char *crrtime)
{
	time_t t=time(NULL);
	struct tm *local=localtime(&t);
	long year=local->tm_year+1900;
	long mth =local->tm_mon+1;
	long day =local->tm_mday;
	long hour=local->tm_hour;
	long min =local->tm_min;
	long sec =local->tm_sec;
	sprintf(crrdate,"%ld/%ld/%ld",mth,day,year);
	sprintf(crrtime,"%ld:%02ld:%02ld",hour,min,sec);
	////////////////////////////////////////////////////////////

	return;
}

double fabsmax(double *v,long n)
{
	double ret=fabsmax(v,(long)1,n);
	return ret;
}

double fabsmax(double *v,long i0,long n)
{
	double ret=0;//init
	for(long i=i0;i<=n;i++)
	{
		double f=fabs(v[i]);
		if(f>ret) ret=f;
	}
	////////////////////////////////////////////////////////////

	return ret;
}

double fabsmaxErr(double *v,double *v0,long n)
{
	double ret=0;//init
	for(long i=1;i<=n;i++)
	{
		double f=fabs(v[i]-v0[i]);
		if(f>ret) ret=f;
	}
	////////////////////////////////////////////////////////////

	return ret;
}

double fabsmin(double *v,long n)
{
	double ret=fabs(v[1]);
	for(long i=1;i<=n;i++)
	{
		double f=fabs(v[i]);
		if(f<ret) ret=f;
	}
	////////////////////////////////////////////////////////////

	return ret;
}

double fabs(int v)
{
	double ret=fabs((double)v);
	return ret;
}

double fabs(long v)
{
	double ret=fabs((double)v);
	return ret;
}

double fabs(COMPLEX v)
{
	double ret=sqrt(v.re*v.re+v.im*v.im);
	return ret;
}

double fabs2(COMPLEX v)
{
	double ret=v.re*v.re+v.im*v.im;
	return ret;
}

double sqsum(double *v,long n)
{
	double sum=0;//init
	for(long i=1;i<=n;i++) sum+= v[i]*v[i];
	return sum;
}

double sqsum(double v1,double v2)
{
	double ret=v1*v1+v2*v2;
	return ret;
}

void delrow(double **a,long i0,long m,long n)
{
	for(long i=i0;i<m;i++) a[i]=a[i+1];
	return;
}

void delcol(double **a,long j0,long m,long n)
{
	for(long i=0;i<=m;i++)
	for(long j=j0;j<n;j++) a[i][j]=a[i][j+1];
	return;
}



void insert(char *string,char ch,long k)
{
	for(long i=strlen(string)+1;i>k;i--)
	string[i] =string[i-1];
	string[k] =ch;
}





void unitVector(double *v,long k,long n)
{
	initVector(v,n);
	v[k]=1;
}

void unitVector(COMPLEX *v,long k,long n)
{
	initVector(v,n);
	v[k]=setval1();
}

void unitrow(double **a,long i,long j,long n)
{
	initrow(a,i,n);
	a[i][j]=1;
}

void unitcol(double **a,long i,long j,long m)
{
	initcol(a,j,m);
	a[i][j]=1;
}



void unitMatrix(COMPLEX **a,long n)
{
    for(long i=0;i<=n;i++)
    for(long j=0;j<=n;j++)
    {
        if(i!=j) a[i][j]=setval0();
        if(i==j) a[i][j]=setval1();
    }
	////////////////////////////////////////////////////////////

    return;
}




void initrow(struct hspar **h,long i)
{
	struct hspar *hp=h[i];
	struct hspar *hpnext;
	while(hp!=NULL)
	{
		free(hp->value);
		free(hp->index); hpnext=hp->next;
		free(hp);        hp=hpnext;
	}	h[i]=NULL;
	////////////////////////////////////////////////////////////

    return;
}

void initrow(double **a,long i,long n)
{
	for(long j=0;j<=n;j++) a[i][j]=0;
	return;
}

void initcol(double **a,long j,long m)
{
	for(long i=0;i<=m;i++) a[i][j]=0;
	return;
}



void initMatrix(double **a,long m,long n)
{
	for(long i=0;i<=m;i++)
	for(long j=0;j<=n;j++) a[i][j]=0;
}

void initMatrix(double **a,long i0,long j0,long m,long n)
{
	for(long i=i0;i<=m;i++)
	for(long j=j0;j<=n;j++) a[i][j]=0;
}

bool zerorow(double **a,long i,long n)
{
	for(long j=0;j<=n;j++)
	if(fabs(a[i][j])>SMLL) return false;
	return true;
}

bool zerocol(double **a,long j,long m)
{
	for(long i=0;i<=m;i++)
	if(fabs(a[i][j])>SMLL) return false;
	return true;
}

void subVector(double *sub,double *v,long i0,long n)
{
    for(long i=i0;i<=n;i++) sub[i+1-i0]=v[i];
	return;
}

void subVector(double *sub,double *v,long *index,long n)
{
    for(long i=1;i<=n;i++) sub[i]=v[index[i]];
	return;
}

void subMatrix(double **sub,struct fspar *s,long i0,long j0,long m,long n)
{
	long m2=m+1-i0;
	long n2=n+1-j0; initMatrix(sub,m2,n2);
	for(long i=i0;i<=m;i++)
	for(long j=1;j<=s[i].n;j++)
	{
		long ii=i-i0+1;
		long jj=s[i].index[j]-j0+1;
		if(jj>0) sub[ii][jj]=s[i].value[j];
	}
	////////////////////////////////////////////////////////////

	return;
}

void subMatrix(double **sub,double **a,long i0,long j0,long m,long n)
{
	for(long i=i0;i<=m;i++)
	for(long j=j0;j<=n;j++)
	{
		long ii=i-i0+1;
		long jj=j-j0+1;
		sub[ii][jj]=a[i][j];
	}
	////////////////////////////////////////////////////////////

	return;
}

void subMatrix(double **sub,double **a,long *icol,long m,long n)
{
    for(long i=1;i<=m;i++)
    for(long j=1;j<=n;j++)
		sub[i][j]=a[i][icol[j]];
}

void copyrow(struct hspar **h,struct hspar **h0,long i)
{
	initrow(h,i);
	struct hspar *hp0=h0[i];
	while(hp0!=NULL)
	{
		struct hspar *hp=(struct hspar *)malloc(sizeof(struct hspar));
		if(hp==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
		memset(hp,0,sizeof(struct hspar));
		hp->nmax=hp0->nmax;
		hp->nh  =hp0->nh;
		hp->n   =hp0->n;
		sparmem(hp,h,i,hp->nmax);
		copy(hp->value,hp0->value,hp0->n);
		copy(hp->index,hp0->index,hp0->n);
		hp->next=h[i]; h[i]=hp;
		hp0=hp0->next;
	}
	////////////////////////////////////////////////////////////

	return;
}

void copyrow(struct fspar *s,struct fspar *s0,long i)
{
	s[i].n=s0[i].n;
	sparmem(s,i);
	copy(s[i].index,s0[i].index,s0[i].n);
	copy(s[i].value,s0[i].value,s0[i].n);
}



void copyrow(double *v,double **a,long i,long n)
{
	memcpy(v,a[i],(n+1)*sizeof(double));
	return;
}

void copyrow(double **a,double *v,long i,long n)
{
	memcpy(a[i],v,(n+1)*sizeof(double));
	return;
}

void copycol(double *v,double **a,long j,long m)
{
	for(long i=0;i<=m;i++) v[i]=a[i][j];
	return;
}

void copycol(double **a,double *v,long j,long m)
{
	for(long i=0;i<=m;i++) a[i][j]=v[i];
	return;
}




double fmax(double *v,long n)
{
	double ret=v[posmax(v,n)];
	return ret;
}

double fmin(double *v,long n)
{
	double ret=v[posmin(v,n)];
	return ret;
}

double nonzero(double *v,long n)
{
	for(long i=1;i<=n;i++)
	if(fabs(v[i])>SMLL) return v[i];
	return 0;
}

long posNonzero(double *v,long n)
{
	for(long i=1;i<=n;i++)
	if(fabs(v[i])>SMLL)	return i;
	return NO;
}

long posNonneg(double *v,long n)
{
	for(long i=1;i<=n;i++)
	{
		if(fabs(v[i])<SMLL) return i;
		if(v[i]>0) return i;
	}
	////////////////////////////////////////////////////////////
	
	return NO;
}

long posAbsmax(double *v,long n)
{
	long ret=posAbsmax(v,(long)1,n);
	return ret;
}

long posAbsmax(double *v,long i0,long n)
{
	double max=fabs(v[i0]);
	long  imax=i0;
	for(long i=i0;i<=n;i++)
	{
		double f=fabs(v[i]);
		if(f>max){ max=f; imax=i; }
	}
	////////////////////////////////////////////////////////////

	return imax;
}

long posmax(double *v,long n)
{
	long ret=posmax(v,(long)1,n);
	return ret;
}

long posmax(double *v,long i0,long n)
{
	double max=v[i0];
	long  imax=i0;
	for(long i=i0;i<=n;i++)
	if(v[i]>max){ max=v[i]; imax=i; }
	////////////////////////////////////////////////////////////

	return imax;
}







void sorting(double *v,long *index,long n)
{
    for(long i=1;i<n;i++)
    {
        long imin=posmin(v,i,n);
        if(imin!=i) swap(v,i,imin);
        if(imin!=i) swap(index,i,imin);
    }
	////////////////////////////////////////////////////////////

    return;
}



void addMatrix(double **a,double **add,long m,long n)
{
	for(long i=0;i<=m;i++)
	for(long j=0;j<=n;j++)
		a[i][j]+= add[i][j];
}

void addrow(double **a,double *v,long i,long n)
{
	for(long j=0;j<=n;j++) a[i][j]+= v[j];
	return;
}

void addrow(double **a,double mul,long i,long i0,long n)
{
    for(long j=0;j<=n;j++) a[i][j]+= mul*a[i0][j];
    return;
}

void addrow(double **a,double mul,long i,long i0,long j0,long n)
{
    for(long j=j0;j<=n;j++) a[i][j]+= mul*a[i0][j];
    return;
}

void addrow(double *v,double mul,FSPAR *s,long i)
{
	for(long j=1;j<=s[i].n;j++)
	{
		long    jj=s[i].index[j];
		double sij=s[i].value[j];
		v[jj]+= mul*sij;
	}
	////////////////////////////////////////////////////////////

	return;
}

void addVector(double *v,double *add,long i0,long n)
{
    for(long i=i0;i<=n;i++) v[i]+= add[i];
	return;
}

void addVector(double *v,double *add,long n)
{
    for(long i=0;i<=n;i++) v[i]+= add[i];
	return;
}

/**
*array sub
*every value of v sub the coodinate value of subtr
*/
void subtrfun(double *v,double *subtr,long n)
{
	for(long i=0;i<=n;i++) v[i]-= subtr[i];
	return;
}




void divVector(double *v,double div,long n)
{
	for(long i=0;i<=n;i++) v[i]/= div;
	return;
}

void mulVector(double *v,double mul,long n)
{
	for(long i=0;i<=n;i++) v[i]*= mul;
	return;
}

void mulMatrix(double **a,double mul,long m,long n)
{
    for(long i=0;i<=m;i++)
    for(long j=0;j<=n;j++) a[i][j]*= mul;
}

void mulMatrix(double **a,double mul,long i0,long j0,long m,long n)
{
    for(long i=i0;i<=m;i++)
    for(long j=j0;j<=n;j++) a[i][j]*= mul;
}

void mulMatrix(double **a,double **a1,double **a2,long m,long l,long n)
{
    for(long i=1;i<=m;i++)
    for(long j=1;j<=n;j++)
    {
        double sum=0;//init
        for(long k=1;k<=l;k++) sum+= a1[i][k]*a2[k][j];
		a[i][j]=sum;
    }
	////////////////////////////////////////////////////////////

    return;
}

void mulVector(double *v,double **a,double *v0,long m,long n)
{
	for(long i=1;i<=m;i++)
	{
		double sum=0;//init
		for(long k=1;k<=n;k++) sum+= a[i][k]*v0[k];
		v[i]=sum;
	}
	////////////////////////////////////////////////////////////
	
	return;
}

void mulrow(double **a,double mul,long i,long n)
{
	for(long j=0;j<=n;j++) a[i][j]*= mul;
	return;
}

void mulcol(double **a,double mul,long j,long m)
{
	for(long i=0;i<=m;i++) a[i][j]*= mul;
	return;
}

void drawline(double n)
{
    printf("\n");
	for(long i=0;i<n;i++) printf("-");
	return;
}

void fdrawline(FILE *fp,long n)
{
    fprintf(fp,"\n");
    for(long i=0;i<n;i++) fprintf(fp,"-");
    return;
}

//long round(double v)
//{
//	long ret=(long)(fabs(v)+0.5)*sign(v);
//	return ret;
//}

void initqstr(char *qstr,char *string)
{
	char *cp1=strstr(string,"\'");
	char *cp2=NULL; initstr(qstr);
	if(cp1!=NULL) cp2=strstr(cp1+1,"\'");
	if(cp2!=NULL)
	{
		long n=(long)(cp2-cp1+1);
		strncpy(qstr,cp1,n);
		strcut (qstr,n);
		memset (cp1,'\'',n);
	}
	////////////////////////////////////////////////////////////

	return;
}

void initstr(char *string)
{
	string[0]=NULL;
	return;
}

void strcut(char *string,long n)
{
	string[n]=NULL;
	return;
}

void strcut(char *string,char ch)
{
	char *cp=strchr(string,ch);
	if(cp!=NULL)
	{
		long n=(long)(cp-string);
		string[n]=NULL;
	}
	////////////////////////////////////////////////////////////

	return;
}

long getval(struct spar *s,long i,long j)
{
	for(long jj=1;jj<=s[i].n;jj++)
	if(s[i].index[jj]==j)
	return s[i].value[jj];
	return 0;
}

double getval(struct fspar *s,long i,long j)
{
	for(long jj=1;jj<=s[i].n;jj++)
	if(s[i].index[jj]==j)
	return s[i].value[jj];
	return 0;
}

COMPLEX getval(struct cspar *s,long i,long j)
{
	for(long jj=1;jj<=s[i].n;jj++)
	if(s[i].index[jj]==j)
	return s[i].value[jj];
	return setval0();
}

void sleep(long sec){ sleep(1000*sec); }
void sleepp(long sec)
{
	while(1)
	{
		sleep(1);
		gettime(crrtime);
		long t=timefun(crrtime);
		if(t%sec==0) break;
	}
	////////////////////////////////////////////////////////////

	return;
}

void pausemy()
{
	fflush(stdin);
	printf("\nPress ENTER to continue...");
	scanf("%c");
}
//end of file
