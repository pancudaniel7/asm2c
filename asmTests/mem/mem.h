// Generated by asm2c https://github.com/frranck/asm2c
#ifndef MEM_H__
#define MEM_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>

#if defined(_WIN32) || defined(__INTEL_COMPILER)
#define INLINE __inline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__>=199901L
#define INLINE inline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

#define realAddress(offset,segment) ((db *)&m+(dd)offset+m.selectors[m.segment])

#define db unsigned char
#define dw unsigned short
#define dd unsigned int

#define VGARAM_SIZE 320*200
#define STACK_SIZE 1024*sizeof(dd)
#define HEAP_SIZE 1024*1024*4
#define NB_SELECTORS 128

#define PUSHAD memcpy (&m.stack[m.stackPointer], &m.eax, sizeof (dd)*8);m.stackPointer+=sizeof(dd)*8
#define POPAD m.stackPointer-=sizeof(dd)*8;memcpy (&m.eax, &m.stack[m.stackPointer], sizeof (dd)*8)

#define PUSH(nbBits,a) memcpy (&m.stack[m.stackPointer], &a, sizeof (a));m.stackPointer+=sizeof(a)
#define POP(nbBits,a) m.stackPointer-=sizeof(a);memcpy (&a, &m.stack[m.stackPointer], sizeof (a))

#define AFFECT_ZF(a) m.ZF=(a==0)
#define AFFECT_CF(a) m.CF=a
#define AFFECT_SF(nbBits, a) m.SF=(a>>(nbBits-1))
#define ISNEGATIVE(nbBits,a) (a & (1 << (nbBits-1)))

// TODO: add missings affected flags on CMP
#define CMP(nbBits,a,nbBitsSrc,b) AFFECT_ZF(a-b);AFFECT_CF(a<b);AFFECT_SF(nbBits,(a-b));
#define OR(nbBits,a,nbBitsSrc,b) a=a|b;AFFECT_ZF(a);AFFECT_SF(nbBits, a)
#define XOR(nbBits,a,nbBitsSrc,b) a=a^b;AFFECT_ZF(a);AFFECT_SF(nbBits, a)
#define AND(nbBits,a,nbBitsSrc,b) a=a&b;AFFECT_ZF(a);AFFECT_SF(nbBits, a)
#define NEG(nbBits,a) AFFECT_CF(a!=0);a=-a;
#define TEST(nbBits,a,nbBitsSrc,b) AFFECT_ZF((a&b));AFFECT_CF(b<a);AFFECT_SF(nbBits, (a&b))

#define SHR(nbBits,a,nbBitsSrc,b) a=a>>b
#define SHL(nbBits,a,nbBitsSrc,b) a=a<<b
#define ROR(nbBits,a,nbBitsSrc,b) a=(a>>b | a<<(nbBits-b))
#define ROL(nbBits,a,nbBitsSrc,b) a=(a<<b | a>>(nbBits-b))

#define READDDp(a) ((dd *) &m.a)
#define READDWp(a) ((dw *) &m.a)
#define READDBp(a) ((db *) &m.a)

#define READDD(a) (*(dd *) &m.a)

#define READDW(a) (*(dw *)(((db *) &m.a)+(m.isLittle?0:(sizeOf ## a==2?0:2))))
#define READDBh(a) (*(((db *) &m.a)+(m.isLittle?1:(sizeOf ## a==2?0:2))))
#define READDBl(a) (*(((db *) &m.a)+(m.isLittle?0:(sizeOf ## a==2?1:3))))

#define READDBhW(a) (*(((db *) &m.a)+(m.isLittle?1:0)))
#define READDBhD(a) (*(((db *) &m.a)+(m.isLittle?1:2)))
#define READDBlW(a) (*(((db *) &m.a)+(m.isLittle?0:1)))
#define READDBlD(a) (*(((db *) &m.a)+(m.isLittle?0:3)))

#define ADD(nbBits,a,nbBitsSrc,b) a=a+b;AFFECT_ZF(a);AFFECT_CF(a<b);AFFECT_SF(nbBits,a);
#define SUB(nbBits,a,nbBitsSrc,b) a=a-b;AFFECT_ZF(a);AFFECT_CF(b<a);AFFECT_SF(nbBits,a);

// TODO: should affects OF, SF, ZF, AF, and PF
#define INC(nbBits,a) a=a+1;AFFECT_ZF(a)
#define DEC(nbBits,a) a=a-1;AFFECT_ZF(a)

#define JE(label) if (m.ZF) GOTOLABEL(label)
#define JZ(label) JE(label)

#define JNE(label) if (!m.ZF) GOTOLABEL(label)
#define JNZ(label) JNE(label)

#define JNB(label) if (!m.CF) GOTOLABEL(label)
#define JAE(label) JNB(label)
#define JNC(label) JNB(label)

#define JB(label) if (m.CF) GOTOLABEL(label)
#define JC(label) JB(label)
#define JNAE(label) JB(label)

#define JA(label) if (!m.CF && !m.ZF) GOTOLABEL(label)
#define JNBE(label) JA(label)

#define JS(label) if (m.SF) GOTOLABEL(label)
#define JNS(label) if (!m.SF) GOTOLABEL(label)

#define JNA(label) if (m.CF || m.ZF) GOTOLABEL(label)
#define JBE(label) JNA(label)

#define MOV(nbBits,dest,nbBitsSrc,src) dest = src
#define MOVZX(nbBits,dest,nbBitsSrc,src) dest = src
#define MOVSX(nbBits,dest,nbBitsSrc,src) if ISNEGATIVE(nbBitsSrc,src) { dest = ((-1 ^ (( 1 <<nbBitsSrc )-1)) | src ); } else { dest = src; }

// LEA - Load Effective Address
#define LEA(nbBits,dest,nbBitsSrc,src) dest = src

// MOVSx (DF FLAG not implemented)
#define MOVS(a,ecx) src=realAddress(m.esi,ds);dest=realAddress(m.edi,es); \
if (labs(((char *)dest)-((char *)src))<=a) { \
    for(i=0;i<ecx;i++) {  \
        src=realAddress(m.esi,ds);dest=realAddress(m.edi,es); \
        memmove(dest,src,a);m.edi+=a;m.esi+=a; } \
} else { \
    memmove(dest,src,a*ecx);m.edi+=a*ecx;m.esi+=a*ecx; \
}

#define MOVSB MOVS(1,1)
#define MOVSW MOVS(2,1)
#define MOVSD MOVS(4,1)

#define REP_MOVS(b) MOVS(b,m.ecx)
#define REP_MOVSB REP_MOVS(1)
#define REP_MOVSW REP_MOVS(2)
#define REP_MOVSD REP_MOVS(4)

#define STOS(a,b) memcpy (realAddress(m.edi,es), ((db *)&m.eax)+b, a);m.edi+=a
#define STOSB STOS(1,(m.isLittle?0:3))
#define STOSW STOS(2,(m.isLittle?0:2))
#define STOSD STOS(4,0)

#define REP_STOSB for (i=0;i<m.ecx;i++) { STOSB; }
#define REP_STOSW for (i=0;i<m.ecx;i++) { STOSW; }
#define REP_STOSD for (i=0;i<m.ecx;i++) { STOSD; }

#define LODS(a,b) memcpy (((db *)&m.eax)+b, realAddress(m.esi,ds), a);m.esi+=a
#define LODSB LODS(1,(m.isLittle?0:3))
#define LODSW LODS(2,(m.isLittle?0:2))
#define LODSD LODS(4,0)

#define REP_LODS(a,b) for (i=0;i<m.ecx;i++) { LODS(a,b); }
#define REP_LODSB REP_LODS(1,(m.isLittle?0:3))
#define REP_LODSW REP_LODS(2,(m.isLittle?0:2))
#define REP_LODSD REP_LODS(4,0)

// JMP - Unconditional Jump
#define JMP(label) GOTOLABEL(label)
#define GOTOLABEL(a) goto a

#ifndef DEBUG
#define R(a) a
#else
#define R(a) printf("l:%d:%s\n",__LINE__,#a);a
#endif

#define LOOP(label) DEC(32,m.ecx);JNZ(label)

#define CLD m.DF=0
#define STD m.DF=1

void stackDump();
void hexDump (void *addr, int len);
void asm2C_INT(int a);
void asm2C_init();
// directjeu nosetjmp,2
// directmenu
#define INT(a) asm2C_INT(a);TESTJUMPTOBACKGROUND

#define TESTJUMPTOBACKGROUND  if (m.jumpToBackGround) CALL(moveToBackGround);

void asm2C_OUT(int16_t address, int data);
#define OUT(a,b) asm2C_OUT(a,b)
int8_t asm2C_IN(int16_t data);
#define IN(a,b) a = asm2C_IN(b);TESTJUMPTOBACKGROUND

#define STI // TODO: STI not implemented
#define PUSHF
#define POPF
#define NOP

#define CALL(label) \
if (setjmp(jmpbuffer) == 0) { \
    PUSH(?,jmpbuffer); \
    JMP(label); \
}

#define RET POP(x,jmpbuffer);longjmp(jmpbuffer, 0);

#ifdef __LIBSDL2__
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#endif

#ifdef __LIBRETRO__
#include "libretro.h"
extern retro_log_printf_t log_cb;
#define log_error(...) log_cb(RETRO_LOG_ERROR,__VA_ARGS__);
#define log_info(...) log_cb(RETRO_LOG_INFO,__VA_ARGS__);
#define log_debug(...) log_cb(RETRO_LOG_DEBUG,__VA_ARGS__);
#else
#define log_error(...) printf(__VA_ARGS__);
#define log_info(...) printf(__VA_ARGS__);
#define log_debug(...) printf(__VA_ARGS__);
#endif



typedef struct __attribute__((packed)) Mem {
db a;
db b;
db c;
db d;
db e;
db f;
db pas_de_mem[70];
db pbs1[43];
db pbs2[56];
db ascii[11];
db dummy1[4096];

dd eax;
dd ebx;
dd ecx;
dd edx;
dd esi;
dd edi;
dd ebp;
dd esp;
dw cs;
dw ds;
dw es;
dw fs;
dw gs;
dw ss;
_Bool CF;
_Bool ZF;
_Bool DF;
_Bool SF;
_Bool isLittle;
_Bool jumpToBackGround;
_Bool executionFinished;
db exitCode;
db vgaPalette[256*3];
int stackPointer;
dd stack[STACK_SIZE];
dd selectorsPointer;
dd selectors[NB_SELECTORS];
dd heapPointer;
db heap[HEAP_SIZE];
db vgaRamPaddingBefore[VGARAM_SIZE];
db vgaRam[VGARAM_SIZE];
db vgaRamPaddingAfter[VGARAM_SIZE];
char *path;
} Memory;
int program();
#define sizeOfeax 4
#define sizeOfebx 4
#define sizeOfecx 4
#define sizeOfedx 4
#define sizeOfesi 4
#define sizeOfedi 4
#define sizeOfebp 4
#define sizeOfesp 4
#define sizeOfcs 2
#define sizeOfds 2
#define sizeOfes 2
#define sizeOffs 2
#define sizeOfgs 2
#define sizeOfss 2
#define sizeOfa  1
#define sizeOfb  1
#define sizeOfc  1
#define sizeOfd  1
#define sizeOfe  1
#define sizeOff  1
#define sizeOfpas_de_mem  1
#define sizeOfpbs1  1
#define sizeOfpbs2  1
#define sizeOfascii  1

#endif


