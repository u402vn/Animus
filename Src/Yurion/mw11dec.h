#ifndef YURION_MW11DEC_H_
#define YURION_MW11DEC_H_

#include "mw11decapi.h"

typedef void TFunctionMwDecInstall(unsigned char *g_buff);
typedef int TFunctionMwDecodeFrameV10Checked(
	const void *const pBufferIn,
	const unsigned int inBufferSizeBytes,
	void *const pBufferOut,
	const int black,
	const int YGain,
	const int CGain,
	const int Cif,
	const int time,
	void *const g_buff);
typedef int TFunctionMwDecV11(unsigned char *pBufferIn, int Size, unsigned char *pBufferOut,
			  int Time, int Deinterlace_en, int Blend, unsigned char *g_buff);

#define MW_DECODER_CONTEXT_SIZE_BYTES (5006288)
enum
{
	MwDecoderContextSizeBytes = MW_DECODER_CONTEXT_SIZE_BYTES,
};

MW11DEC_API TFunctionMwDecInstall _MWDec_Install;
MW11DEC_API TFunctionMwDecodeFrameV10Checked MwDecodeFrameV10Checked;
MW11DEC_API TFunctionMwDecV11 MWDec_v11;

#endif //YURION_MW11DEC_H_

