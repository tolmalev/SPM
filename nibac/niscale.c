/*
 *
 *  scale.c
 *
 *
 */

#include "spm_types.h"

#include "nilib.h"

/* private type definitions */

typedef union {
	u8  b[4];
	f32 f;
}tEepromF32;

typedef struct
{
	u32 order;
	f32 c[4];
}tMode;

typedef struct
{
	f32 gain;
	f32 offset;
}tInterval; 


/* private eeprom map constants */


#define kCalibrationAreaOffset 24

#define kHeaderSize 16

#define kAiModeBlockStart kHeaderSize   
#define kAiModeBlockSize 17
#define kAiMaxNumberOfModes 4
#define kAiModeBlockEnd \
        (kAiModeBlockStart + kAiModeBlockSize*kAiMaxNumberOfModes)

#define kAiIntervalBlockStart kAiModeBlockEnd    
#define kAiIntervalBlockSize 8
#define kAiMaxNumberOfIntervals 7
#define kAiIntervalBlockEnd \
        (kAiIntervalBlockStart + kAiIntervalBlockSize*kAiMaxNumberOfIntervals)

#define kAoModeBlockStart kAiIntervalBlockEnd
#define kAoModeBlockSize 17
#define kAoMaxNumberOfModes 1
#define kAoModeBlockEnd \
        (kAoModeBlockStart + kAoModeBlockSize*kAoMaxNumberOfModes)

#define kAoIntervalBlockStart kAoModeBlockEnd  
#define kAoMaxNumberOfIntervals 4
#define kAoIntervalBlockSize 8
#define kAoIntervalBlockEnd \
        (kAoIntervalBlockStart + kAoIntervalBlockSize*kAoMaxNumberOfIntervals)

#define kAiChannelBlockSize \
        (kAiModeBlockSize*kAiMaxNumberOfModes + \
         kAiIntervalBlockSize*kAiMaxNumberOfIntervals)
#define kAiMaxNumberOfChannels 1
    
#define kAoChannelBlockSize (kAoModeBlockSize*kAoMaxNumberOfModes + \
                             kAoIntervalBlockSize*kAoMaxNumberOfIntervals)
#define kAoMaxNumberOfChannels 4


f32 getF32FromEeprom ( u8 *eepromMemory, u32 offset) {

	tEepromF32 value;
   
#if LittleEndian
   
	value.b[3] = eepromMemory[offset++];
	value.b[2] = eepromMemory[offset++];
	value.b[1] = eepromMemory[offset++];
	value.b[0] = eepromMemory[offset++];
   
#elif BigEndian
   
	value.b[0] = eepromMemory[offset++];
	value.b[1] = eepromMemory[offset++];
	value.b[2] = eepromMemory[offset++];
	value.b[3] = eepromMemory[offset++];
  
#else
#error target endianness not specified
#endif
   
	return value.f;
}


u32 getCalibrationAreaOffset ( u8 *eepromMemory) {

	return (eepromMemory[ kCalibrationAreaOffset ] << 8) |
		eepromMemory[kCalibrationAreaOffset+1];
}

/*
 *
 * aoGetScalingCoefficients --
 *
 * modeIdx
 *    ignored - AO does not use the mode constants
 *
 * intervalIdx
 *    0 -> 10V reference
 *    1 -> 5V reference
 *    2 -> 2V reference
 *    3 -> 1V reference
 *
 * channel
 *   dac number
 *
 */

void ao_get_scaling_coefficients (u8 *eepromMemory,
				  u32 intervalIdx,
				  u32 modeIdx,
				  u32 channel,
				  struct scale_table *scaling) {
	u32 calOffset = 0;
    
	calOffset = getCalibrationAreaOffset (eepromMemory); 
    
	tInterval interval; 
    
	u32 intervalOffset = calOffset + kAoIntervalBlockStart + 
		(intervalIdx*kAoIntervalBlockSize) +
		(channel*kAoChannelBlockSize);
    
	interval.gain   = getF32FromEeprom (eepromMemory, intervalOffset);
	intervalOffset += 4;
	interval.offset = getF32FromEeprom (eepromMemory, intervalOffset);

	scaling->order = 1; 
	scaling->c[0] = interval.offset;
	scaling->c[1] = interval.gain;

	return; 
}

/*
 *
 * aiGetScalingCoefficients --
 *
 * modeIdx
 *    0 -> default
 *
 * intervalIdx
 *    0 -> +/- 10V
 *    1 -> +/- 5V
 *    2 -> +/- 2V
 *    3 -> +/- 1V
 *    4 -> +/- 500mV
 *    5 -> +/- 200mV
 *    6 -> +/- 100mV
 *
 * channel
 *    ignored - all channels use the same ADC
 *
 */

void ai_get_scaling_coefficients ( u8 *eepromMemory, u32 intervalIdx,
				   u32 modeIdx, u32 channel,
				   struct scale_table *scaling) {

	u32 calOffset = 0;
    
	calOffset = getCalibrationAreaOffset (eepromMemory);
    
	u32 modeOffset = calOffset + kAiModeBlockStart +
		(modeIdx*kAiModeBlockSize);
    
	tMode mode;
    
	mode.order = eepromMemory[modeOffset++];
	mode.c[0] = getF32FromEeprom (eepromMemory, modeOffset);
	modeOffset += 4;
	mode.c[1] = getF32FromEeprom (eepromMemory, modeOffset);
	modeOffset += 4;
	mode.c[2] = getF32FromEeprom (eepromMemory, modeOffset);
	modeOffset += 4;
	mode.c[3] = getF32FromEeprom (eepromMemory, modeOffset);

	tInterval interval; 
    
	u32 intervalOffset = calOffset + kAiIntervalBlockStart +
		(intervalIdx*kAiIntervalBlockSize);
    
	interval.gain = getF32FromEeprom (eepromMemory, intervalOffset);
	intervalOffset += 4;
	interval.offset = getF32FromEeprom (eepromMemory, intervalOffset);

	scaling->order = mode.order;
    
	u32 i;
	for ( i=0; i <= mode.order; i++) {
		scaling->c[i] = mode.c[i] * interval.gain;
		if (i == 0)
			scaling->c[i] = scaling->c[i] + interval.offset;
	}
    
	return; 
}

void ai_polynomial_scaler (i32 *raw,
			   f32 *scaled,
			   struct scale_table * scaling) {

	*scaled = scaling->c[scaling->order];

	i32 j;

	for( j = scaling->order-1 ; j >= 0 ; j--) {
		(*scaled) *= (f32)*raw;
		(*scaled) += scaling->c[j];
	} 
    
	return; 
}

void ao_linear_scaler (i32 *raw, f32 *scaled,  struct scale_table * scaling) {
	*raw = (i32)((*scaled) * scaling->c[1] + scaling->c[0]);
  
	return;
}

/*
 *    reverse scaling
 *
 *    adc_elacs takes volt and returns channel number
 *
 *    dac_elacs takes channel number and returns volt
 *
 */

int adc_elacs (float volt, struct scale_table * adc_scale) {
	return (volt - adc_scale->c[0]) / adc_scale->c[1] + 0.5;
}

float dac_elacs (int channel, struct scale_table * dac_scale) {
	return (float) (channel - dac_scale->c[0]) / dac_scale->c[1];
}
