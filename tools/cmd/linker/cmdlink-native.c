/* cmdlink-native: authoritative native-prefix CMD linker.
 * The historical tools/cmd/cmdlink.c is retained unchanged. This implementation
 * is used by the native toolchain and keeps payload integrity self-contained.
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../format/cmd-format.h"
#include "../format/cmd-validate.h"

typedef struct { uint32_t h[8]; uint64_t bits; uint8_t b[64]; size_t n; } sha256_ctx;
static const uint32_t K[64] = {
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xbf597fc7,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
#define R(x,n) ((x>>n)|(x<<(32-n)))
#define CH(x,y,z) ((x&y)^(~x&z))
#define MAJ(x,y,z) ((x&y)^(x&z)^(y&z))
#define E0(x) (R(x,2)^R(x,13)^R(x,22))
#define E1(x) (R(x,6)^R(x,11)^R(x,25))
#define G0(x) (R(x,7)^R(x,18)^(x>>3))
#define G1(x) (R(x,17)^R(x,19)^(x>>10))
static void shinit(sha256_ctx*c){static const uint32_t H[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};memcpy(c->h,H,sizeof H);c->bits=0;c->n=0;}
static void shblk(sha256_ctx*c,const uint8_t*p){uint32_t w[64],a,b,d,e,f,g,h,x;for(int i=0;i<16;i++)w[i]=((uint32_t)p[4*i]<<24)|((uint32_t)p[4*i+1]<<16)|((uint32_t)p[4*i+2]<<8)|p[4*i+3];for(int i=16;i<64;i++)w[i]=G1(w[i-2])+w[i-7]+G0(w[i-15])+w[i-16];a=c->h[0];b=c->h[1];x=c->h[2];d=c->h[3];e=c->h[4];f=c->h[5];g=c->h[6];h=c->h[7];for(int i=0;i<64;i++){uint32_t t=h+E1(e)+CH(e,f,g)+K[i]+w[i],u=E0(a)+MAJ(a,b,x);h=g;g=f;f=e;e=d+t;d=x;x=b;b=a;a=t+u;}c->h[0]+=a;c->h[1]+=b;c->h[2]+=x;c->h[3]+=d;c->h[4]+=e;c->h[5]+=f;c->h[6]+=g;c->h[7]+=h;}
static void shupd(sha256_ctx*c,const uint8_t*p,size_t n){c->bits+=(uint64_t)n*8;while(n){size_t k=64-c->n;if(k>n)k=n;memcpy(c->b+c->n,p,k);c->n+=k;p+=k;n-=k;if(c->n==64){shblk(c,c->b);c->n=0;}}}
static void shfin(sha256_ctx*c,uint8_t o[32]){size_t i=c->n;c->b[i++]=0x80;while(i!=56){if(i==64){shblk(c,c->b);i=0;}c->b[i++]=0;}for(int j=0;j<8;j++)c->b[63-j]=(uint8_t)(c->bits>>(j*8));shblk(c,c->b);for(i=0;i<8;i++){o[4*i]=(uint8_t)(c->h[i]>>24);o[4*i+1]=(uint8_t)(c->h[i]>>16);o[4*i+2]=(uint8_t)(c->h[i]>>8);o[4*i+3]=(uint8_t)c->h[i];}}
static uint8_t* readfile(const char*p,size_t*n){FILE*f=fopen(p,"rb");if(!f)return NULL;if(fseek(f,0,SEEK_END)||ftell(f)<0){fclose(f);return NULL;}long z=ftell(f);rewind(f);uint8_t*b=malloc((size_t)z);if(!b){fclose(f);return NULL;}if(fread(b,1,(size_t)z,f)!=(size_t)z){free(b);fclose(f);return NULL;}fclose(f);*n=(size_t)z;return b;}
static int append(FILE*f,const void*p,size_t n){return n && fwrite(p,1,n,f)!=n;}
static const char*base(const char*p){const char*s=strrchr(p,'/');return s?s+1:p;}
static char*jsonq(const char*s){size_t n=2;for(const char*p=s?s:"";*p;p++)n+=(*p=='"'||*p=='\\')?2:1;char*r=malloc(n+1),*q=r;if(!r)return NULL;*q++='"';for(const char*p=s?s:"";*p;p++){if(*p=='"'||*p=='\\')*q++='\\';*q++=*p;}*q++='"';*q=0;return r;}
static void usage(const char*p){fprintf(stderr,"Usage: %s input.class|input.jar -o output.cmd [--main=Class] [--launcher=file] [--icon=file]\n",p);}
int main(int ac,char**av){const char*in=NULL,*out=NULL,*mainc=NULL,*launcher=NULL,*icon=NULL;int jar=0,headless=0,pin=1,neg=0,grain=0,graal=0;for(int i=1;i<ac;i++){const char*a=av[i];if(!strcmp(a,"--headless"))headless=1;else if(!strcmp(a,"--no-pin"))pin=0;else if(!strcmp(a,"--negamane"))neg=1;else if(!strcmp(a,"--graal-hint"))graal=1;else if(!strncmp(a,"--grain=",8))grain=atoi(a+8);else if(!strncmp(a,"--main=",7))mainc=a+7;else if(!strncmp(a,"--launcher=",11))launcher=a+11;else if(!strncmp(a,"--icon=",7))icon=a+7;else if(!strcmp(a,"-o")&&i+1<ac)out=av[++i];else if(a[0]!='-'){in=a;size_t n=strlen(a);jar=n>4&&!strcmp(a+n-4,".jar");}else {usage(av[0]);return 2;}}
if(!in||!out||grain<0||grain>4){usage(av[0]);return 2;}if(jar&&!mainc){fprintf(stderr,"JAR input requires --main=Class\n");return 2;}if(!launcher){launcher=getenv("CMD_LAUNCHER_TEMPLATE");if(!launcher||!*launcher)launcher="launcher/linux/cmd-launch-linux";}
size_t lsz,csz,isz=0;uint8_t*ld=readfile(launcher,&lsz),*cd=readfile(in,&csz),*id=icon?readfile(icon,&isz):NULL;if(!ld||!cd){fprintf(stderr,"cannot read input or launcher\n");free(ld);free(cd);free(id);return 1;}if((jar&&(csz<4||memcmp(cd,"PK\003\004",4)))||(!jar&&(csz<4||cd[0]!=0xca||cd[1]!=0xfe||cd[2]!=0xba||cd[3]!=0xbe))){fprintf(stderr,"invalid class/JAR input\n");free(ld);free(cd);free(id);return 1;}
uint8_t hash[32];sha256_ctx s;shinit(&s);shupd(&s,cd,csz);shfin(&s,hash);char*mq=jsonq(mainc?mainc:"(auto-detect)");if(!mq){free(ld);free(cd);free(id);return 1;}char manifest[4096];int mn=snprintf(manifest,sizeof(manifest,"{\n  \"format\": \"cmd\",\n  \"version\": \"1.0\",\n  \"main_class\": %s,\n  \"jdk_minimum\": 28,\n  \"preferred_runtime\": \"SecureJDK 28\",\n  \"pinnable\": %s,\n  \"headless\": %s,\n  \"negamane\": %s,\n  \"graal_native_hint\": %s,\n  \"source_type\": \"%s\"\n}\n",mq,pin?"true":"false",headless?"true":"false",neg?"true":"false",graal?"true":"false",jar?"jar":"class");free(mq);if(mn<0||(size_t)mn>=sizeof(manifest)){free(ld);free(cd);free(id);return 1;}
char hex[65];for(int i=0;i<32;i++)sprintf(hex+i*2,"%02x",hash[i]);hex[64]=0;char security[2048];int sn=snprintf(security,sizeof(security),"{\n  \"sha256\": \"%s\",\n  \"integrity_verification\": \"required-before-launch\"\n}\n",hex);if(sn<0||(size_t)sn>=sizeof(security)){free(ld);free(cd);free(id);return 1;}
size_t hoff=lsz,ioff=hoff+sizeof(cmd_header_t),moff=ioff+isz,coff=moff+(size_t)mn,soff=coff+csz,total=soff+(size_t)sn;if(total>UINT32_MAX){free(ld);free(cd);free(id);return 1;}cmd_header_t h={0};h.magic=CMD_MAGIC;h.version=CMD_VERSION;h.flags=(jar?CMD_FLAG_EMBEDDED_JAR:CMD_FLAG_EMBEDDED_CLASS)|(pin?CMD_FLAG_PINNABLE:0);if(headless)h.flags|=CMD_FLAG_HEADLESS;if(neg)h.flags|=CMD_FLAG_NEGAMANE;if(grain)h.flags|=CMD_FLAG_GRAIN_AWARE;if(graal)h.flags|=CMD_FLAG_NATIVE_IMAGE;h.icon_offset=(uint32_t)(ioff-hoff);h.icon_size=(uint32_t)isz;h.manifest_offset=(uint32_t)(moff-hoff);h.manifest_size=(uint32_t)mn;h.class_offset=(uint32_t)(coff-hoff);h.class_size=(uint32_t)csz;h.security_offset=(uint32_t)(soff-hoff);h.security_size=(uint32_t)sn;h.jdk_min_version=CMD_MIN_JDK;memcpy(h.sha256,hash,32);
FILE*f=fopen(out,"wb");if(!f){perror(out);free(ld);free(cd);free(id);return 1;}int bad=append(f,ld,lsz)||append(f,&h,sizeof(h))||append(f,id,isz)||append(f,manifest,(size_t)mn)||append(f,cd,csz)||append(f,security,(size_t)sn);if(fclose(f)||bad){fprintf(stderr,"CMD write failed\n");free(ld);free(cd);free(id);return 1;}chmod(out,0755);printf("Created native CMD: %s\nSHA-256: %s\n",out,hex);free(ld);free(cd);free(id);return 0;}
