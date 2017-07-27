//common.h by wanggt
////////////////////////////////////////////////////////////
#ifndef _COMMON_H
#define _COMMON_H
////////////////////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<time.h>
//#pragma warning(disable:4267)
//#pragma warning(disable:4996)
////////////////////////////////////////////////////////////
//constants
#define PI      3.1415926535
#define E3      1.0E+03
#define E6      1.0E+06
#define TOL     1.0E-06
#define SMLL    1.0E-10
#define SMLL2   1.0E-20
#define LARGE   1.0E+20
#define LARGE2  1.0E+20
////////////////////////////////////////////////////////////
//status/relation
#define ON_LINE   0
#define ON_LINEX  1
#define ON_LEFT   2
#define ON_RIGHT  3
#define UPPER     1
#define LOWER     0
#define YES    1  //success
#define NO    -1  //failed
#define EQ     0  //equal
#define MT     1  //more than
#define LT    -1  //less than
#define LE     2
#define ME     3
#define NE     4
////////////////////////////////////////////////////////////
//macro functions
#define hash10(n)  ((n)/10)
#define hash100(n) ((n)/100)
#define nmax10(n)  (((n)/10+1)*10)
#define nmax100(n) (((n)/100+1)*100)
#ifndef max
#define max(a,b)   (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b)   (((a)<(b))?(a):(b))
#endif
////////////////////////////////////////////////////////////
#define sincos(x)  (sin(x)*cos(x))
#define sin2(x)    (sin(x)*sin(x))
#define cossin(x)  (cos(x)*sin(x))
#define cos2(x)    (cos(x)*cos(x))
#define pow2(x)    ((x)*(x))
#define pow3(x)    ((x)*(x)*(x))
////////////////////////////////////////////////////////////
//file/path names
extern char basfile[100]; //1: base file
extern char bmpfile[100]; //2: bmp  file
extern char datfile[100]; //3: data file
extern char dmpfile[100]; //4: dump file
extern char logfile[100]; //5: log  file
extern char matfile[100]; //6: matlab file
extern char rawfile[100]; //7: psse/raw file
extern char resfile[100]; //8: result file
extern char tmpFile[100]; //9: temp file
extern char datpath[100]; //10: data path
extern char hispath[100]; //11: history path
extern char respath[100]; //12: result path
extern char osipath[100]; //13: osi/monarch
extern char userpath[100];//14: osi/user
////////////////////////////////////////////////////////////
//debug level/file
extern char  debug_level;
extern char  debug_file[100];
extern FILE *debug_fp;
extern clock_t clock0;
////////////////////////////////////////////////////////////
//date/time and ret
extern char start_time[100];
extern char end_time  [100];
extern char case_str  [100];
extern char date_str  [100];
extern char time_str  [100];
extern char crrdate   [100];
extern char crrtime   [100];
extern double f1,f2,f3,f4,f5,f6,f7,f8,f9;
extern double fa,fb,fc,fd,fe,ff;
extern long   ia,ib,ic,id,ie;
extern long   ret;
////////////////////////////////////////////////////////////
//buffer, token/seps
extern char buffer[1000];
extern char buf   [1000];
extern char seg1  [100];
extern char seg2  [100];
extern char seg3  [100];
extern char seg4  [100];
extern char seg5  [100];
extern char seg6  [100];
extern char seg7  [100];
extern char seg8  [100];
extern char seg9  [100];
extern char *token;
#define seps " ,;@/\\\t\n"
#define seps_date ".-/\\"
#define seps_time ":"
////////////////////////////////////////////////////////////
//complex
typedef struct complex
{
	double re;
	double im;
}COMPLEX;
////////////////////////////////////////////////////////////
//pntstr
typedef struct pntstr
{
	double x;
	double y;
//	double z;
}PNTSTR;
////////////////////////////////////////////////////////////
//spar/cspar/fspar/hspar
typedef struct spar
{
	long *value;
	long *index;
	long  nmax;
	long  n;
}SPAR;
////////////////////////////////////////////////////////////
typedef struct cspar
{
	COMPLEX *value;
	long    *index;
	long    nmax;
	long    n;
}CSPAR;
////////////////////////////////////////////////////////////
typedef struct fspar
{
	double *value;
	long   *index;
	long   nmax;
	long   n;
}FSPAR;
////////////////////////////////////////////////////////////
typedef struct hspar
{
	struct hspar *next;
	double *value;
	long   *index;
	long   nmax;
	long   nh;
	long   n;
}HSPAR;
////////////////////////////////////////////////////////////
//function prototypes
bool isarg(char *arg,long argc,char **argv);
bool iscond(double v,double v0,long mel);
bool isfind(char *filename);
bool islarge(double v);
bool islarge(PNTSTR point);
bool isleap(long year);
bool isnull(char *string);
bool ismel (char ch);
bool isqmark(char ch);
bool isqstr (char *string);
bool isspace(char *string);
bool issymm(double **a,long n);
bool interv(double v,double a,double b);
bool inside(PNTSTR point0,PNTSTR *point,long n);
bool posdata(FILE *fp,char *data);
bool posdata(FILE *fp,long n);
bool zerorow(double **a,long i,long n);
bool zerocol(double **a,long j,long m);
////////////////////////////////////////////////////////////

COMPLEX conj(COMPLEX v);
COMPLEX pow (COMPLEX v,long n);
COMPLEX getval(struct cspar *s,long i,long j);
COMPLEX setval (double re,double im);
COMPLEX setvalp(double r,double rad);
COMPLEX setval1();
COMPLEX setval0();
COMPLEX operator+(COMPLEX v1,COMPLEX v2);
COMPLEX operator-(COMPLEX v1,COMPLEX v2);
COMPLEX operator-(COMPLEX v1);
COMPLEX operator*(COMPLEX v1,COMPLEX v2);
COMPLEX operator/(COMPLEX v1,COMPLEX v2);

HSPAR *findhp(HSPAR **h,long i,long j);
PNTSTR setpnt(double x,double y);
PNTSTR crosspnt(PNTSTR p11,PNTSTR p12,PNTSTR p21,PNTSTR p22);
void operator+= (COMPLEX &v1,COMPLEX v2);
void operator-= (COMPLEX &v1,COMPLEX v2);
void operator*= (COMPLEX &v1,COMPLEX v2);
void operator/= (COMPLEX &v1,COMPLEX v2);
////////////////////////////////////////////////////////////

double adegfun(double rad);
double adegfun(COMPLEX v);
double avgfun(double *v,long i0,long n);
double avgfun(double *v,long n);
double getfval(char *string,long n);
double getfval(char *string);
double getval(struct fspar *s,long i,long j);
double getval(HSPAR **h,long i,long j);
double fabs(int v);
double fabs(long v);
double fabs (COMPLEX v);
double fabs2(COMPLEX v);
double fabsmaxErr(double *v,double *v0,long n);
double fabsmax(double *v,long i0,long n);
double fabsmax(double *v,long n);
double fabsmin(double *v,long n);
double fabssum(double *v,long n);
double fabssum(double v1,double v2);
double fabssum(double v1,double v2,double v3);
double fmaxdv(double *v,double *v0,long n);
double fmaxdv(double *v,long n);
double fmax (double *v,long n);
double fmin (double *v,long n);
double frand(double v1,double v2);
double frand();
////////////////////////////////////////////////////////////

double interp(double x,double x1,double y1,double x2,double y2);
double interp(double x1,double x2,double r);
double interp(double x1,double *x,double *y,long n);
double lineq_a0(double x1,double y1,double x2,double y2);
double lineq_a1(double x1,double y1,double x2,double y2);
double linelen(PNTSTR p1,PNTSTR p2);
double msefun (double *v,double *v0,long n);
double msefun (double *v,long n);
double nonzero(double *v,long n);
double normal (double v1,double v2);
double normal (double x);
////////////////////////////////////////////////////////////

double PID(double *e,long t,double Kp,double Ti,double Td);
double pow(double x,long n);
double radfun(double adeg);
double rmsfun(double *x,long n);
double sigmfun(double x);
double simpfun(double (*fun)(double),double a,double b);
double solve(double (*fun)(double),double x0,double x1);
double sqsum(double v1,double v2);
double sqsum(double *v,long n);
double sum(double **a,long m,long n);
double sum(double *a,double x,long n);
double sum(double *v,long n);
double sum(double *v,long i0,long n);
double sum(double *v1,double *v2,long n);
double sumcol(double **a,long j,long m);
double sumcol(double **a,long j,long m,double *v);
double sumcol(double **a,long j,long i0,long m);
double sumrow(double **a,long i,long n);
double sumrow(double **a,long i,long n,double *v);
double sumrow(double **a,long i,long j0,long n);
double sumrow(struct fspar *s,long i,double *v);
////////////////////////////////////////////////////////////

template<class TYPE>
long chainlen(TYPE *chain)
{
	long ret=0;//init.
	TYPE *cp=chain;
	while(cp!=NULL){ ret++; cp=cp->next; }
	return ret;
}

template<class TYPE>
long colnumStat(TYPE *s,long m)
{
	long ret=0;//init
	for(long i=1;i<=m;i++)
	for(long j=1;j<=s[i].n;j++)
		ret=max(ret,s[i].index[j]);
	////////////////////////////////////////////////////////////

	return ret;
}
long cavgStat(double *v,long i0,long n);
long datediff(char *date_str,char *date0);
long datefun0(char *date_str);
long datefun (char *date_str);
long getdate_mth (char *date_str);
long getdate_day (char *date_str);
long getdate_year(char *date_str);
long gettime_hour(char *time_str);
long gettime_min (char *time_str);
long gettime_sec (char *time_str);
long getval(char *string);
long getval(struct spar *s,long i,long j);
////////////////////////////////////////////////////////////

long lookup(long v1,long v2,long *v,long n);
long lookup(long v0,long *v,long n);
long lookup(double v0,double *v,long n);
long lookup2(double v0,double *v,long n);
long LORfun(PNTSTR point,PNTSTR p1,PNTSTR p2);
long melfun(char *mel);
long mthfun(char *month);
long mthdaysFun(long mth,long year);
long posAbsmax(double *v,long n);
long posAbsmax(double *v,long i0,long n);
long posmaxdv(double *v,long n);
long posmax(double *v,long n);
long posmax(double *v,long i0,long n);
////////////////////////////////////////////////////////////
template<class TYPE>
long posmin(TYPE *v,long i0,long n)
{
    double min=v[i0];
	long  imin=i0;
    for(long i=i0;i<=n;i++)
	if(v[i]<min){ min=v[i]; imin=i; }
	////////////////////////////////////////////////////////////

    return imin;
}
template<class TYPE>
long posmin(TYPE *v,long n)
{
	long ret=posmin(v,(long)1,n);
	return ret;
}



template <class TYPE>
long posnode(TYPE *node,TYPE *chain)
{
	TYPE *cp=chain;	long ret=0;
	while(cp!=NULL)
	{
		ret++;
		if(cp==node) return ret;
		cp=cp->next;
	}
	////////////////////////////////////////////////////////////

	return NO;
}
long posNonneg(double *v,long n);
long posNonzero(double *v,long n);
long pospp(double **a,long k,long m);
long pospp(struct fspar *s,long k,long m);
////////////////////////////////////////////////////////////

//long round(double v);
long sign (double v);
long sign (char ch);
long sdfun(char *time_str,long sdnum);
long strchar(char *string,char ch);
long strncmp(char *str1,char *str2);
long strseg (char *string);
long timediff(char *time_str,char *time0);
long timefun(char *time_str);
long wdayfun(char *date_str);
////////////////////////////////////////////////////////////

template<class TYPE>
void addnode(TYPE *node,TYPE *&chain)
{
	TYPE *cp=chain;
	if(chain==NULL){ chain=node; return; }
	while(cp!=NULL)
	{
		if(cp->next==NULL){ cp->next=node; break; }
		cp=cp->next;
	}
    ////////////////////////////////////////////////////////////

	return;
}
void addrow(double **a,double mul,long i,long i0,long n);
void addrow(double **a,double mul,long i,long i0,long j0,long n);
void addrow(double **a,double *v,long i,long n);
void addrow(double *v,double mul,FSPAR *s,long i);

template<class T_spar,class TYPE>
void addval(T_spar *s,long i,long j,TYPE add)
{
	TYPE sij=getval(s,i,j);
	setval(s,i,j,sij+add);
}
void addMatrix(double **a,double **add,long m,long n);
void addVector(double *v,double *add,long n);
void addVector(double *v,double *add,long i0,long n);
void andset(long *v,long &n,long *v1,long n1,long *v2,long n2);
void orset(long *v,long &n,long *v1,long n1,long *v2,long n2);
////////////////////////////////////////////////////////////

/**
*wrap the memcpy,this function copy space with the unit of type,rather than byte
*
*/
template<class TYPE>
void copy(TYPE *dest,TYPE *src,long n0)
{
	if(n0>0) memcpy(dest,src,(n0+1)*sizeof(TYPE));
	return;
}

template<class TYPE>
void copy(TYPE **a,TYPE **a0,long m,long n)
{
	for(long i=0;i<=m;i++) copy(a[i],a0[i],n);
	return;
}
void copy(double *v,long i,long i0,long n);
void copy(double *v,double *v0,long i0,long n);

template<class TYPE>
void copyspar(TYPE *s,TYPE *s0,long m)
{
	for(long i=0;i<=m;i++)
	{
		s[i].n=s0[i].n;
		sparmem(s,i);
		copy(s[i].index,s0[i].index,s0[i].n);
		copy(s[i].value,s0[i].value,s0[i].n);
	}
	////////////////////////////////////////////////////////////

	return;
}

////////////////////////////////////////////////////////////
template<class TYPE>
void copyrow(TYPE *s,long i,long i0)
{
	s[i].n=s[i0].n;
	sparmem(s,i);
	copy(s[i].index,s[i0].index,s[i0].n);
	copy(s[i].value,s[i0].value,s[i0].n);
	////////////////////////////////////////////////////////////

	return;
}

void copyrow(struct hspar **h,struct hspar **h0,long i);
void copyrow(struct fspar *s,struct fspar *s0,long i);
void copycol(struct fspar *s,double *v,long j,long m);
void copycol2(struct fspar *s,double *v,long j,long m);
void copyrow(double **a,double *v,long i,long n);
void copyrow(double *v,double **a,long i,long n);
void copycol(double **a,double *v,long j,long m);
void copycol(double *v,double **a,long j,long m);
////////////////////////////////////////////////////////////
template<class TYPE>
void delelm(TYPE *v,long i0,long n)
{
	for(long i=i0;i<n;i++) v[i]=v[i+1];
	return;
}

template<class TYPE>
void delelm_spar(TYPE *s,long i,long j)
{
	long k=lookup(j,s[i].index,s[i].n);
	if(k!=NO)
	{
		delelm (s[i].value,k,s[i].n);
		delelm (s[i].index,k,s[i].n); s[i].n--;
		sparmem(s,i);
	}
	////////////////////////////////////////////////////////////

	return;
}

void delelm_spar(HSPAR **h,long i,long j);

template<class TYPE>
void delnode(TYPE *node,TYPE *&chain)
{
	if(node==chain)
	{
		chain=chain->next;
		free(node); return;
	}
    ////////////////////////////////////////////////////////////

	TYPE *cp=chain;
    while(cp!=NULL)
    {
        if(cp->next==node)
        {
			cp->next=node->next;
			free(node); break;
        }	cp=cp->next;
    }
    ////////////////////////////////////////////////////////////

    return;
}

void delrow(double **a,long i0,long m,long n);
void delcol(double **a,long j0,long m,long n);
void delcol(struct fspar *s,long j0,long m);
////////////////////////////////////////////////////////////
template<class TYPE>
void divrow(TYPE **a,TYPE div,long i,long n)
{
	for(long j=0;j<=n;j++) a[i][j]/= div;
	return;
}

template<class TYPE>
void divrow(TYPE **a,TYPE div,long i,long j0,long n)
{
	for(long j=j0;j<=n;j++) a[i][j]/= div;
	return;
}

void divVector(double *v,double div,long n);
void drawline (double n);
void fdrawline(FILE *fp,long n);
void frand(double *v,long n);
////////////////////////////////////////////////////////////
template<class TYPE>
void freespar(TYPE *s,long m)
{
	for(long i=0;i<m;i++)
	{
		free(s[i].value);
		free(s[i].index);
	}	free(s);
	////////////////////////////////////////////////////////////

	return;
}

void freespar(HSPAR **h,long m);

template<class TYPE>
void freeChain(TYPE *chain)
{
	TYPE *cpnext;
	TYPE *cp=chain;
	while(cp!=NULL)
	{
		cpnext=cp->next; free(cp);
		cp=cpnext;
	}
	////////////////////////////////////////////////////////////

	return;
}

template<class TYPE>
void free(TYPE **a,long m)
{
	for(long i=0;i<m;i++)
	free(a[i]);
	free(a);
}

////////////////////////////////////////////////////////////

void gausslu(struct fspar *s,double *b,long n);
void gausspp(struct fspar *s,double *b,long n);
void gausspp(double **a,double *b,long n);
void getfile(char *file,char *pathfile);
void getval (char *value,char *string);
void getdate_time(char *crrdate,char *crrtime);
void getdate_time(char *crrtime);
void getdate(char *crrdate);
void gettime(char *crrtime);
void getpnt_x(double *x,long n,PNTSTR *pnt);
void getpnt_y(double *y,long n,PNTSTR *pnt);
////////////////////////////////////////////////////////////


template<class TYPE>
void initrow(TYPE *s,long i)
{
	free(s[i].value);
	free(s[i].index);
	s[i].value=NULL;
	s[i].index=NULL;
	s[i].nmax =0;
	s[i].n    =0;
	////////////////////////////////////////////////////////////

    return;
}
void initrow(struct hspar **h,long i);
void initrow(double **a,long i,long n);
void initcol(double **a,long j,long m);

template<class TYPE>
void initspar(TYPE *s,long m)
{
	for(long i=0;i<=m;i++) initrow(s,i);
	return;
}

void initstr (char *string);
void initqstr(char *qstr,char *string);
void initMatrix(double **a,long i0,long j0,long m,long n);
void initMatrix(double **a,long m,long n);
////////////////////////////////////////////////////////////
template<class TYPE>
void initVector(TYPE *v,long n)
{
	memset(v,0,(n+1)*sizeof(TYPE));
	return;
}

void initVector(double *v,long i0,long n);
void initVector(double *v,long n,double v0);

template<class TYPE>
void insert(TYPE *v,TYPE vk,long k,long n)
{
	for(long i=n+1;i>k;i--)
	v[i]=v[i-1];
	v[k]=vk;
}

void insert(char *string,char ch,long k);
void invfun(double **a,long n);
void lufun(double **lu,double **a,long n);
void lufun(struct fspar *lu,struct fspar *s,long n);
void lu_forwd(struct fspar *lu,double *b,long n);
void lu_back (struct fspar *lu,double *b,long n);
////////////////////////////////////////////////////////////

template<class TYPE>
void makeMatrix(TYPE **&a,long m,long n)
{
	a=(TYPE **)calloc(m,sizeof(TYPE *));
	if(a==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
	for(long i=0;i<m;i++)
	{
		a[i]=(TYPE *)calloc(n,sizeof(TYPE));
		if(a[i]!=NULL) memset(a[i],0,n*sizeof(TYPE));
//		if(a[i]==NULL){ printf("\nError --- %ld",__LINE__); exit(0); }
	}
	////////////////////////////////////////////////////////////

    return;
}


template<class TYPE>
void makespar(TYPE *&s,long m)
{
    s=(TYPE *)calloc(m,sizeof(TYPE));
	if(s!=NULL) memset(s,0,m*sizeof(TYPE));
	if(s==NULL)
	{
		printf("\nError --- %ld",__LINE__);
		exit(0);
	}
	////////////////////////////////////////////////////////////

	return;
}
void makespar(HSPAR **&h,long m);
void mulcol(double **a,double mul,long j,long m);
void mulrow(double **a,double mul,long i,long n);
void mulrow(struct fspar *s,double mul,long i);
void mulrow(struct hspar **h,double mul,long i);
void mulspar(FSPAR *s,FSPAR *s1,FSPAR *s2,long m,long l,long n);
void mulspar(CSPAR *s,CSPAR *s1,CSPAR *s2,long m,long l,long n);
void mulspar(double *v,FSPAR *s,double *v0,long m,long n);
void mulMatrix(double **a,double mul,long i0,long j0,long m,long n);
void mulMatrix(double **a,double mul,long m,long n);
void mulMatrix(double **a,double **a1,double **a2,long m,long l,long n);
void mulVector(double *v,double **a,double *v0,long m,long n);
void mulVector(double *v,double *mul,long n);
void mulVector(double *v,double  mul,long n);
////////////////////////////////////////////////////////////

void natural(double *v,long n);
void negfun(double *v,long n);
void negfun(double *v,double *v0,long n);
void normal(double *v,long n);
void normal(double *v,long n,double vmin,double vmax);
void print(COMPLEX **a,long m,long n,char *descr);
void print(COMPLEX *v,long n,char *descr);
void print(double **a,long m,long n,char *descr);
void print(double *v,long n,char *descr);
void print(long *v,long n,char *descr);
void print(char *string,long n);
void print(char *string);
void pausemy();
////////////////////////////////////////////////////////////

template<class TYPE>
void reverse(TYPE *&chain)
{
	TYPE *front=NULL;
	while(chain!=NULL)
	{
		TYPE *next=chain->next;
		chain->next=front;
		front=chain;
		chain=next;
	}	chain=front;
    ////////////////////////////////////////////////////////////

	return;
}
void reverse(double *v,long n);
void reverse(double *v,long i0,long n);
void read(double **a,long m,long n,char *filename);
void read(double *v,long n,char *filename);
////////////////////////////////////////////////////////////

template<class T_spar,class TYPE>
void setval(T_spar *s,long i,long j,TYPE v)
{
	if(fabs(v)<SMLL)
	{
//		printf("\nWarning --- %ld",__LINE__);
		delelm_spar(s,i,j);
		return;
	}
	////////////////////////////////////////////////////////////

	long k=lookup(j,s[i].index,s[i].n);
	if(k!=NO){ s[i].value[k]=v; return; }
	s[i].n++;  sparmem(s,i);
	s[i].index[s[i].n]=j;
	s[i].value[s[i].n]=v;
	////////////////////////////////////////////////////////////

	return;
}
void setval(HSPAR **h,long i,long j,double v);
void setval2(HSPAR **h,long i,long j,double v);

template<class T_spar,class TYPE>
void setval2(T_spar *s,long i,long j,TYPE v)
{
	s[i].n++; sparmem(s,i);
	s[i].index[s[i].n]=j;
	s[i].value[s[i].n]=v;
	////////////////////////////////////////////////////////////

	return;
}

void sdfun(char *time_str,long sd,long sdnum);
void sleep(char *time_str);
void sleep(long sec);
void sleepp(long sec);

template<class TYPE>
void sorting(TYPE *v,long n)
{
    for(long i=1;i<n;i++)
    {
        long imin=posmin(v,i,n);
        if(imin!=i) swap(v,i,imin);
    }
	////////////////////////////////////////////////////////////

    return;
}
void sorting(double *v,long *index,long n);
////////////////////////////////////////////////////////////

template<class TYPE>
void sparcol(TYPE *scol,TYPE *s,long m)
{
	long n=colnumStat(s,m); initspar(scol,n);
	for(long i=1;i<=m;i++)
	for(long j=1;j<=s[i].n;j++)
	{
		long jj=s[i].index[j];
		scol[jj].n++; sparmem(scol,jj);
		scol[jj].index[scol[jj].n]=i;
		scol[jj].value[scol[jj].n]=s[i].value[j];
	}
	////////////////////////////////////////////////////////////

	return;
}
void sparfun(struct fspar *s,double **a,long m,long n);
void sparfun(struct fspar *s,long i,double *v,long n);

template<class TYPE>
void sparmem(TYPE *s,long i)
{
	long nmax=nmax10(s[i].n);
	if(nmax!=s[i].nmax)
		sparmem(s,i,nmax);
}
void sparmem(struct fspar *s,long i,long nmax);
void sparmem(struct cspar *s,long i,long nmax);
void sparmem(struct  spar *s,long i,long nmax);
void sparmem(HSPAR *hp,HSPAR **h,long i);
void sparmem(HSPAR *hp,HSPAR **h,long i,long nmax);
void srand();
void strcut(char *string,long n);
void strcut(char *string,char ch);
void strrep(char *string,char *str,char *rep);
void strseg(char *seg,char *string,long k);
void strseg(char *seg,char *string,long k,long k2);
void strpack(char *string);
void strtrim(char *string);
////////////////////////////////////////////////////////////

template<class TYPE>
void subtrspar(TYPE *s,TYPE *subtr,long m)
{
	for(long i=1;i<=m;i++)
	for(long j=1;j<=subtr[i].n;j++)
	{
		long jj=subtr[i].index[j];
		addval(s,i,jj,-subtr[i].value[j]);
	}
	////////////////////////////////////////////////////////////

	return;
}
void subtrfun (double *v,double *subtr,long n);
void subtrfun (double **a,double **subtr,long m,long n);
void subMatrix(double **sub,struct fspar *s,long i0,long j0,long m,long n);
void subMatrix(double **sub,double **a,long i0,long j0,long m,long n);
void subMatrix(double **sub,double **a,long *icol,long m,long n);
void subVector(double *sub,double *v,long *index,long n);
void subVector(double *sub,double *v,long i0,long n);
////////////////////////////////////////////////////////////

template<class TYPE>
void swap(TYPE &v1,TYPE &v2)
{
	TYPE _v1=v1;
	v1 =  v2;
	v2 = _v1;
}

template<class TYPE>
void swap(TYPE *v,long i,long j)
{
	TYPE vi=v[i];
	v[i]=v[j];
	v[j]=vi;
}

template<class TYPE>
void swap(TYPE **a,long i0,long j0,long i,long j)
{
	TYPE  a0 =a[i0][j0];
	a[i0][j0]=a[i ][j ];
	a[i ][j ]=a0;
}

template<class TYPE>
void swaprow(TYPE **a,long i0,long i)
{
	TYPE *a0=a[i0];
	a[i0]=a[i];
	a[i ]=a0;
}
void timefun(char *time_str,long sec);
void transp(double **a,long n);
void transp(double **at,double **a,long m,long n);
////////////////////////////////////////////////////////////

template<class TYPE>
void unitMatrix(TYPE **a,long n)
{
    for(long i=1;i<=n;i++)
    for(long j=1;j<=n;j++)
    {
        if(i!=j) a[i][j]=0;
        if(i==j) a[i][j]=1;
    }
	////////////////////////////////////////////////////////////

    return;
}
void unitMatrix(COMPLEX **a,long n);
void unitVector(COMPLEX *v,long k,long n);
void unitVector(double *v,long k,long n);
void unitrow(double **a,long i,long j,long n);
void unitcol(double **a,long i,long j,long m);

template<class T_spar,class TYPE>
void unspar(TYPE *v,long n,T_spar *s,long i)
{
	memset(v,0,(n+1)*sizeof(double));
	for(long j=1;j<=s[i].n;j++)
	{
		long   jj=s[i].index[j];
		TYPE  sij=s[i].value[j];
		v[jj]=sij;
	}
	////////////////////////////////////////////////////////////

	return;
}
void unspar(double *v,long k,long n,FSPAR *s,long i);
void unspar(double **a,struct fspar *s,long m,long n);
////////////////////////////////////////////////////////////


template<class TYPE>
TYPE *finddata(long ibs,TYPE *chain)
{
    TYPE *cp=chain;
    while(cp!=NULL)
    {
        if(cp->i==abs(ibs)) return cp;
        cp=cp->next;
    }
    ////////////////////////////////////////////////////////////

    return NULL;
}

template<class TYPE>
TYPE *finddata(long ibs,long zbs,TYPE *chain)
{
    TYPE *cp=chain;
    while(cp!=NULL)
    {
        if(abs(ibs)==abs(cp->i))
		if(abs(zbs)==abs(cp->j)) return cp;
        if(abs(ibs)==abs(cp->j))
		if(abs(zbs)==abs(cp->i)) return cp;
        cp=cp->next;
    }
    ////////////////////////////////////////////////////////////

    return NULL;
}
template<class TYPE>
TYPE *finddata(char *id,TYPE *chain)
{
    TYPE *cp=chain;
    while(cp!=NULL)
    {
        if(!strcmp(id,cp->id)) return cp;
        cp=cp->next;
    }
    ////////////////////////////////////////////////////////////

    return NULL;
}
template<class TYPE>
TYPE *finddata_i(long i,TYPE *chain)
{
	TYPE *cp=chain; long k=0;
	while(cp!=NULL)
	{
		k++;
		if(k==i) return cp;
		cp=cp->next;
	}
    ////////////////////////////////////////////////////////////

	return NULL;
}
////////////////////////////////////////////////////////////

#endif
