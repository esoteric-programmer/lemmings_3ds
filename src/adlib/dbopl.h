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

#include <3ds.h>
#include "adlib.h"

//Use 8 handlers based on a small logarithmic wavetabe and an exponential table for volume
#define WAVE_HANDLER	10
//Use a logarithmic wavetable with an exponential table for volume
#define WAVE_TABLELOG	11
//Use a linear wavetable with a multiply table for volume
#define WAVE_TABLEMUL	12

//Select the type of wave generator routine
#define DBOPL_WAVE WAVE_TABLEMUL

namespace DBOPL {

struct Chip;
struct Operator;
struct Channel;

#if (DBOPL_WAVE == WAVE_HANDLER)
typedef signed long ( DB_FASTCALL *WaveHandler) ( unsigned long i, unsigned long volume );
#endif

typedef signed long ( DBOPL::Operator::*VolumeHandler) ( );
typedef Channel* ( DBOPL::Channel::*SynthHandler) ( Chip* chip, u32 samples, s32* output );

//Different synth modes that can generate blocks of data
typedef enum {
	sm2AM,
	sm2FM,
	sm3AM,
	sm3FM,
	sm4Start,
	sm3FMFM,
	sm3AMFM,
	sm3FMAM,
	sm3AMAM,
	sm6Start,
	sm2Percussion,
	sm3Percussion,
} SynthMode;

//Shifts for the values contained in chandata variable
enum {
	SHIFT_KSLBASE = 16,
	SHIFT_KEYCODE = 24,
};

struct Operator {
public:
	//Masks for operator 20 values
	enum {
		MASK_KSR = 0x10,
		MASK_SUSTAIN = 0x20,
		MASK_VIBRATO = 0x40,
		MASK_TREMOLO = 0x80,
	};

	typedef enum {
		OFF,
		RELEASE,
		SUSTAIN,
		DECAY,
		ATTACK,
	} State;

	VolumeHandler volHandler;

#if (DBOPL_WAVE == WAVE_HANDLER)
	WaveHandler waveHandler;	//Routine that generate a wave
#else
	s16* waveBase;
	u32 waveMask;
	u32 waveStart;
#endif
	u32 waveIndex;			//WAVE_signed long shifted counter of the frequency index
	u32 waveAdd;				//The base frequency without vibrato
	u32 waveCurrent;			//waveAdd + vibratao

	u32 chanData;			//Frequency/octave and derived data coming from whatever channel controls this
	u32 freqMul;				//Scale channel frequency with this, TODO maybe remove?
	u32 vibrato;				//Scaled up vibrato strength
	s32 sustainLevel;		//When stopping at sustain level stop here
	s32 totalLevel;			//totalLevel is added to every generated volume
	u32 currentLevel;		//totalLevel + tremolo
	s32 volume;				//The currently active volume

	u32 attackAdd;			//Timers for the different states of the envelope
	u32 decayAdd;
	u32 releaseAdd;
	u32 rateIndex;			//Current position of the evenlope

	u8 rateZero;				//signed long for the different states of the envelope having no changes
	u8 keyOn;				//Bitmask of different values that can generate keyon
	//Registers, also used to check for changes
	u8 reg20, reg40, reg60, reg80, regE0;
	//Active part of the envelope we're in
	u8 state;
	//0xff when tremolo is enabled
	u8 tremoloMask;
	//Strength of the vibrato
	u8 vibStrength;
	//Keep track of the calculated KSR so we can check for changes
	u8 ksr;
private:
	void SetState( u8 s );
	void UpdateAttack( const Chip* chip );
	void UpdateRelease( const Chip* chip );
	void UpdateDecay( const Chip* chip );
public:
	void UpdateAttenuation();
	void UpdateRates( const Chip* chip );
	void UpdateFrequency( );

	void Write20( const Chip* chip, u8 val );
	void Write40( const Chip* chip, u8 val );
	void Write60( const Chip* chip, u8 val );
	void Write80( const Chip* chip, u8 val );
	void WriteE0( const Chip* chip, u8 val );

	bool Silent() const;
	void Prepare( const Chip* chip );

	void KeyOn( u8 mask);
	void KeyOff( u8 mask);

	template< State state>
	signed long TemplateVolume( );

	s32 RateForward( u32 add );
	unsigned long ForwardWave();
	unsigned long ForwardVolume();

	signed long GetSample( signed long modulation );
	signed long GetWave( unsigned long index, unsigned long vol );
public:
	Operator();
};

struct Channel {
	Operator op[2];
	inline Operator* Op( unsigned long index ) {
		return &( ( this + (index >> 1) )->op[ index & 1 ]);
	}
	SynthHandler synthHandler;
	u32 chanData;		//Frequency/octave and derived values
	s32 old[2];			//Old data for feedback

	u8 feedback;			//Feedback shift
	u8 regB0;			//Register values to check for changes
	u8 regC0;
	//This should correspond with reg104, bit 6 indicates a Percussion channel, bit 7 indicates a silent channel
	u8 fourMask;
	s8 maskLeft;		//Sign extended values for both channel's panning
	s8 maskRight;

	//Forward the channel data to the operators of the channel
	void SetChanData( const Chip* chip, u32 data );
	//Change in the chandata, check for new values and if we have to forward to operators
	void UpdateFrequency( const Chip* chip, u8 fourOp );
	void WriteA0( const Chip* chip, u8 val );
	void WriteB0( const Chip* chip, u8 val );
	void WriteC0( const Chip* chip, u8 val );
	void ResetC0( const Chip* chip );

	//call this for the first channel
	template< bool opl3Mode >
	void GeneratePercussion( Chip* chip, s32* output );

	//Generate blocks of data in specific modes
	template<SynthMode mode>
	Channel* BlockTemplate( Chip* chip, u32 samples, s32* output );
	Channel();
};

struct Chip {
	//This is used as the base counter for vibrato and tremolo
	u32 lfoCounter;
	u32 lfoAdd;


	u32 noiseCounter;
	u32 noiseAdd;
	u32 noiseValue;

	//Frequency scales for the different multiplications
	u32 freqMul[16];
	//Rates for decay and release for rate of this chip
	u32 linearRates[76];
	//Best match attack rates for the rate of this chip
	u32 attackRates[76];

	//18 channels with 2 operators each
	Channel chan[18];

	u8 reg104;
	u8 reg08;
	u8 reg04;
	u8 regBD;
	u8 vibratoIndex;
	u8 tremoloIndex;
	s8 vibratoSign;
	u8 vibratoShift;
	u8 tremoloValue;
	u8 vibratoStrength;
	u8 tremoloStrength;
	//Mask for allowed wave forms
	u8 waveFormMask;
	//0 or -1 when enabled
	s8 opl3Active;

	//Return the maximum amount of samples before and LFO change
	u32 ForwardLFO( u32 samples );
	u32 ForwardNoise();

	void WriteBD( u8 val );
	void WriteReg(u32 reg, u8 val );

	u32 WriteAddr( u32 port, u8 val );

	void GenerateBlock2( unsigned long samples, s32* output );
	void GenerateBlock3( unsigned long samples, s32* output );

	void Generate( u32 samples );
	void Setup( u32 r );

	Chip();
};

struct Handler : public Adlib::Handler {
	DBOPL::Chip chip;
	virtual u32 WriteAddr( u32 port, u8 val );
	virtual void WriteReg( u32 addr, u8 val );
	virtual void Generate( unsigned long samples );
	virtual void Init( unsigned long rate );
};


};		//Namespace
