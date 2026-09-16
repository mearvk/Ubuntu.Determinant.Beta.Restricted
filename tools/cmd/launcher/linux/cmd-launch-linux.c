/* Linux native CMD launcher template.
 * The resulting ELF is used as the executable prefix of a .cmd file.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../format/cmd-format.h"

typedef struct { unsigned int h[8]; unsigned long long bits; unsigned char block[64]; size_t used; } sha256_ctx;
static const unsigned int K[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dcf,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,
  0x4d2c6dfc,0x5b9cca4f,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,
  0xbef9a3f7,0xc67178f2,0xca273ece,0xd186b8c1,0xeada7dd6,0x19a4c116,0x1e376c08,0x2748774c,
  0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,
  0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2,0xca273ece,0xd186b8c1,0xeada7dd6
};
/* The compact SHA-256 implementation below uses the standard constants; duplicate
 * constants above are harmless but are avoided by using the canonical first 64 values. */
static const unsigned int CK[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b69c1,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
#define ROR(x,n) ((x >> n) | (x << (32-n)))
#define CH(x,y,z) ((x&y)^(~x&z))
#define MJ(x,y,z) ((x&y)^(x&z)^(y&z))
#define S0(x) (ROR(x,2)^ROR(x,13)^ROR(x,22))
#define S1(x) (ROR(x,6)^ROR(x,11)^ROR(x,25))
#define s0(x) (ROR(x,7)^ROR(x,18)^(x>>3))
#define s1(x) (ROR(x,17)^ROR(x,19)^(x>>10))
static void sha_init(sha256_ctx *c){ static const unsigned int H[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19}; memcpy(c->h,H,sizeof H); c->bits=0;c->used=0; }
static void sha_block(sha256_ctx *c,const unsigned char *p){ unsigned int w[64],a,b,d,e,f,g,h,x,y; int i; for(i=0;i<16;i++)w[i]=(p[i*4]<<24)|(p[i*4+1]<<16)|(p[i*4+2]<<8)|p[i*4+3]; for(;i<64;i++)w[i]=s1(w[i-2])+w[i-7]+s0(w[i-15])+w[i-16]; a=c->h[0];b=c->h[1];x=c->h[2];d=c->h[3];e=c->h[4];f=c->h[5];g=c->h[6];h=c->h[7]; for(i=0;i<64;i++){unsigned int t1=h+S1(e)+CH(e,f,g)+CK[i]+w[i],t2=S0(a)+MJ(a,b,x);h=g;g=f;f=e;e=d+t1;d=x;x=b;b=a;a=t1+t2;} c->h[0]+=a;c->h[1]+=b;c->h[2]+=x;c->h[3]+=d;c->h[4]+=e;c->h[5]+=f;c->h[6]+=g;c->h[7]+=h; }
static void sha_update(sha256_ctx *c,const unsigned char *p,size_t n){c->bits+=(unsigned long long)n*8;while(n){size_t k=64-c->used;if(k>n)k=n;memcpy(c->block+c->used,p,k);c->used+=k;p+=k;n-=k;if(c->used==64){sha_block(c,c->block);c->used=0;}}}
static void sha_final(sha256_ctx *c,unsigned char out[32]){size_t i=c->used;c->block[i++]=0x80;while(i!=56){if(i==64){sha_block(c,c->block);i=0;}c->block[i++]=0;}for(int j=0;j<8;j++)c->block[63-j]=(unsigned char)(c->bits>>(j*8));sha_block(c,c->block);for(i=0;i<8;i++){out[i*4]=(unsigned char)(c->h[i]>>24);out[i*4+1]=(unsigned char)(c->h[i]>>16);out[i*4+2]=(unsigned char)(c->h[i]>>8);out[i*4+3]=(unsigned char)c->h[i];}}

static int file_size(int fd, off_t *size){struct stat st;if(fstat(fd,&st)<0)return -1;*size=st.st_size;return 0;}
static int read_at(int fd, void *buf, size_t n, off_t off){size_t got=0;while(got<n){ssize_t r=pread(fd,(char*)buf+got,n-got,off+(off_t)got);if(r<=0)return -1;got+=(size_t)r;}return 0;}
static int valid_range(off_t file_size, off_t header_off, uint32_t off, uint32_t size){uint64_t a=(uint64_t)header_off+off,b=a+size;return a>=0&&b>=a&&(uint64_t)b<=(uint64_t)file_size;}
static int find_header(int fd, off_t file_size, off_t *result){unsigned char buf[4096];for(off_t base=0;base<file_size;){size_t n=(size_t)((file_size-base)>=(off_t)sizeof buf?sizeof buf:(size_t)(file_size-base));if(read_at(fd,buf,n,base)<0)return -1;for(size_t i=0;i+CMD_HEADER_SIZE<=n;i++){cmd_header_t h;if(read_at(fd,&h,sizeof h,base+(off_t)i)<0)return -1;if(h.magic==CMD_MAGIC&&h.version==CMD_VERSION&&valid_range(file_size,base+(off_t)i,h.icon_offset,h.icon_size)&&valid_range(file_size,base+(off_t)i,h.manifest_offset,h.manifest_size)&&valid_range(file_size,base+(off_t)i,h.class_offset,h.class_size)&&valid_range(file_size,base+(off_t)i,h.security_offset,h.security_size)){*result=base+(off_t)i;return 0;}}if(n<sizeof buf)break;base+=(off_t)(n-(CMD_HEADER_SIZE-1));}return -1;}
static char *manifest_string(int fd, off_t hoff, const cmd_header_t *h,const char *key){char *m=malloc(h->manifest_size+1);if(!m)return NULL;if(read_at(fd,m,h->manifest_size,hoff+h->manifest_offset)<0){free(m);return NULL;}m[h->manifest_size]=0;char needle[96];snprintf(needle,sizeof needle,"\"%s\"",key);char *p=strstr(m,needle);if(!p){free(m);return NULL;}p=strchr(p,':');if(!p){free(m);return NULL;}p++;while(*p==' '||*p=='\t')p++;if(*p!='\"'){free(m);return NULL;}p++;char *q=p;while(*q&&*q!='\"'){if(*q=='\\'&&q[1])q+=2;else q++;}if(*q!='\"'){free(m);return NULL;}size_t n=(size_t)(q-p);char *v=malloc(n+1);if(!v){free(m);return NULL;}memcpy(v,p,n);v[n]=0;free(m);return v;}
static char *runtime_java(void){const char *v=getenv("SECUREJDK_HOME");if(v&&*v){static char p[PATH_MAX];snprintf(p,sizeof p,"%s/bin/java",v);if(access(p,X_OK)==0)return strdup(p);}v=getenv("JAVA_HOME");if(v&&*v){static char p[PATH_MAX];snprintf(p,sizeof p,"%s/bin/java",v);if(access(p,X_OK)==0)return strdup(p);}const char *candidates[]={"/usr/lib/jvm/securejdk-28/bin/java","/usr/lib/jvm/java-28-openjdk-amd64/bin/java","/usr/lib/jvm/java-28-openjdk/bin/java",NULL};for(int i=0;candidates[i];i++)if(access(candidates[i],X_OK)==0)return strdup(candidates[i]);char *p=NULL;size_t n=0;FILE *fp=popen("command -v java 2>/dev/null","r");if(fp){getline(&p,&n,fp);pclose(fp);if(p){p[strcspn(p,"\r\n")]=0;if(access(p,X_OK)==0)return p;free(p);}}return NULL;}
static int write_payload(int fd,off_t hoff,uint32_t off,uint32_t size,const char *path){int out=open(path,O_WRONLY|O_CREAT|O_TRUNC,0700);if(out<0)return -1;unsigned char buf[65536];uint32_t left=size;off_t pos=hoff+off;while(left){size_t n=left>sizeof buf?sizeof buf:left;if(read_at(fd,buf,n,pos)<0||write(out,buf,n)!=(ssize_t)n){close(out);return -1;}pos+=n;left-=(uint32_t)n;}close(out);return 0;}
static char **split_args(const char *s,int *count){int cap=8,n=0;char **a=calloc((size_t)cap,sizeof *a);if(!a)return NULL;while(s&&*s){while(*s==' '||*s=='\t'||*s=='\n')s++;if(!*s)break;if(n+1>=cap){cap*=2;a=realloc(a,(size_t)cap*sizeof *a);if(!a)return NULL;}char q=0;size_t capv=32,lv=0;char *v=malloc(capv);if(!v)return NULL;while(*s&&(q||(*s!=' '&&*s!='\t'&&*s!='\n'))){if(!q&&(*s=='\''||*s=='\"')){q=*s++;continue;}if(*s=='\\'&&s[1])s++;if(lv+1>=capv){capv*=2;v=realloc(v,capv);}v[lv++]=*s++;}v[lv]=0;a[n++]=v;}a[n]=NULL;*count=n;return a;}
static void free_args(char **a){if(!a)return;for(char **p=a;*p;p++)free(*p);free(a);}

int main(int argc,char **argv){(void)argc;int fd=open("/proc/self/exe",O_RDONLY);if(fd<0){perror("CMD: open self");return 126;}off_t fs,hoff;if(file_size(fd,&fs)<0||find_header(fd,fs,&hoff)<0){fprintf(stderr,"CMD: valid CMD header not found\n");return 126;}cmd_header_t h;if(read_at(fd,&h,sizeof h,hoff)<0){fprintf(stderr,"CMD: truncated header\n");return 126;}unsigned char actual[32];sha256_ctx sc;sha_init(&sc);unsigned char buf[65536];uint32_t left=h.class_size;off_t pos=hoff+h.class_offset;while(left){size_t n=left>sizeof buf?sizeof buf:left;if(read_at(fd,buf,n,pos)<0){fprintf(stderr,"CMD: cannot read class data\n");return 126;}sha_update(&sc,buf,n);pos+=n;left-=(uint32_t)n;}sha_final(&sc,actual);if(memcmp(actual,h.sha256,32)!=0){fprintf(stderr,"CMD: SHA-256 verification failed; refusing to execute\n");return 126;}if(h.jdk_min_version>CMD_MIN_JDK){fprintf(stderr,"CMD: unsupported JDK requirement %u\n",h.jdk_min_version);return 126;}char *main_class=manifest_string(fd,hoff,&h,"main_class");if(!main_class||!strcmp(main_class,"(auto-detect)")){fprintf(stderr,"CMD: manifest does not define main_class\n");return 126;}char *java=runtime_java();if(!java){fprintf(stderr,"CMD: no compatible Java runtime found (SecureJDK 28 preferred)\n");return 126;}char tmp[]="/tmp/cmd-run-XXXXXX";char *dir=mkdtemp(tmp);if(!dir){perror("CMD: mkdtemp");return 126;}char payload[PATH_MAX];snprintf(payload,sizeof payload,"%s/%s",dir,(h.flags&CMD_FLAG_EMBEDDED_JAR)?"application.jar":"Main.class");if(h.flags&CMD_FLAG_EMBEDDED_CLASS){const char *slash=strrchr(main_class,'.');const char *leaf=slash?slash+1:main_class;char rel[PATH_MAX];snprintf(rel,sizeof rel,"%s.class",leaf);snprintf(payload,sizeof payload,"%s/%s",dir,rel);char parent[PATH_MAX];snprintf(parent,sizeof parent,"%s",payload);char *last=strrchr(parent,'/');if(last){*last=0;mkdir(parent,0700);}/* package directories are built below */char pkg[PATH_MAX];snprintf(pkg,sizeof pkg,"%s",main_class);for(char *p=pkg;*p;p++)if(*p=='.')*p='/';char relpath[PATH_MAX];snprintf(relpath,sizeof relpath,"%s.class",pkg);snprintf(payload,sizeof payload,"%s/%s",dir,relpath);char dirs[PATH_MAX];snprintf(dirs,sizeof dirs,"%s",payload);char *slash2=strrchr(dirs,'/');if(slash2){*slash2=0;for(char *p=dirs+strlen(dir)+1;*p;p++)if(*p=='/'){*p=0;mkdir(dirs,0700);*p='/';}mkdir(dirs,0700);}}if(write_payload(fd,hoff,h.class_offset,h.class_size,payload)<0){fprintf(stderr,"CMD: cannot extract embedded payload\n");return 126;}char *jvm=manifest_string(fd,hoff,&h,"jvm_args");int jn=0;char **ja=split_args(jvm?jvm:"",&jn);size_t max=16+(size_t)jn+(size_t)argc;char **av=calloc(max,sizeof *av);if(!av)return 126;size_t ai=0;av[ai++]=java;for(int i=0;i<jn;i++)av[ai++]=ja[i];char cp[PATH_MAX];snprintf(cp,sizeof cp,"%s",(h.flags&CMD_FLAG_EMBEDDED_JAR)?payload:dir);av[ai++]="-cp";av[ai++]=cp;av[ai++]=main_class;for(int i=1;i<argc;i++)av[ai++]=argv[i];av[ai]=NULL;close(fd);execv(java,av);perror("CMD: exec Java");return 127;}
