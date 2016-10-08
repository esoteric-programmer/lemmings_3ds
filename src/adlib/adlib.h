/*
 * OPL implementation from DOSBox. Slightly modified for Lemmings for 3DS.
 */

/*
 *  Copyright (C) 2002-2010  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* $Id: adlib.h,v 1.5 2009-04-28 21:45:43 c2woody Exp $ */

#ifndef DOSBOX_ADLIB_H
#define DOSBOX_ADLIB_H
#include <math.h>
#include <3ds.h>

namespace Adlib {

struct Timer {
	double start;
	double delay;
	bool enabled, overflow, masked;
	u8 counter;
	Timer() {
		masked = false;
		overflow = false;
		enabled = false;
		counter = 0;
		delay = 0;
	}
	//Call update before making any further changes
	void Update( double time ) {
		if ( !enabled || !delay )
			return;
		double deltaStart = time - start;
		//Only set the overflow flag when not masked
		if ( deltaStart >= 0 && !masked ) {
			overflow = 1;
		}
	}
	//On a reset make sure the start is in sync with the next cycle
	void Reset(const double& time ) {
		overflow = false;
		if ( !delay || !enabled )
			return;
		double delta = (time - start);
		double rem = fmod( delta, delay );
		double next = delay - rem;
		start = time + next;
	}
	void Stop( ) {
		enabled = false;
	}
	void Start( const double& time, signed long scale ) {
		//Don't enable again
		if ( enabled ) {
			return;
		}
		enabled = true;
		delay = 0.001 * (256 - counter ) * scale;
		start = time + delay;
	}

};

struct Chip {
	//Last selected register
	Timer timer[2];
	//Check for it being a write to the timer
	bool Write( u32 addr, u8 val );
	//Read the current timer state, will use current double
	u8 Read( );
};

class Handler {
public:
	//Write an address to a chip, returns the address the chip sets
	virtual u32 WriteAddr( u32 port, u8 val ) = 0;
	//Write to a specific register in the chip
	virtual void WriteReg( u32 addr, u8 val ) = 0;
	//Generate a certain amount of samples
	virtual void Generate( unsigned long samples ) = 0;
	//Initialize at a specific sample rate and mode
	virtual void Init( unsigned long rate ) = 0;
	virtual ~Handler() {
	}
};

//The cache for 2 chips or an opl3
typedef u8 RegisterCache[512];

class Module {
//	IO_ReadHandleObject ReadHandler[3];
//	IO_WriteHandleObject WriteHandler[3];

	//Last selected address in the chip for the different modes
	union {
		u32 normal;
		u8 dual[2];
	} reg;
	void CacheWrite( u32 reg, u8 val );
	void DualWrite( u8 index, u8 reg, u8 val );
public:

	Handler* handler;				//Handler that will generate the sound
	RegisterCache cache;
	Chip	chip[2];

	//Handle port writes
	void PortWrite( unsigned long port, unsigned long val, unsigned long iolen );
	unsigned long PortRead( unsigned long port, unsigned long iolen );
	void Init();

	Module(unsigned long sample_rate); // Section* configuration);
	~Module();
};


}		//Adlib namespace

#endif
