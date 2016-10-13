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

/* $Id: adlib.cpp,v 1.42 2009-11-03 20:17:42 qbix79 Exp $ */

#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
#include "adlib.h"
#include "import_adlib.h"

//#include "setup.h"
//#include "mapper.h"
//#include "mem.h"
#include "dbopl.h"

#define RAW_SIZE 1024


/*
	Main Adlib implementation

*/

namespace Adlib {

/*
Chip
*/

bool Chip::Write( u32 reg, u8 val ) {
	switch ( reg ) {
	case 0x02:
		timer[0].counter = val;
		return true;
	case 0x03:
		timer[1].counter = val;
		return true;
	case 0x04:
		double time;
		time = 0;
		if ( val & 0x80 ) {
			timer[0].Reset( time );
			timer[1].Reset( time );
		} else {
			timer[0].Update( time );
			timer[1].Update( time );
			if ( val & 0x1 ) {
				timer[0].Start( time, 80 );
			} else {
				timer[0].Stop( );
			}
			timer[0].masked = (val & 0x40) > 0;
			if ( timer[0].masked )
				timer[0].overflow = false;
			if ( val & 0x2 ) {
				timer[1].Start( time, 320 );
			} else {
				timer[1].Stop( );
			}
			timer[1].masked = (val & 0x20) > 0;
			if ( timer[1].masked )
				timer[1].overflow = false;

		}
		return true;
	}
	return false;
}


u8 Chip::Read( ) {
	double time( 0.0 );
	timer[0].Update( time );
	timer[1].Update( time );
	u8 ret = 0;
	//Overflow won't be set if a channel is masked
	if ( timer[0].overflow ) {
		ret |= 0x40;
		ret |= 0x80;
	}
	if ( timer[1].overflow ) {
		ret |= 0x20;
		ret |= 0x80;
	}
	return ret;

}

void Module::CacheWrite( u32 reg, u8 val ) {
	//Store it into the cache
	cache[ reg ] = val;
}

void Module::DualWrite( u8 index, u8 reg, u8 val ) {
	//Make sure you don't use opl3 features
	//Don't allow write to disable opl3
	if ( reg == 5 ) {
		return;
	}
	//Only allow 4 waveforms
	if ( reg >= 0xE0 ) {
		val &= 3;
	}
	//Write to the timer?
	if ( chip[index].Write( reg, val ) )
		return;
	//Enabling panning
	if ( reg >= 0xc0 && reg <=0xc8 ) {
		val &= 0x0f;
		val |= index ? 0xA0 : 0x50;
	}
	u32 fullReg = reg + (index ? 0x100 : 0);
	handler->WriteReg( fullReg, val );
	CacheWrite( fullReg, val );
}


void Module::PortWrite( unsigned long port, unsigned long val, unsigned long iolen ) {
	//Maybe only enable with a keyon?
	if ( port&1 ) {
		if ( !chip[0].Write( reg.normal, val ) ) {
			handler->WriteReg( reg.normal, val );
			CacheWrite( reg.normal, val );
		}
	} else {
		//Ask the handler to write the address
		//Make sure to clip them in the right range
		reg.normal = handler->WriteAddr( port, val ) & 0xff;
	}
}


unsigned long Module::PortRead( unsigned long port, unsigned long iolen ) {
	//We allocated 4 ports, so just return -1 for the higher ones
	if ( !(port & 3 ) ) {
		//Make sure the low bits are 6 on opl2
		return chip[0].Read() | 0x6;
	} else {
		return 0xff;
	}
	return 0;
}


void Module::Init() {
}

}; //namespace



static Adlib::Module* module = 0;

static void OPL_CallBack(unsigned long len) {
	module->handler->Generate( len );
}

static unsigned long OPL_Read(unsigned long port,unsigned long iolen) {
	return module->PortRead( port, iolen );
}

void OPL_Write(unsigned long port,unsigned long val,unsigned long iolen) {
	module->PortWrite( port, val, iolen );
}


namespace Adlib {

Module::Module( unsigned long sample_rate ) { //Section* configuration ) : Module_base(configuration) {
	reg.dual[0] = 0;
	reg.dual[1] = 0;
	reg.normal = 0;
	handler = 0;

	//Section_prop * section=static_cast<Section_prop *>(configuration);
	unsigned long rate = sample_rate; //section->Get_int("oplrate");
	//Make sure we can't select lower than 8000 to prevent fixed point issues
	if ( rate < 8000 )
		rate = 8000;
	//std::string oplemu( section->Get_string( "oplemu" ) );

	handler = new DBOPL::Handler();
	handler->Init( rate );
	Init();
	install_opl_handler(OPL_CallBack);
	install_port_handler(OPL_Read, OPL_Write);
}

Module::~Module() {
	if ( handler ) {
		delete handler;
	}
}

};	//Adlib Namespace


void OPL_Init(unsigned long sample_rate) {
	module = new Adlib::Module( sample_rate);
}

void OPL_ShutDown(){
	delete module;
	module = 0;

}
