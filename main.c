#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <time.h>
#define MAXW 8
static volatile sig_atomic_t running=1;
static pthread_t th[MAXW]; static int workers=2, memmb=128, duration=15;
static unsigned char *block=NULL;
static void stop_handler(int s){(void)s;running=0;}
static void *work(void *p){long id=(long)p; volatile double x=.123+id; while(running){x=x*1.0000001+.0000003;if(x>1000)x=.123;}return NULL;}
static int cpu_times(unsigned long long *idle,unsigned long long *total){
 FILE*f=fopen("/proc/stat","r"); if(!f)return 0;
 unsigned long long u=0,n=0,s=0,i=0,w=0,ir=0,si=0,st=0;
 int r=fscanf(f,"cpu %llu %llu %llu %llu %llu %llu %llu %llu",&u,&n,&s,&i,&w,&ir,&si,&st);fclose(f);
 if(r<4)return 0;
 *idle=i+w;*total=u+n+s+i+w+ir+si+st;return 1;
}
static double cpu(void){unsigned long long i1,t1,i2,t2;if(!cpu_times(&i1,&t1))return 0;usleep(100000);if(!cpu_times(&i2,&t2)||t2==t1)return 0;unsigned long long dt=t2-t1,di=i2-i1;if(di>dt)return 0;return 100.0*(double)(dt-di)/(double)dt;}
static int mem(double*p,unsigned long long*used,unsigned long long*avail){
 struct sysinfo x;if(sysinfo(&x))return 0;unsigned long long unit=x.mem_unit?x.mem_unit:1;
 unsigned long long total=(unsigned long long)x.totalram*unit, freeb=((unsigned long long)x.freeram+(unsigned long long)x.bufferram)*unit;
 if(freeb>total)freeb=total;*used=(total-freeb)/(1024ULL*1024);*avail=freeb/(1024ULL*1024);*p=total?100.0*(double)(total-freeb)/(double)total:0;return 1;
}
int main(int argc,char**argv){
 const char*csv="os_stresslab_results.csv";
 for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--workers")&&i+1<argc)workers=atoi(argv[++i]);else if(!strcmp(argv[i],"--memory")&&i+1<argc)memmb=atoi(argv[++i]);else if(!strcmp(argv[i],"--duration")&&i+1<argc)duration=atoi(argv[++i]);else if(!strcmp(argv[i],"--csv")&&i+1<argc)csv=argv[++i];}
 if(workers<1||workers>MAXW||memmb<0||memmb>512||duration<5||duration>60){fprintf(stderr,"Invalid settings\n");return 2;}
 signal(SIGTERM,stop_handler);signal(SIGINT,stop_handler);
 if(memmb){size_t bytes=(size_t)memmb*1024*1024;block=malloc(bytes);if(!block){fprintf(stderr,"Memory allocation failed\n");return 3;}for(size_t i=0;i<bytes;i+=4096)block[i]=(unsigned char)i;}
 FILE*f=fopen(csv,"w");if(!f){perror("CSV");free(block);return 4;}fprintf(f,"elapsed_seconds,cpu_percent,ram_percent,used_ram_mb,available_ram_mb\n");fflush(f);
 for(long i=0;i<workers;i++)if(pthread_create(&th[i],NULL,work,(void*)i)){running=0;for(long j=0;j<i;j++)pthread_join(th[j],NULL);fclose(f);free(block);return 5;}
 time_t start=time(NULL);while(running&&difftime(time(NULL),start)<duration){double cp=cpu(),rp=0;unsigned long long used=0,avail=0;mem(&rp,&used,&avail);fprintf(f,"%d,%.2f,%.2f,%llu,%llu\n",(int)difftime(time(NULL),start),cp,rp,used,avail);fflush(f);for(int i=0;i<10&&running;i++)usleep(100000);}
 running=0;for(int i=0;i<workers;i++)pthread_join(th[i],NULL);free(block);fclose(f);return 0;
}
