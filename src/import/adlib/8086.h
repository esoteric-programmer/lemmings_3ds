#ifndef EMU8086_H
#define EMU8086_H
#include <3ds.h>
#include "decode.h"
#include "adlib.h"

#define FLAG_O (1<<11)
#define FLAG_D (1<<10)
#define FLAG_I (1<< 9)
#define FLAG_T (1<< 8)
#define FLAG_S (1<< 7)
#define FLAG_Z (1<< 6)
#define FLAG_A (1<< 4)
#define FLAG_P (1<< 2)
#define FLAG_C (1<< 1)
#define STACK_SIZE 0x100

class Emu8086 {
private:
	static const u8 parity[];
	struct Registers {
		u16 ax;
		u16 cx;
		u16 dx;
		u16 bx;

		u16 sp; // stack pointer
		u16 bp; // base pointer
		u16 si; // source index
		u16 di; // destination index


		u16 cs; // code segment
		u16 ds; // data segment
		u16 ss; // stack segment
		u16 es; // extra segment

		u16 ip; // instruction pointer
		u16 sr; // status register (flags)
	} registers;
	u8* memory;
	u16 stack[STACK_SIZE];
	Adlib::Module* module;

	int x86_step();
	int in_out(u8 direction, u8 width);
	inline void parse_mod_rm(void** other, u8 mod_rm, u8 width);
	inline void parse_mod_reg_rm(void** reg, void** other, u8 width);
	inline int pop(u16* val);
	inline int push(u16 val);
	inline void add(void* dest, void* source, int width);
	inline void _and(void* dest, void* source, int width);
	inline void _xor(void* dest, void* source, int width);
	inline void _or(void* dest, void* source, int width);
	inline void sub(void* dest, void* source, int width);
	inline void dec(void* dest, int width);
	inline void inc(void* dest, int width);
	inline void cmp(void* dest, void* source, int width);
	int and1(u8 direction, u8 width);
	int xor1(u8 direction, u8 width);
	int xor2(u8 direction, u8 width);
	int and2(u8 direction, u8 width);
	int or1(u8 direction, u8 width);
	int cmp1(u8 direction, u8 width);
	int add1(u8 direction, u8 width);
	int cmp2(u8 direction, u8 width);
	int mov1(u8 direction, u8 width);
	int sbb2(u8 direction, u8 width);
	int add2(u8 direction, u8 width);
	int sub2(u8 direction, u8 width);
	int or2(u8 direction, u8 width);
	int jz(u8 direction, u8 width);
	int jo(u8 direction, u8 width);
	int js(u8 direction, u8 width);
	int jl(u8 direction, u8 width);
	int jmp(u8 direction, u8 width);
	int push1(u8 direction, u8 width);
	int push2(u8 direction, u8 width);
	int pop1(u8 direction, u8 width);
	int pop2(u8 direction, u8 width);
	int ret(u8 direction, u8 width);
	int iret(u8 direction, u8 width);
	int mov_al(u8 direction, u8 width);
	int mov_l(u8 direction, u8 width);
	int les(u8 direction, u8 width);
	int grp1(u8 direction, u8 width);
	int grp2(u8 direction, u8 width);
	int grp4(u8 direction, u8 width);
	int mov_general_purpose(u8 direction, u8 width);
	int xchg1(u8 direction, u8 width);
	int mov_special_purpose(u8 direction, u8 width);
	int loop(u8 direction, u8 width);
	int inc1(u8 direction, u8 width);
	int inc2(u8 direction, u8 width);
	int lods(u8 direction, u8 width);
	int cmc(u8 direction, u8 width);
	int lea(u8 direction, u8 width);
	static int (Emu8086::*execute_opcode[64])(u8 direction, u8 width);
public:
	Emu8086(unsigned long sample_rate);
	int load_adlib_data(struct Data* decoded_adlib_dat);
	int call_adlib(u16 ax);
	void query_opl_samples(unsigned long samples, int module);
	void free_adlib_data();
	~Emu8086();
};
#endif
