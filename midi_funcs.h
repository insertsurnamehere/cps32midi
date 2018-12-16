// MIDI Output Routines
// --------------------
// written by Valley Bell
// to be included as header file

// Note: MidiDelayCallback can be used to inject additional events.
//       IMPORTANT: You must not call any of the Write*Event() functions or
//       WriteMidiDelay() within this function.

#include <stdlib.h>
#include <string.h>

#include "stdtype.h"

typedef struct _midi_track_state
{
	UINT32 trkBase;
	UINT32 curDly;	// delay until next event
	UINT8 midChn;
} MID_TRK_STATE;

typedef struct file_information
{
	UINT32 alloc;	// allocated bytes
	UINT32 pos;		// current file offset
	UINT8* data;	// file data
} FILE_INF;


#ifndef INLINE
#define INLINE static
#endif

static void WriteMidiDelay(FILE_INF* fInf, UINT32* delay);
static void WriteEvent(FILE_INF* fInf, MID_TRK_STATE* MTS, UINT8 evt, UINT8 val1, UINT8 val2);
static void WriteLongEvent(FILE_INF* fInf, MID_TRK_STATE* MTS, UINT8 evt, UINT32 dataLen, const void* data);
static void WriteMetaEvent(FILE_INF* fInf, MID_TRK_STATE* MTS, UINT8 metaType, UINT32 dataLen, const void* data);
static void WriteMidiValue(FILE_INF* fInf, UINT32 value);
static void File_CheckRealloc(FILE_INF* FileInf, UINT32 bytesNeeded);
static void WriteMidiHeader(FILE_INF* fInf, UINT16 format, UINT16 tracks, UINT16 resolution);
static void WriteMidiTrackStart(FILE_INF* fInf, MID_TRK_STATE* MTS);
static void WriteMidiTrackEnd(FILE_INF* fInf, MID_TRK_STATE* MTS);

INLINE void WriteBE32(UINT8* buffer, UINT32 value);
INLINE void WriteBE16(UINT8* buffer, UINT16 value);


// optional callback for injecting raw data before writing delays
// Returning nonzero makes it skip writing the delay.
static UINT8 (*MidiDelayCallback)(FILE_INF* fInf, UINT32* delay) = NULL;

static void WriteMidiDelay(FILE_INF* fInf, UINT32* delay)
{
	if (MidiDelayCallback != NULL && MidiDelayCallback(fInf, delay))
		return;

	WriteMidiValue(fInf, *delay);
	*delay = 0;

	return;
}

static void WriteEvent(FILE_INF* fInf, MID_TRK_STATE* MTS, UINT8 evt, UINT8 val1, UINT8 val2)
{
	WriteMidiDelay(fInf, &MTS->curDly);

	File_CheckRealloc(fInf, 0x03);
	switch(evt & 0xF0)
	{
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xE0:
		fInf->data[fInf->pos + 0x00] = evt | MTS->midChn;
		fInf->data[fInf->pos + 0x01] = val1;
		fInf->data[fInf->pos + 0x02] = val2;
		fInf->pos += 0x03;
		break;
	case 0xC0:
	case 0xD0:
		fInf->data[fInf->pos + 0x00] = evt | MTS->midChn;
		fInf->data[fInf->pos + 0x01] = val1;
		fInf->pos += 0x02;
		break;
	case 0xF0:	// for Meta Event: Track End
		fInf->data[fInf->pos + 0x00] = evt;
		fInf->data[fInf->pos + 0x01] = val1;
		fInf->data[fInf->pos + 0x02] = val2;
		fInf->pos += 0x03;
		break;
	default:
		break;
	}

	return;
}

static void WriteLongEvent(FILE_INF* fInf, MID_TRK_STATE* MTS, UINT8 evt, UINT32 dataLen, const void* data)
{
	WriteMidiDelay(fInf, &MTS->curDly);

	File_CheckRealloc(fInf, 0x01 + 0x04 + dataLen);	// worst case: 4 bytes of data length
	fInf->data[fInf->pos + 0x00] = evt;
	fInf->pos += 0x01;
	WriteMidiValue(fInf, dataLen);
	memcpy(&fInf->data[fInf->pos], data, dataLen);
	fInf->pos += dataLen;

	return;
}

static void WriteMetaEvent(FILE_INF* fInf, MID_TRK_STATE* MTS, UINT8 metaType, UINT32 dataLen, const void* data)
{
	WriteMidiDelay(fInf, &MTS->curDly);

	File_CheckRealloc(fInf, 0x02 + 0x05 + dataLen);	// worst case: 5 bytes of data length
	fInf->data[fInf->pos + 0x00] = 0xFF;
	fInf->data[fInf->pos + 0x01] = metaType;
	fInf->pos += 0x02;
	WriteMidiValue(fInf, dataLen);
	memcpy(&fInf->data[fInf->pos], data, dataLen);
	fInf->pos += dataLen;

	return;
}

static void WriteMidiValue(FILE_INF* fInf, UINT32 value)
{
	UINT8 valSize;
	UINT8* valData;
	UINT32 tempLng;
	UINT32 curPos;

	valSize = 0x00;
	tempLng = value;
	do
	{
		tempLng >>= 7;
		valSize ++;
	} while(tempLng);

	File_CheckRealloc(fInf, valSize);
	valData = &fInf->data[fInf->pos];
	curPos = valSize;
	tempLng = value;
	do
	{
		curPos --;
		valData[curPos] = 0x80 | (tempLng & 0x7F);
		tempLng >>= 7;
	} while(tempLng);
	valData[valSize - 1] &= 0x7F;

	fInf->pos += valSize;

	return;
}

static void File_CheckRealloc(FILE_INF* fInf, UINT32 bytesNeeded)
{
#define REALLOC_STEP	0x8000	// 32 KB block
	UINT32 minPos;

	minPos = fInf->pos + bytesNeeded;
	if (minPos <= fInf->alloc)
		return;

	while(minPos > fInf->alloc)
		fInf->alloc += REALLOC_STEP;
	fInf->data = (UINT8*)realloc(fInf->data, fInf->alloc);

	return;
}

static void WriteMidiHeader(FILE_INF* fInf, UINT16 format, UINT16 tracks, UINT16 resolution)
{
	File_CheckRealloc(fInf, 0x08 + 0x06);

	WriteBE32(&fInf->data[fInf->pos + 0x00], 0x4D546864);	// write 'MThd'
	WriteBE32(&fInf->data[fInf->pos + 0x04], 0x00000006);	// Header Length
	fInf->pos += 0x08;

	WriteBE16(&fInf->data[fInf->pos + 0x00], format);		// MIDI Format (0/1/2)
	WriteBE16(&fInf->data[fInf->pos + 0x02], tracks);		// number of tracks
	WriteBE16(&fInf->data[fInf->pos + 0x04], resolution);	// Ticks per Quarter
	fInf->pos += 0x06;

	return;
}

static void WriteMidiTrackStart(FILE_INF* fInf, MID_TRK_STATE* MTS)
{
	File_CheckRealloc(fInf, 0x08);

	WriteBE32(&fInf->data[fInf->pos + 0x00], 0x4D54726B);	// write 'MTrk'
	WriteBE32(&fInf->data[fInf->pos + 0x04], 0x00000000);	// write dummy length
	fInf->pos += 0x08;

	MTS->trkBase = fInf->pos;
	MTS->curDly = 0;

	return;
}

static void WriteMidiTrackEnd(FILE_INF* fInf, MID_TRK_STATE* MTS)
{
	UINT32 trkLen;

	trkLen = fInf->pos - MTS->trkBase;
	WriteBE32(&fInf->data[MTS->trkBase - 0x04], trkLen);	// write Track Length

	return;
}


INLINE void WriteBE32(UINT8* buffer, UINT32 value)
{
	buffer[0x00] = (value >> 24) & 0xFF;
	buffer[0x01] = (value >> 16) & 0xFF;
	buffer[0x02] = (value >>  8) & 0xFF;
	buffer[0x03] = (value >>  0) & 0xFF;

	return;
}

INLINE void WriteBE16(UINT8* buffer, UINT16 value)
{
	buffer[0x00] = (value >> 8) & 0xFF;
	buffer[0x01] = (value >> 0) & 0xFF;

	return;
}
    const uint16_t vibrato_depth_table[128] = {
    0x0, 0xC, 0xD, 0xE, 0xE, 0xF, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x20, 0x22, 0x24, 0x25,
    0x27, 0x29, 0x2A, 0x2C, 0x2D, 0x2F, 0x31, 0x32, 0x34, 0x38, 0x3B, 0x3E, 0x41,
    0x45, 0x48, 0x4B, 0x4E, 0x52, 0x55, 0x58, 0x5B, 0x5F, 0x62, 0x65, 0x6A, 0x72,
    0x7A, 0x82, 0x8A, 0x92, 0x9A, 0xA2, 0xAA, 0xB2, 0xBA, 0xC2, 0xCA, 0xD2,
    0xDA, 0xE2, 0xEA, 0xF2, 0xFA, 0x104, 0x114, 0x124, 0x134, 0x144, 0x154,
    0x164, 0x174, 0x184, 0x194, 0x1A4, 0x1B4, 0x1C4, 0x1D4, 0x1E4, 0x1F4, 0x208,
    0x228, 0x248, 0x268, 0x288, 0x2A8, 0x2C8, 0x2E8, 0x308, 0x328, 0x348, 0x368,
    0x388, 0x3A8, 0x3C8, 0x3E8, 0x410, 0x450, 0x490, 0x4D0, 0x510, 0x550, 0x590,
    0x5D0, 0x610, 0x650, 0x690, 0x6D0, 0x710, 0x750, 0x790, 0x7D0, 0x828, 0x888,
    0x8E8, 0x948, 0x9A8, 0xA08, 0xA68, 0xAC8, 0xB28, 0xB88, 0xBE8
    };
    const uint16_t lfo_rate_table[128] = {
    0, 0x118, 0x126, 0x133, 0x142, 0x150, 0x15D, 0x16C, 0x179, 0x187, 0x196,
    0x1A3, 0x1B1, 0x1C0, 0x1DB, 0x1F7, 0x213, 0x22F, 0x24B, 0x267, 0x283, 0x29F,
    0x2BB, 0x2D7, 0x2F3, 0x30F, 0x32B, 0x347, 0x363, 0x37F, 0x3B7, 0x3EF, 0x427,
    0x45F, 0x496, 0x4CF, 0x506, 0x53E, 0x576, 0x5AE, 0x5E6, 0x61E, 0x656, 0x68E,
    0x6C6, 0x6FE, 0x76E, 0x7DE, 0x84D, 0x8BD, 0x92D, 0x99D, 0xA0D, 0xA7D,
    0xAEC, 0xB5C, 0xBCC, 0xC3C, 0xCAC, 0xD1C, 0xD8C, 0xDFB, 0xEDB,
    0xFBB, 0x109B, 0x117A, 0x125A, 0x133A, 0x141A, 0x14F9, 0x15D9, 0x16B9,
    0x1799, 0x1878, 0x1958, 0x1A38, 0x1B17, 0x1BF7, 0x1DB6, 0x1F76, 0x2135,
    0x22F5, 0x24B4, 0x2674, 0x2833, 0x29F3, 0x2BB2, 0x2D71, 0x2F31, 0x30F0,
    0x32B0, 0x346F, 0x362F, 0x37EE, 0x3B6D, 0x3EEC, 0x426B, 0x45EA, 0x4969,
    0x4CE8, 0x5066, 0x53E5, 0x5764, 0x5AE3, 0x5E62, 0x61E1, 0x6550, 0x68DE,
    0x6C5E, 0x6FDC, 0x76DA, 0x7DD8, 0x84D6, 0x8BD3, 0x92D1, 0x99CF, 0xA0CD,
    0xA7CB, 0xAEC8, 0xB5C6, 0xBCC4, 0xC3C2, 0xCABF, 0xD1BD, 0xD8BB,
    0xDFB9, 0xEDB4, 0xFBB0
    };