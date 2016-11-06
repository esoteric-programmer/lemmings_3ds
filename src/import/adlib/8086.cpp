/**
 * Incomplete 8086 CPU Emulator
 *
 * Only instructions actual used in ADLIB.DAT of supported lemmings games
 * are implemented. Thus, many opcodes are not implemented.
 *
 * Flag A is not implemented.
 * rol, ror do not set flags (and sar may set wrong flags)
 *
 * However, this is sufficient to grab IO port writes from ADLIB.DAT
 *
 * call_adlib(u16) parses its parameter as follows:
 * 0x0000: update (should be called about every 10 to 15ms to play sound/music)
 * 0x0100: initialize (send adlib initialization sequence to IO port)
 * 0x0200: stop all music and sound
 * 0x0300+i: start playback of music i in an infinite loop
 *           (i from 0x01 to some value depending on current adlib.dat)
 * 0x400+i: play sound i
 *          (i from 0x01 to 0x11)
 */

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "8086.h"

#define AX registers.ax
#define AL *((u8*)&(registers.ax))
#define AH *((u8*)&(registers.ax)+1)
#define BX registers.bx
#define BL *((u8*)&(registers.bx))
#define BH *((u8*)&(registers.bx)+1)
#define CX registers.cx
#define CL *((u8*)&(registers.cx))
#define CH *((u8*)&(registers.cx)+1)
#define DX registers.dx
#define DL *((u8*)&(registers.dx))
#define DH *((u8*)&(registers.dx)+1)
#define SP registers.sp
#define BP registers.bp
#define SI registers.si
#define DI registers.di
#define CS registers.cs
#define DS registers.ds
#define SS registers.ss
#define ES registers.es
#define IP registers.ip
#define SR registers.sr

const u8 Emu8086::parity[] = {
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

Emu8086::Emu8086(unsigned long sample_rate) {
	this->module = new Adlib::Module(sample_rate);
	this->memory = 0;
}

Emu8086::~Emu8086() {
	if (this->module) {
		delete this->module;
	}
	free_adlib_data();
}

int Emu8086::load_adlib_data(struct Data* decoded_adlib_dat) {
	size_t adlib_size = 0;
	if (sizeof(struct Registers) != 14*2) {
		return 0; // wrong struct size (recompile!)
	}
	if (!decoded_adlib_dat) {
		return 0;
	}
	adlib_size = decoded_adlib_dat->size;
	if (!adlib_size) {
		return 0; // error
	}
	free_adlib_data();
	memory = (u8*)malloc(adlib_size);
	if (!memory) {
		return 0; // error
	}
	memcpy(memory, decoded_adlib_dat->data, adlib_size);
	if (!call_adlib(0x0100)) {
		free(memory);
		memory = 0;
		return 0; // error
	}
	return 1;
}

void Emu8086::free_adlib_data() {
	if (memory) {
		free(memory);
		memory = 0;
	}
}

int Emu8086::call_adlib(u16 ax) {
	if (!memory) {
		return 0;
	}
	memset(&registers,0,sizeof(struct Registers));
	SP = sizeof(u16)*STACK_SIZE;
/*	BX = 0x8000; // only for debugging
	DX = 0x0388; // only for debugging
	SI = 0x0002; // only for debugging
	DI = 0x0100; // only for debugging
	BP = 0x091C; // only for debugging */
	AX = ax;
	int ret;
	do {
		ret = x86_step();
	} while(ret == 1);
	return (ret==2?1:0);
}


// interesting 8086 command, because IO ports are used for communicaton with adlib sound card
int Emu8086::in_out(u8 direction, u8 width) {
	// communication with ADLIB
	if (direction) {
		// send instruction to adlib emulator!
		if (width) {
			// OUT DX, AX
			if (this->module) {
				this->module->PortWrite(DX, AX, 16);
			}
		}else{
			// OUT DX, AL
			if (this->module) {
				this->module->PortWrite(DX, AL, 8);
			}
		}
		return 1;
	}else{
		// IN: simulate read of adlib
		if (width) {
			if (this->module) {
				AX = this->module->PortRead(DX, 16);
			}else{
				AX = 255;
			}
		}else{
			if (this->module) {
				AL = this->module->PortRead(DX, 8);
			} else {
				AL = 255;
			}
		}
		return 1;
	}
	return 0;
}


inline void Emu8086::parse_mod_rm(void** other, u8 mod_rm, u8 width) {
	u8 mod = mod_rm>>6;
	u8 rm = mod_rm&0x7;
	s16 displacement;

	switch (rm) {
		case 0:
			displacement = (s16)BX + (s16)SI;
			break;
		case 1:
			displacement = (s16)BX + (s16)DI;
			break;
		case 2:
			displacement = (s16)BP + (s16)SI;
			break;
		case 3:
			displacement = (s16)BP + (s16)DI;
			break;
		case 4:
			displacement = (s16)SI;
			break;
		case 5:
			displacement = (s16)DI;
			break;
		case 6:
			displacement = (s16)BP;
			break;
		case 7:
			displacement = (s16)BX;
			break;
	}

	switch (mod) {
		case 0:
			if (rm == 0x6) {
				*other = memory + *((s16*)(memory+IP));
				IP += 2;
				break;
			}
			*other = memory + displacement;
			break;
		case 1:
			displacement += *((s8*)(memory+IP));
			IP++;
			*other = memory+displacement;
			break;
		case 2:
			displacement += *((s16*)(memory+IP));
			IP+=2;
			*other = memory+displacement;
			break;
		case 3:
			*other = ((u8*)&registers) + ((rm&0x3)<<1) + (((rm&0x4)>>2)<<(width*3));
			break;
	}
}

inline void Emu8086::parse_mod_reg_rm(void** reg, void** other, u8 width) {
	u8 val = memory[IP];
	IP++;
	u8 reg_id = (val>>3)&0x7;
	*reg = ((u8*)&registers) + ((reg_id&0x3)<<1) + (((reg_id&0x4)>>2)<<(width*3));
	parse_mod_rm(other, val, width);
}

inline int Emu8086::push(u16 val) {
	if (SP == 0) {
		return 0; // error
	}
	SP-=sizeof(u16);
	*((u16*)((u8*)stack+SP)) = val;
	return 1;
}

inline int Emu8086::pop(u16* val) {
	if (SP >= 2*STACK_SIZE) {
		return 0; // error
	}
	*val = *((u16*)((u8*)stack+SP));
	SP+=sizeof(u16);
	return 1;
}


inline void Emu8086::add(void* dest, void* source, int width) {
	s16 tmp;
	s16 src;
	if (width) {
		tmp = (s16)(*((u16*)dest));
		src = (s16)(*((u16*)source));
	}else{
		tmp = (s8)(*((u8*)dest));
		src = (s8)(*((u8*)source));
	}

	s32 result = (s32)src + (s32)tmp;
	// clear flags
	SR &= ~(FLAG_C | FLAG_Z | FLAG_S | FLAG_O | FLAG_P | FLAG_A);
	if (width) {
		SR |= ((result&0xFFFF)?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0) | (result&0x8000?FLAG_S:0)
				| ((((result&0x10000)>>1) != (result&0x8000))?FLAG_O:0)
				| (( (u32)(*((u16*)dest)) + (u32)(*((u16*)source)))>0xFFFF?FLAG_C:0);
		// NOTE: flag A is not implemented here
		(*((s16*)dest)) = (s16)result;
	}else{
		SR |= ((result&0xFF)?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0) | (result&0x80?FLAG_S:0)
				| ((((result&0x100)>>1) != (result&0x80))?FLAG_O:0)
				| (( (u32)(*((u8*)dest)) + (u32)(*((u8*)source)))>0xFF?FLAG_C:0);
		// NOTE: flag A is not implemented here
		(*((s8*)dest)) = (s8)result;
	}
}

inline void Emu8086::_and(void* dest, void* source, int width) {
	// clear flags
	SR &= ~(FLAG_C | FLAG_Z | FLAG_S | FLAG_O | FLAG_P);
	// execute operation
	if (width) {
		u16 res = (*((u16*)dest) = (*((u16*)dest) & *((u16*)source)));
		SR |= (res?0:FLAG_Z) | (parity[res&0xFF]?FLAG_P:0) | (res&0x8000?FLAG_S:0);
	}else{
		u8 res = (*((u8*)dest) = (*((u8*)dest) & *((u8*)source)));
		SR |= (res?0:FLAG_Z) | (parity[res&0xFF]?FLAG_P:0) | (res&0x80?FLAG_S:0);
	}
}


inline void Emu8086::_xor(void* dest, void* source, int width) {
	// clear flags
	SR &= ~(FLAG_C | FLAG_Z | FLAG_S | FLAG_O | FLAG_P);
	// execute operation
	if (width) {
		u16 res = (*((u16*)dest) = (*((u16*)dest) ^ *((u16*)source)));
		SR |= (res?0:FLAG_Z) | (parity[res&0xFF]?FLAG_P:0) | (res&0x8000?FLAG_S:0);
	}else{
		u8 res = (*((u8*)dest) = (*((u8*)dest) ^ *((u8*)source)));
		SR |= (res?0:FLAG_Z) | (parity[res&0xFF]?FLAG_P:0) | (res&0x80?FLAG_S:0);
	}
}

inline void Emu8086::_or(void* dest, void* source, int width) {
	// clear flags
	SR &= ~(FLAG_C | FLAG_Z | FLAG_S | FLAG_O | FLAG_P);
	// execute operation
	if (width) {
		u16 res = (*((u16*)dest) = (*((u16*)dest) | *((u16*)source)));
		SR |= (res?0:FLAG_Z) | (parity[res&0xFF]?FLAG_P:0) | (res&0x8000?FLAG_S:0);
	}else{
		u8 res = (*((u8*)dest) = (*((u8*)dest) | *((u8*)source)));
		SR |= (res?0:FLAG_Z) | (parity[res&0xFF]?FLAG_P:0) | (res&0x80?FLAG_S:0);
	}
}


inline void Emu8086::sub(void* dest, void* source, int width) {
	s32 result;
	if (width) {
	 	result = (s32)(*((s16*)dest)) - (s32)(*((s16*)source));
	 }else{
	 	result = (s32)(*((s8*)dest)) - (s32)(*((s8*)source));
	 }
	// clear flags
	SR &= ~(FLAG_C | FLAG_Z | FLAG_S | FLAG_O | FLAG_P | FLAG_A);
	if (width) {
		SR |= ((result&0xFFFF)?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0) | (result&0x8000?FLAG_S:0)
				| ((((result&0x10000)>>1) != (result&0x8000))?FLAG_O:0)
				| ((*((u16*)dest) < *((u16*)source))?FLAG_C:0);
		// NOTE: flag A is not implemented here
		(*((s16*)dest)) = (s16)result;
	}else{
		SR |= ((result&0xFF)?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0) | (result&0x80?FLAG_S:0)
				| ((((result&0x100)>>1) != (result&0x80))?FLAG_O:0)
				| ((*((u8*)dest) < *((u8*)source))?FLAG_C:0);
		// NOTE: flag A is not implemented here
		(*((s8*)dest)) = (s8)result;
	}
}

inline void Emu8086::dec(void* dest, int width) {
	// don't change carry flag
	u16 s = (SR & FLAG_C);
	s16 one = 1;
	if (width) {
	 	sub(dest,&one,width);
	 }else{
	 	sub(dest,&one,width);
	 }
	 SR &= ~FLAG_C;
	 SR |= s;
}

inline void Emu8086::inc(void* dest, int width) {
	// don't change carry flag
	u16 s = (SR & FLAG_C);
	s16 one = 1;
	if (width) {
	 	add(dest,&one,width);
	 }else{
	 	add(dest,&one,width);
	 }
	 SR &= ~FLAG_C;
	 SR |= s;
}

inline void Emu8086::cmp(void* dest, void* source, int width) {
	// only change flags according to sub, don't save result
	if (width) {
		u16 tmp = *((u16*)dest);
		sub(dest,source,width);
		*((u16*)dest) = tmp;
	}else{
		u8 tmp = *((u8*)dest);
		sub(dest,source,width);
		*((u8*)dest) = tmp;
	}
}


int Emu8086::and1(u8 direction, u8 width) {
	void* dest;
	void* source;
	// get operands
	parse_mod_reg_rm(direction?&dest:&source, direction?&source:&dest, width);
	_and(dest,source,width);
	return 1;
}

int Emu8086::xor1(u8 direction, u8 width) {
	void* dest;
	void* source;
	// get operands
	parse_mod_reg_rm(direction?&dest:&source, direction?&source:&dest, width);
	_xor(dest,source,width);
	return 1;
}

int Emu8086::xor2(u8 direction, u8 width) {
	if (direction) {
		return 0; // not implemented
	}

	void* source;
	if (width) {
		source = memory + IP;
		IP+=2;
	}else{
		source = memory + IP;
		IP++;
	}
	void* dest = &registers;

	_xor(dest,source,width);
	return 1;
}

int Emu8086::and2(u8 direction, u8 width) {
	if (direction) {
		return 0; // command DAA not implemented
	}else{
		void* source;
		if (width) {
			source = memory + IP;
			IP+=2;
		}else{
			source = memory + IP;
			IP++;
		}
		void* dest = &registers;
		_and(dest,source,width);
		return 1;
	}
}

int Emu8086::or1(u8 direction, u8 width) {
	void* dest;
	void* source;
	// get operands
	parse_mod_reg_rm(direction?&dest:&source, direction?&source:&dest, width);
	_or(dest,source,width);
	return 1;
}

int Emu8086::cmp1(u8 direction, u8 width) {
	void* dest;
	void* source;
	// get operands
	parse_mod_reg_rm(direction?&dest:&source, direction?&source:&dest, width);
	cmp(dest,source,width);
	return 1;
}

int Emu8086::add1(u8 direction, u8 width) {
	void* dest;
	void* source;
	// get operands
	parse_mod_reg_rm(direction?&dest:&source, direction?&source:&dest, width);
	add(dest,source,width);
	return 1;
}

int Emu8086::cmp2(u8 direction, u8 width) {
	if (direction) {
		return 0; // command AAS not implemented
	}
	// get operands
	void* dest = &registers;
	void* source;
	if (width) {
		source = memory + IP;
		IP+=2;
	}else{
		source = memory + IP;
		IP++;
	}
	cmp(dest,source,width);
	return 1;
}

int Emu8086::mov1(u8 direction, u8 width) {
	void* dest;
	void* source;
	// get operands
	parse_mod_reg_rm(direction?&dest:&source, direction?&source:&dest, width);
	// execute operation
	if (width) {
		*((u16*)dest) = *((u16*)source);
	}else{
		*((u8*)dest) = *((u8*)source);
	}
	return 1;
}

int Emu8086::sbb2(u8 direction, u8 width) {
	if (direction) {
		// PUSH or POP: ds
		if (!width) {
			// PUSH ds
			return push(DS);
		}else{
			// POP ds
			return pop(&DS);
		}
	}else{
		// command SBB not implemented
		return 0;
	}
}

int Emu8086::add2(u8 direction, u8 width) {
	if (direction) {
		// PUSH es or POP es
		if (!width) {
			// PUSH es
			return push(ES);
		}else{
			// POP es
			return pop(&ES);
		}
	}
	// ADD
	void* source;
	if (width) {
		source = memory + IP;
		IP+=2;
	}else{
		source = memory + IP;
		IP++;
	}
	void* dest = &registers;
	add(dest, source, width);
	return 1;
}

int Emu8086::sub2(u8 direction, u8 width) {
	if (direction) {
		return 0; // not implemented
	}
	// SUB
	void* source;
	if (width) {
		source = memory + IP;
		IP+=2;
	}else{
		source = memory + IP;
		IP++;
	}
	void* dest = &registers;
	sub(dest,source,width);
	return 1;
}

int Emu8086::or2(u8 direction, u8 width) {
	if (direction) {
		// PUSH cs
		if (!width) {
			// PUSH cs
			return push(CS);
		}else{
			// INVALID COMMAND!
			return 0;
		}
	}else{
		// command OR not implemented
		return 0;
	}
}

int Emu8086::jz(u8 direction, u8 width) {
	// jz, jnz, jbe, ja
	s8 dest = (s8)(memory[IP]);
	IP++;
	u8 condition = 0;
	if (!direction) {
		// JZ or JNZ
		condition = (SR & FLAG_Z);
	}else {
		// JBE
		condition = (SR & (FLAG_C | FLAG_Z));
	}
	if (width) {
		condition = !condition;
	}
	if (condition) {
		IP += dest;
	}
	return 1;
}

int Emu8086::jo(u8 direction, u8 width) {
	// jo, jno, jb, jnb
	s8 dest = (s8)(memory[IP]);
	IP++;
	u8 condition = 0;
	if (!direction) {
		// JO or JNO
		condition = ((SR & FLAG_O)!=0);
	}else {
		// JB
		condition = ((SR & FLAG_C)!=0);
	}
	if (width) {
		condition = !condition;
	}
	if (condition) {
		IP += dest;
	}
	return 1;
}

int Emu8086::js(u8 direction, u8 width) {
	// js, jns, jpe, jpo
	s8 dest = (s8)(memory[IP]);
	IP++;
	u8 condition = 0;
	if (!direction) {
		// JS or JNS
		condition = ((SR & FLAG_S)!=0);
	}else {
		// JP
		condition = ((SR & FLAG_P)==0);
	}
	if (width) {
		condition = !condition;
	}
	if (condition) {
		IP += dest;
	}
	return 1;
}

int Emu8086::jl(u8 direction, u8 width) {
	// jl, jge, jle, jg
	s8 dest = (s8)(memory[IP]);
	IP++;
	u8 condition = 0;
	if (!direction) {
		// JL or JGE
		condition = ((SR & FLAG_S)!=0);
		if ((SR & FLAG_O)!=0) {
			condition = !condition;
		}
	}else {
		// JLE
		condition = ((SR & FLAG_S)!=0);
		if ((SR & FLAG_O)==1) {
			condition = !condition;
		}
		condition |= ((SR & FLAG_Z)!=0);
	}
	if (width) {
		condition = !condition;
	}
	if (condition) {
		IP += dest;
	}
	return 1;
}

int Emu8086::jmp(u8 direction, u8 width) {
	s16 dest = 0;
	if (direction) {
		// jmp far or short
		if (width) {
			// short
			dest = *((s8*)(memory+IP));
			IP++;
			IP += dest;
			return 1;
		}
		return 0; // not implemented
	}else{
		// jmp or call
		dest = *((s16*)(memory+IP));
		IP += 2;
		if (!width) {
			// call: push IP to stack
			if (!push(IP)) {
				return 0; // error
			}
		}
	}
	IP += dest; // jump
	return 1;
}

int Emu8086::push1(u8 direction, u8 width) {
	return push(*(((u16*)(&registers)) + ((direction << 1) | width)));
}

int Emu8086::push2(u8 direction, u8 width) {
	return push(*(((u16*)(&registers)) + 4 + ((direction << 1) | width)));
}

int Emu8086::pop1(u8 direction, u8 width) {
	return pop(((u16*)(&registers)) + ((direction << 1) | width));
}

int Emu8086::pop2(u8 direction, u8 width) {
	return pop(((u16*)(&registers)) + 4 + ((direction << 1) | width));
}

int Emu8086::ret(u8 direction, u8 width) {
	if (!direction) {
		return 0; // invalid command
	}
	if (!width) {
		return 0; // not implemented
	}
	return pop(&IP);
}

int Emu8086::iret(u8 direction, u8 width) {
	if (direction && width) {
		if (SP == 2*STACK_SIZE) {
			return 2; // exit successful
		}else{
			return 0; // error: adlib.dat should execute IRET command only when stack is cleared
		}
	}else{
		return 0; // not implemented
	}
}

int Emu8086::mov_al(u8 direction, u8 width) {
	void* address = memory + *((s16*)(memory+IP));
	IP += 2;
	void* reg = &AX;

	void* dest;
	void* source;
	// get operands
	if (!direction) {
		dest = reg;
		source = address;
	}else{
		dest = address;
		source = reg;
	}
	if (width) {
		*((u16*)dest) = *((u16*)source);
	}else{
		*((u8*)dest) = *((u8*)source);
	}
	return 1;
}

int Emu8086::mov_l(u8 direction, u8 width) {
	u8 val = memory[IP];
	IP++;
	if (!direction && !width) {
		AL = val;
	}else if (!direction) {
		CL = val;
	}else if (!width) {
		DL = val;
	}else{
		BL = val;
	}
	return 1;
}

int Emu8086::les(u8 direction, u8 width) {
	if (!direction) {
		// LES, LDS
		return 0; // not implemented
	}
	// MOV
	u8 val = memory[IP];
	IP++;
	void* dest;
	parse_mod_rm(&dest, val, width);
	if (width) {
		*((u16*)dest) = *((u16*)(memory+IP));
		IP+=2;
	}else{
		*((u8*)dest) = *((u8*)(memory+IP));
		IP++;
	}
	return 1;
}

int Emu8086::grp1(u8 direction, u8 width) {
	u8 op = (memory[IP]>>3)&0x7;
	void* reg;
	u8 mod_rm = memory[IP];
	IP++;
	parse_mod_rm(&reg, mod_rm, width);
	s16 data;
	if (width && !direction) {
		data = *((s16*)(memory+IP));
		IP += 2;
	}else{
		data = *((s8*)(memory+IP));
		IP ++;
	}

	switch (op) {
		case 0: // ADD
			add(reg, &data, width);
			return 1;
		case 1: // OR
			_or(reg, &data, width);
			return 1;
		case 4: // AND
			_and(reg, &data, width);
			return 1;
		case 5: // SUB
			sub(reg, &data, width);
			return 1;
		case 7: // CMP
			cmp(reg, &data, width);
			return 1;
		default:
			return 0; // not implemented
	}
}


int Emu8086::grp2(u8 direction, u8 width) {
//	DBG_OUT("grp2: \n");
	u8 op = (memory[IP]>>3)&0x7;
	void* reg;
	u8 mod_rm = memory[IP];
	IP++;
	parse_mod_rm(&reg, mod_rm, width);

	u8 param;
	if (direction) {
		param = CL;
	}else{
		param = 1;
	}

	switch (op) {
		case 0:
			// ROL
			// TODO: FLAGS
			if (width) {
				param %= 16;
				u16 tmp = (*((u16*)reg));
				*((u16*)reg) <<= param;
				*((u16*)reg) |= (tmp >> (16-param));
			}else{
				param %= 8;
				u16 tmp = (*((u8*)reg));
				*((u8*)reg) <<= param;
				*((u8*)reg) |= (tmp >> (8-param));
			}
			return 1;
		case 1:
			// ROR
			// TODO: FLAGS
			if (width) {
				param %= 16;
				u16 tmp = (*((u16*)reg));
				*((u16*)reg) >>= param;
				*((u16*)reg) |= (tmp << (16-param));
			}else{
				param %= 8;
				u16 tmp = (*((u8*)reg));
				*((u8*)reg) >>= param;
				*((u8*)reg) |= (tmp << (8-param));
			}
			return 1;
		case 4:
		{
			// SHL
			u16 sign_o;
			u16 sign_n;
			u16 c = 0;
			u16 result = 0;
			if (width) {
				sign_o = *((u16*)reg) & 0x8000;
				if (param > 0) {
					c = (*((u16*)reg) << (param-1))&0x8000;
				}
				*((u16*)reg) <<= param;
				result = *((u16*)reg);
				sign_n = *((u16*)reg) & 0x8000;
			}else{
				if (param > 0) {
					c = (*((u16*)reg) << (param-1))&0x80;
				}
				sign_o = *((u8*)reg) & 0x80;
				*((u8*)reg) <<= param;
				result = *((u16*)reg);
				sign_n = *((u8*)reg) & 0x80;
			}
			SR &= ~(FLAG_O | FLAG_C | FLAG_S | FLAG_Z | FLAG_P);
			SR |= (((sign_o!=sign_n || param!=1)?FLAG_O:0) | (c?FLAG_C:0));
			SR |= ((result?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0));
			if (width) {
				SR |= (result&0x8000?FLAG_S:0);
			}else{
			 SR |= (result&0x80?FLAG_S:0);
			}
			return 1;
		}
		case 5:
		{
			// SHR
			SR &= ~(FLAG_O | FLAG_C | FLAG_S | FLAG_Z | FLAG_P);
			u16 result = 0;
			if (width) {
				SR |= ((param==1 && (*((u16*)reg)&0x8000)!=0)?FLAG_O:0);
				if (param > 0) {
					if ((*((u16*)reg) >> (param-1))&0x01) {
						SR |= FLAG_C;
					}
				}
				*((u16*)reg) >>= param;
				result = *((u16*)reg);
				SR |= (result&0x8000?FLAG_S:0);
			}else{
				SR |= ((param==1 && (*((u8*)reg)&0x80)!=0)?FLAG_O:0);
				if (param > 0) {
					if ((*((u8*)reg) >> (param-1))&0x01) {
						SR |= FLAG_C;
					}
				}
				*((u8*)reg) >>= param;
				result = *((u8*)reg);
				SR |= (result&0x80?FLAG_S:0);
			}
			SR |= ((result?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0));
			return 1;
		}
		case 7:
		{
			// SAR
			SR &= ~(FLAG_O | FLAG_C | FLAG_S | FLAG_Z | FLAG_P);
			u16 result = 0;
			if (width) {
				SR |= ((param==1 && (*((u16*)reg)&0x8000)!=0)?FLAG_O:0);
				if (param > 0) {
					if ((*((u16*)reg) >> (param-1))&0x01) {
						SR |= FLAG_C;
					}
				}
				*((u16*)reg) >>= param;
				*((u16*)reg) |= (((*((u16*)reg)) & 0x4000) << 1);
				result = *((u16*)reg);
				SR |= (result&0x8000?FLAG_S:0);
			}else{
				SR |= ((param==1 && (*((u8*)reg)&0x80)!=0)?FLAG_O:0);
				if (param > 0) {
					if ((*((u8*)reg) >> (param-1))&0x01) {
						SR |= FLAG_C;
					}
				}
				*((u8*)reg) >>= param;
				*((u8*)reg) |= (((*((u8*)reg)) & 0x40) << 1);
				result = *((u8*)reg);
				SR |= (result&0x80?FLAG_S:0);
			}
			SR |= ((result?0:FLAG_Z) | (parity[result&0xFF]?FLAG_P:0));
			return 1;
		}
		default:
			return 0; // not implemented
	}
}


int Emu8086::grp4(u8 direction, u8 width) {
	if (!direction) {
		return 0; // invalid command
	}
	u8 op = (memory[IP]>>3)&0x7;
	void* reg;
	u8 mod_rm = memory[IP];
	IP++;
	parse_mod_rm(&reg, mod_rm, width);

	switch (op) {
		case 0: // INC
			inc(reg, width);
			return 1;
		case 1: // DEC
			dec(reg, width);
			return 1;
		case 4: // Grp5: JMP
			if (!width) {
				return 0; // invalid command
			}
			IP = *((u16*)reg);
			return 1;
		default:
			return 0; // not implemented
	}
}

int Emu8086::mov_general_purpose(u8 direction, u8 width) {
	s16 val = *((s16*)(memory+IP));
	IP += 2;
	if (!direction && !width) {
		AX = val;
	}else if (!direction) {
		CX = val;
	}else if (!width){
		DX = val;
	}else{
		BX = val;
	}
	return 1;
}

int Emu8086::xchg1(u8 direction, u8 width) {
	if (!direction && !width) {
		// NOP
		return 1;
	}
	return 0; // command XCHG not implemented
}

int Emu8086::mov_special_purpose(u8 direction, u8 width) {
	s16 val = *((s16*)(memory+IP));
	IP += 2;
	if (!direction && !width) {
		SP = val;
	}else if (!direction) {
		BP = val;
	}else if (!width){
		SI = val;
	}else{
		DI = val;
	}
	return 1;
}

int Emu8086::loop(u8 direction, u8 width) {
	s8 dest = (s8)memory[IP];
	IP++;
	if (direction && !width) {
		// loop
		(*((s16*)(&CX)))--;
		if (CX != 0) {
			IP += dest;
		}
		return 1;
	}
	if (direction && width) {
		// jcxz
		if (CX == 0) {
			IP += dest;
		}
		return 1;
	}
	return 0;
}

int Emu8086::inc1(u8 direction, u8 width) {
	u16* reg = (u16*)(&registers) + (direction?2:0) + (width?1:0);
	inc(reg, 1);
	return 1;
}

int Emu8086::inc2(u8 direction, u8 width) {
	u16* reg = (u16*)(&registers) + 4 + (direction?2:0) + (width?1:0);
	inc(reg,width);
	return 1;
}

int Emu8086::lods(u8 direction, u8 width) {
	if (direction) {
		return 0; // not implemented
	}
	if (width) {
		AX = *((u16*)(memory+SI));
		if (SR & FLAG_D) {
			SI-=2;
		}else{
			SI+=2;
		}
	}else{
		AL = *((u8*)(memory+SI));
		if (SR & FLAG_D) {
			SI--;
		}else{
			SI++;
		}
	}
	return 1;
}

int Emu8086::cmc(u8 direction, u8 width) {
	if (!direction && width) {
		// CMC
		u16 cf = (SR & FLAG_C);
		SR &= ~FLAG_C;
		if (!cf) {
			SR |= FLAG_C;
		}
		return 1;
	}
	return 0; // not implemented
}

int Emu8086::lea(u8 direction, u8 width) {
	if (!direction && width) {
		// LEA
		void* reg;
		void* mem;
		// get operands
		parse_mod_reg_rm(&reg, &mem, 1);
		*((u16*)reg) = (u16)((u8*)mem-memory);
		return 1;
	}else{
		return 0; // not implemented
	}
}

// array of function pointers to (partial) implemented opcodes resp. commands
int (Emu8086::*Emu8086::execute_opcode[64])(u8 direction, u8 width) = {
	&Emu8086::add1,&Emu8086::add2,&Emu8086::or1,&Emu8086::or2,
	0,0,0,&Emu8086::sbb2,
	&Emu8086::and1,&Emu8086::and2,0,&Emu8086::sub2,
	&Emu8086::xor1,&Emu8086::xor2,&Emu8086::cmp1,&Emu8086::cmp2,
	&Emu8086::inc1,&Emu8086::inc2,0,0,
	&Emu8086::push1,&Emu8086::push2,&Emu8086::pop1,&Emu8086::pop2,
	0,0,0,0,
	&Emu8086::jo,&Emu8086::jz,&Emu8086::js,&Emu8086::jl,
	&Emu8086::grp1,0,&Emu8086::mov1,&Emu8086::lea,
	&Emu8086::xchg1,0,0,0,
	&Emu8086::mov_al,0,0,&Emu8086::lods,
	&Emu8086::mov_l,0,&Emu8086::mov_general_purpose,&Emu8086::mov_special_purpose,
	&Emu8086::ret,&Emu8086::les,0,&Emu8086::iret,
	&Emu8086::grp2,0,0,0,
	&Emu8086::loop,0,&Emu8086::jmp,&Emu8086::in_out,
	0,&Emu8086::cmc,0,&Emu8086::grp4
};

int Emu8086::x86_step() {
	// memory[IP]: current instruction
	// check for prefix
	switch (memory[IP]) {
		case 0x26:
		case 0x2E:
		case 0x36:
		case 0x3E:
		case 0xF0:
		case 0xF2:
		case 0xF3:
			// handle prefix
			IP++;
			return 0; // not implemented yet
		default:
			break; // no prefix
	}

	u8 opcode     = ((memory[IP]) & 0xFC) >> 2;
	u8 direction  = ((memory[IP]) & 0x02) >> 1;
	u8 width      = (memory[IP]) & 0x01;
	IP++;
	if (execute_opcode[opcode]) {
		return (this->*execute_opcode[opcode])(direction, width);
	}
	return 0; // error: invalid or unimplemented command
}

void Emu8086::query_opl_samples(unsigned long samples, int module) {
	this->module->handler->Generate(samples, (unsigned long)module);
}
