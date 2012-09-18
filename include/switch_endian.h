

/*
 *   Set to match the target endianness of the target platform
 */

inline u32 SwitchEndianU32(u32 x)
{
	volatile u32 rval;
	u8  *tmpPtr;
	rval = x;
	tmpPtr = (u8 *)(&rval);
	tmpPtr[0] ^= tmpPtr[3];
	tmpPtr[3] ^= tmpPtr[0];
	tmpPtr[0] ^= tmpPtr[3];
	tmpPtr[1] ^= tmpPtr[2];
	tmpPtr[2] ^= tmpPtr[1];
	tmpPtr[1] ^= tmpPtr[2];
	return rval;
}

inline u16 SwitchEndianU16(u16 x)
{
	volatile u16 rval;
	u8  *tmpPtr;

	rval = x;
		tmpPtr = (u8 *)(&rval);
    
		tmpPtr[0] ^= tmpPtr[1];
		tmpPtr[1] ^= tmpPtr[0];
		tmpPtr[0] ^= tmpPtr[1];

		return rval;
}
