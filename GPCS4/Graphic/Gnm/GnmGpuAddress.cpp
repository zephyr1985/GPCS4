#include "GnmGpuAddress.h"

#include "GnmRegInfo.h"
#include "GnmTexture.h"

#include "GnmGpuAddressTool.h"

#include <algorithm>

LOG_CHANNEL(Graphic.Gnm.GpuAddress);

namespace GpuAddress
{;

constexpr uint32_t g_tileModeDefaultsAsInts[32] = 
{
	0x90800310,
	0x90800B10,
	0x90801310,
	0x90801B10,
	0x90802310,
	0x90800308,
	0x90801318,
	0x90802318,
	0x90000304,
	0x90000308,
	0x92000310,
	0x92000294,
	0x92000318,
	0x90400308,
	0x92400310,
	0x924002B0,
	0x92400294,
	0x92400318,
	0x9240032C,
	0x9100030C,
	0x9100031C,
	0x910002B4,
	0x910002A4,
	0x91000328,
	0x910002BC,
	0x91000320,
	0x910002B8,
	0x90C00308,
	0x92C00310,
	0x92C00294,
	0x92C00318,
	0x00000000,
};

constexpr uint32_t g_macroTileModeDefaultsAsInts[16] = 
{
	0x26E8,
	0x26D4,
	0x21D0,
	0x21D0,
	0x2080,
	0x2040,
	0x1000,
	0x0000,
	0x36EC,
	0x26E8,
	0x21D4,
	0x20D0,
	0x1080,
	0x1040,
	0x0000,
	0x0000,
};
	
bool isMacroTiled(TileMode tileMode)
{
	const uint64_t macroTiledMask = 0 
		| (1 << kTileModeDepth_2dThin_64) 
		| (1 << kTileModeDepth_2dThin_128) 
		| (1 << kTileModeDepth_2dThin_256) 
		| (1 << kTileModeDepth_2dThin_512) 
		| (1 << kTileModeDepth_2dThin_1K) 
		| (0 << kTileModeDepth_1dThin) 
		| (1 << kTileModeDepth_2dThinPrt_256) 
		| (1 << kTileModeDepth_2dThinPrt_1K) 
		| (0 << kTileModeDisplay_LinearAligned) 
		| (0 << kTileModeDisplay_1dThin) 
		| (1 << kTileModeDisplay_2dThin) 
		| (1 << kTileModeDisplay_ThinPrt) 
		| (1 << kTileModeDisplay_2dThinPrt) 
		| (0 << kTileModeThin_1dThin) 
		| (1 << kTileModeThin_2dThin) 
		| (1 << kTileModeThin_3dThin) 
		| (1 << kTileModeThin_ThinPrt) 
		| (1 << kTileModeThin_2dThinPrt) 
		| (1 << kTileModeThin_3dThinPrt) 
		| (0 << kTileModeThick_1dThick) 
		| (1 << kTileModeThick_2dThick) 
		| (1 << kTileModeThick_3dThick) 
		| (1 << kTileModeThick_ThickPrt) 
		| (1 << kTileModeThick_2dThickPrt) 
		| (1 << kTileModeThick_3dThickPrt) 
		| (1 << kTileModeThick_2dXThick) 
		| (1 << kTileModeThick_3dXThick) 
		| (0 << kTileModeDisplay_LinearGeneral);
	return (macroTiledMask & (1ULL << tileMode)) != 0;
}

bool isPartiallyResidentTexture(ArrayMode arrayMode)
{
	const uint64_t prtTiledMask = 0 | (0 << kArrayModeLinearGeneral) | (0 << kArrayModeLinearAligned) | (0 << kArrayMode1dTiledThin) | (0 << kArrayMode1dTiledThick) | (0 << kArrayMode2dTiledThin) | (1 << kArrayModeTiledThinPrt) | (1 << kArrayMode2dTiledThinPrt) | (0 << kArrayMode2dTiledThick) | (0 << kArrayMode2dTiledXThick) | (1 << kArrayModeTiledThickPrt) | (1 << kArrayMode2dTiledThickPrt) | (1 << kArrayMode3dTiledThinPrt) | (0 << kArrayMode3dTiledThin) | (0 << kArrayMode3dTiledThick) | (0 << kArrayMode3dTiledXThick) | (1 << kArrayMode3dTiledThickPrt);
	return (prtTiledMask & (1ULL << arrayMode)) != 0;
}

int32_t getMicroTileMode(MicroTileMode* outMicroTileMode, TileMode tileMode)
{
	*outMicroTileMode = (MicroTileMode)SCE_GNM_GET_FIELD(g_tileModeDefaultsAsInts[tileMode], GB_TILE_MODE0, MICRO_TILE_MODE_NEW);
	return kStatusSuccess;
}

int32_t getArrayMode(ArrayMode* outArrayMode, TileMode tileMode)
{
	*outArrayMode = (ArrayMode)SCE_GNM_GET_FIELD(g_tileModeDefaultsAsInts[tileMode], GB_TILE_MODE0, ARRAY_MODE);
	return kStatusSuccess;
}

int32_t getPipeConfig(PipeConfig* outPipeConfig, TileMode tileMode)
{
	*outPipeConfig = (PipeConfig)SCE_GNM_GET_FIELD(g_tileModeDefaultsAsInts[tileMode], GB_TILE_MODE0, PIPE_CONFIG);
	return kStatusSuccess;
}

int32_t getTileSplit(TileSplit* outTileSplit, TileMode tileMode)
{
	*outTileSplit = (TileSplit)SCE_GNM_GET_FIELD(g_tileModeDefaultsAsInts[tileMode], GB_TILE_MODE0, TILE_SPLIT);
	return kStatusSuccess;
}

int32_t getSampleSplit(SampleSplit* outSampleSplit, TileMode tileMode)
{
	*outSampleSplit = (SampleSplit)SCE_GNM_GET_FIELD(g_tileModeDefaultsAsInts[tileMode], GB_TILE_MODE0, SAMPLE_SPLIT);
	return kStatusSuccess;
}

int32_t getAltPipeConfig(PipeConfig* outAltPipeConfig, TileMode tileMode)
{
	LOG_ASSERT_RETURN((uint32_t)tileMode < 0x20, kStatusInvalidArgument, "tileMode (0x%02X) does not correspond to a valid TileMode enum entry.", tileMode);
	*outAltPipeConfig = (PipeConfig)SCE_GNM_GET_FIELD(g_tileModeDefaultsAsInts[tileMode], GB_TILE_MODE0, ALT_PIPE_CONFIG);
	return kStatusSuccess;
}

int32_t computeSurfaceMacroTileMode(MacroTileMode* outMacroTileMode, TileMode tileMode,
										   uint32_t bitsPerElement, uint32_t numFragmentsPerPixel)
{
	int32_t status = -1;
	do
	{
		ArrayMode arrayMode;
		int32_t status = getArrayMode(&arrayMode, tileMode);
		if (status != kStatusSuccess)
		{
			break;
		}

		MicroTileMode microTileMode;
		SampleSplit sampleSplitHw;
		TileSplit tileSplitHw;
		status = getMicroTileMode(&microTileMode, tileMode);
		status = getSampleSplit(&sampleSplitHw, tileMode);
		status = getTileSplit(&tileSplitHw, tileMode);

		uint32_t tileThickness  = getMicroTileThickness(arrayMode);
		uint32_t tileBytes1x    = bitsPerElement * kMicroTileWidth * kMicroTileHeight * tileThickness / 8;
		uint32_t sampleSplit    = 1 << sampleSplitHw;
		uint32_t colorTileSplit = std::max(256U, sampleSplit * tileBytes1x);
		uint32_t tileSplit      = (microTileMode == kMicroTileModeDepth) ? (64UL << tileSplitHw) : colorTileSplit;
		uint32_t tileSplitC     = std::min(kDramRowSize, tileSplit);
		uint32_t tileBytes      = std::min(tileSplitC, numFragmentsPerPixel * tileBytes1x);
		uint32_t mtmIndex       = fastIntLog2(tileBytes / 64);
		*outMacroTileMode       = (MacroTileMode)(isPartiallyResidentTexture(arrayMode) ? (mtmIndex + 8) : mtmIndex);
	} while (false);
	return status;
}

int32_t computeSurfaceInfo(SurfaceInfo* infoOut, const TilingParameters* tp)
{
	LOG_ASSERT_RETURN(infoOut != NULL, kStatusInvalidArgument, "infoOut must not be NULL.");
	LOG_ASSERT_RETURN(tp != NULL, kStatusInvalidArgument, "tp must not be NULL.");
	LOG_ASSERT_RETURN(tp->m_surfaceFlags.m_volume == 1 || tp->m_linearDepth == 1, kStatusInvalidArgument, "If tp->m_surfaceFlags.m_volume is not set, tp->m_linearDepth must be 1.");
	if (tp->m_mipLevel == 0)
	{
		LOG_ASSERT_RETURN(tp->m_baseTiledPitch == 0 || tp->m_baseTiledPitch >= tp->m_linearWidth, kStatusInvalidArgument,
						  "if infoIn->m_mipLevel is 0, tp->m_baseTiledPitch must be 0 or >= tp->m_linearWidth.");
	}
	else
	{
		LOG_ASSERT_RETURN(tp->m_baseTiledPitch >= tp->m_linearWidth, kStatusInvalidArgument, "tp->m_baseTiledPitch (%u) must be >= tp->m_linearWidth (%u).", tp->m_baseTiledPitch, tp->m_linearWidth);
	}

	uint32_t bitsPerElement = tp->m_bitsPerFragment;
	LOG_ASSERT_RETURN(isPowerOfTwo(bitsPerElement) && bitsPerElement > 0 && bitsPerElement <= 128, kStatusInvalidArgument, "tp->m_bitsPerFragment (%u) must be a power of 2 in the range [1..128].", bitsPerElement);
	TilingParameters tpTemp = {};  // Starts as a copy of surfInfoIn, but may change along the way
	memcpy(&tpTemp, tp, sizeof(tpTemp));
	MicroTileMode microTileMode;
	int32_t status = getMicroTileMode(&microTileMode, tpTemp.m_tileMode);
	if (status != kStatusSuccess)
		return status;
	LOG_ASSERT_RETURN(!(tpTemp.m_numFragmentsPerPixel > 1 && tpTemp.m_mipLevel > 0), kStatusInvalidArgument, "multi-sampled surfaces cannot have mipmaps.");
	LOG_ASSERT_RETURN(tpTemp.m_surfaceFlags.m_depthTarget == 0 || microTileMode == kMicroTileModeDepth,
					  kStatusInvalidArgument, "if tp->m_surfaceFlags.m_depthTarget is 1, tp->m_tileMode must be kTileModeDepth_*dThin.");

	PipeConfig pipeConfig;
	if (tpTemp.m_minGpuMode == kGpuModeNeo)
	{
		status = getAltPipeConfig(&pipeConfig, tpTemp.m_tileMode);
	}
	else
	{
		status = getPipeConfig(&pipeConfig, tpTemp.m_tileMode);
	}
	if (status != kStatusSuccess)
		return status;
	uint32_t numPipes = getPipeCount(pipeConfig);

	uint32_t requestedPitch = tpTemp.m_baseTiledPitch;

	// Preserve some output values before we change them
	infoOut->m_bitsPerElement = static_cast<uint16_t>(tp->m_bitsPerFragment);

	// Mipmap check.
	uint32_t numTexelsPerElement = 1;
	uint32_t expandX             = 1;
	uint32_t expandY             = 1;
	if (tp->m_isBlockCompressed)
	{
		switch (bitsPerElement)
		{
		case 1:  // 1bpp
			expandX             = 8;
			bitsPerElement      = 8;
			numTexelsPerElement = 8;
			if (tpTemp.m_mipLevel == 0)
			{
				// 1-bit mip level 0 must have width divisible by 8.
				tpTemp.m_linearWidth = (tpTemp.m_linearWidth + 7) & ~7;
				requestedPitch       = (requestedPitch + 7) & ~7;
			}
			break;
		case 4:  // BC1, BC4
		case 8:  // other BCn formats
			expandX = 4;
			expandY = 4;
			bitsPerElement *= 16;
			numTexelsPerElement = 16;
			if (tpTemp.m_mipLevel == 0)
			{
				// BCn mip level 0 must have width/height divisible by 4.
				tpTemp.m_linearWidth  = (tpTemp.m_linearWidth + 3) & ~3;
				tpTemp.m_linearHeight = (tpTemp.m_linearHeight + 3) & ~3;
				requestedPitch        = (requestedPitch + 3) & ~3;
			}
			break;
		case 16:  // GB_GR, BG_RG
			// TODO
			break;
		default:
			LOG_ASSERT_RETURN(!tp->m_isBlockCompressed, kStatusInvalidArgument, "Unknown bit depth %u for block-compressed format", bitsPerElement);
			break;
		}
	}
	if (tpTemp.m_mipLevel > 0)
	{
		// mip level pitches should be generated from base level's pitch instead of its width.
		tpTemp.m_linearWidth = std::max(tpTemp.m_baseTiledPitch >> tpTemp.m_mipLevel, 1U);
	}
	tpTemp.m_linearWidth  = std::max((tpTemp.m_linearWidth + expandX - 1U) / expandX, 1U);
	tpTemp.m_linearHeight = std::max((tpTemp.m_linearHeight + expandY - 1U) / expandY, 1U);
	requestedPitch        = std::max((requestedPitch + expandX - 1U) / expandX, 1U);
	if (tp->m_surfaceFlags.m_pow2Pad)
	{
		tpTemp.m_linearWidth  = nextPowerOfTwo(tpTemp.m_linearWidth);
		tpTemp.m_linearHeight = nextPowerOfTwo(tpTemp.m_linearHeight);
		tpTemp.m_linearDepth  = nextPowerOfTwo(tpTemp.m_linearDepth);
		requestedPitch        = nextPowerOfTwo(requestedPitch);
	}

	// R800AddrLib::HwlComputeSurfaceInfo()
	// Compute surface thickness
	ArrayMode arrayMode;
	status = getArrayMode(&arrayMode, tpTemp.m_tileMode);
	if (status != kStatusSuccess)
		return status;
	uint32_t thickness = getMicroTileThickness(arrayMode);

	LOG_ASSERT_RETURN(numTexelsPerElement != 16 || !isLinear(arrayMode), kStatusInvalidArgument,
					  "if numTexelsPerElement is 16, tp->m_tileMode must not be LinearAligned or LinearGeneral!\n");

	// Degrade array mode from thick to thin for compressed/packed surfaces
	if (thickness > 1)
	{
		ArrayMode oldArrayMode = arrayMode;
		if (numTexelsPerElement > 1)  // BCn or 1bpp
		{
			switch (arrayMode)
			{
			case kArrayMode1dTiledThick:
				arrayMode = kArrayMode1dTiledThin;
				break;
			case kArrayMode2dTiledXThick:
			case kArrayMode2dTiledThick:
				arrayMode = kArrayMode2dTiledThin;
				break;
			case kArrayMode3dTiledXThick:
			case kArrayMode3dTiledThick:
				arrayMode = kArrayMode3dTiledThin;
				break;
			case kArrayModeTiledThickPrt:
				arrayMode = kArrayModeTiledThinPrt;
				break;
			case kArrayMode2dTiledThickPrt:
				arrayMode = kArrayMode2dTiledThinPrt;
				break;
			case kArrayMode3dTiledThickPrt:
				arrayMode = kArrayMode3dTiledThinPrt;
				break;
			default:
				break;
			}
			if (oldArrayMode != arrayMode)
			{
				thickness = 1;
			}
		}
	}

	LOG_ASSERT_RETURN(!(bitsPerElement > 64 && thickness == 8), kStatusInvalidArgument, ">64bpp surfaces can not use XThick tile modes.");
	LOG_ASSERT_RETURN(!(tpTemp.m_numFragmentsPerPixel < 1 || tpTemp.m_numFragmentsPerPixel > 16), kStatusInvalidArgument, "tp->m_numFragmentsPerPixel must be in the range [1..16].");

	arrayMode = degradeLargeThickArrayMode(arrayMode, bitsPerElement);
	status    = adjustTileMode(tpTemp.m_minGpuMode, &tpTemp.m_tileMode, tpTemp.m_tileMode, arrayMode);
	thickness = getMicroTileThickness(arrayMode);  // recalculate thickness, in case arrayMode changed

	if (tpTemp.m_minGpuMode == kGpuModeNeo)
	{
		MicroTileMode microTileMode;
		getMicroTileMode(&microTileMode, tpTemp.m_tileMode);
		infoOut->m_isTexCompatible = tpTemp.m_surfaceFlags.m_texCompatible;
		if (microTileMode == kMicroTileModeDepth)
		{
			uint32_t tileSizeBytes = thickness * bitsPerElement * tpTemp.m_numFragmentsPerPixel * kMicroTileWidth * kMicroTileHeight / 8;
			if (kDramRowSize < tileSizeBytes)
			{
				infoOut->m_isTexCompatible = false;
			}
		}
		else
		{
			if (isMacroTiled(arrayMode))
			{
				SampleSplit sampleSplitHw = kSampleSplit1;
				getSampleSplit(&sampleSplitHw, tpTemp.m_tileMode);
				uint32_t sampleSplit    = 1 << sampleSplitHw;
				uint32_t tileBytes1x    = bitsPerElement * kMicroTileWidth * kMicroTileHeight * thickness / 8;
				uint32_t colorTileSplit = std::max(256U, sampleSplit * tileBytes1x);
				if (kDramRowSize < colorTileSplit)
				{
					infoOut->m_isTexCompatible = false;
				}
			}
			else
			{
				infoOut->m_isTexCompatible = false;
			}
		}
	}

	uint32_t numDimensionsToPad = 0;  // ???
	if (tpTemp.m_surfaceFlags.m_cube)
	{
		if (tpTemp.m_mipLevel == 0)
			numDimensionsToPad = 2;
	}

	switch (arrayMode)
	{
	case kArrayMode1dTiledThin:
	case kArrayMode1dTiledThick:
	{
		// skip this check when jumping here from other code
		LOG_ASSERT_RETURN(tpTemp.m_numFragmentsPerPixel == 1, kStatusInvalidArgument, "numFragmentsPerPixel (%u) must be 1 for 1D-tiled surfaces surfaces.", tpTemp.m_numFragmentsPerPixel);

		thickness = getMicroTileThickness(arrayMode);
		// MSAA surfaces must be 2D tiled

		// Compute alignments
		infoOut->m_baseAlign  = kPipeInterleaveBytes;
		infoOut->m_pitchAlign = 8;
		//if (tpTemp.m_surfaceFlags.m_display && tpTemp.m_mipLevel == 0 /*|| infoTemp.m_flags.m_overlay*/)
		//{
		//	// Display engine hardwires low 5 bits of GRPH_PITCH to zero, which forces
		//	// 32-pixel alignment for displayable surfaces.
		//	infoOut->m_pitchAlign = (infoOut->m_pitchAlign + 31) & ~31;
		//}
		infoOut->m_heightAlign = 8;  // MicroTileHeight
		infoOut->m_depthAlign  = thickness;

		uint32_t outPitch  = (tpTemp.m_mipLevel == 0 && tpTemp.m_baseTiledPitch > 0) ? requestedPitch : tpTemp.m_linearWidth;
		uint32_t outHeight = tpTemp.m_linearHeight;
		uint32_t outDepth  = tpTemp.m_linearDepth;
		// Pad dimensions -- AddrLib::PadDimensions()
		if (tpTemp.m_mipLevel > 0 && tpTemp.m_surfaceFlags.m_cube)
		{
			numDimensionsToPad = (tpTemp.m_linearDepth > 1) ? 3 : 2;  // pad cubemap sub-levels when we treat it as a 3d texture
		}
		if (numDimensionsToPad == 0)
			numDimensionsToPad = 3;
		LOG_ASSERT_RETURN(isPowerOfTwo(infoOut->m_pitchAlign), kStatusInternalTilingError, "pitchAlign (%u) must be a power of two.", infoOut->m_pitchAlign);
		outPitch = (outPitch + infoOut->m_pitchAlign - 1) & ~(infoOut->m_pitchAlign - 1);
		if (numDimensionsToPad > 1)
		{
			LOG_ASSERT_RETURN(isPowerOfTwo(infoOut->m_heightAlign), kStatusInternalTilingError, "heightAlign (%u) must be a power of two.", infoOut->m_heightAlign);
			outHeight = (outHeight + infoOut->m_heightAlign - 1) & ~(infoOut->m_heightAlign - 1);
		}
		if (numDimensionsToPad > 2 || thickness > 1)
		{
			// For cubemap single-face, we do not pad slices.
			// If we pad it, the slice number should be set to 6 and current mip level > 1
			if (tpTemp.m_surfaceFlags.m_cube)  // && (1 || surfInfoTemp.flags.cubeAsArray))
				outDepth = nextPowerOfTwo(outDepth);
			// Normal 3D textures or 2D arrays or cubemaps have a thick mode?
			if (thickness > 1)
			{
				LOG_ASSERT_RETURN(isPowerOfTwo(infoOut->m_depthAlign), kStatusInternalTilingError, "depthAlign (%u) must be a power of two.", infoOut->m_depthAlign);
				outDepth = (outDepth + infoOut->m_depthAlign - 1) & ~(infoOut->m_depthAlign - 1);
			}
		}
		// DCC-compatible MSAA surfaces require additional pitch padding.
		const bool dccCompatible = (microTileMode != kMicroTileModeDepth) && (tpTemp.m_minGpuMode >= kGpuModeNeo);
		if (dccCompatible && tpTemp.m_numFragmentsPerPixel > 1 && tpTemp.m_mipLevel == 0 && isMacroTiled(arrayMode))
		{
			SampleSplit sampleSplitHW;
			TileSplit tileSplitHW;
			GpuAddress::getSampleSplit(&sampleSplitHW, tpTemp.m_tileMode);
			GpuAddress::getTileSplit(&tileSplitHW, tpTemp.m_tileMode);
			uint32_t tileBytes1x = (thickness * bitsPerElement * kMicroTileWidth * kMicroTileHeight + 7) / 8;
			uint32_t sampleSplit = 1 << sampleSplitHW;
			uint32_t tileSplit   = (microTileMode == kMicroTileModeDepth)
									 ? (64 << tileSplitHW)                         // depth modes store the tile split bytes directly
									 : std::max(256U, sampleSplit * tileBytes1x);  // other modes store a sample split multiplier
			uint32_t tileSplitBytes = std::min(kDramRowSize, tileSplit);

			uint32_t tileSizePerFragment = (bitsPerElement * kMicroTileWidth * kMicroTileHeight + 7) / 8;
			uint32_t fragmentsPerSplit   = tileSplitBytes / tileSizePerFragment;
			if (fragmentsPerSplit < tpTemp.m_numFragmentsPerPixel)
			{
				uint32_t dccFastClearByteAlign = numPipes * kPipeInterleaveBytes * 256;
				uint32_t bytesPerSplit         = outPitch * outHeight * bitsPerElement * fragmentsPerSplit;
				LOG_ASSERT(isPowerOfTwo(dccFastClearByteAlign), "dccFastClearByteAlign is supposed to be a power of two...");
				if (0 != (bytesPerSplit & (dccFastClearByteAlign - 1)))
				{
					uint32_t dccFastClearPixelAlign = dccFastClearByteAlign / ((bitsPerElement + 7) / 8) / fragmentsPerSplit;
					uint32_t macroTilePixelAlign    = infoOut->m_pitchAlign * infoOut->m_heightAlign;
					if (dccFastClearPixelAlign >= macroTilePixelAlign &&
						(dccFastClearPixelAlign % macroTilePixelAlign) == 0)
					{
						uint32_t dccFastClearPitchAlignInMacroTile = dccFastClearPixelAlign / macroTilePixelAlign;
						uint32_t heightInMacroTile                 = outHeight / infoOut->m_heightAlign;
						while (heightInMacroTile > 1 &&
							   (heightInMacroTile % 2) == 0 &&
							   dccFastClearPitchAlignInMacroTile > 1 &&
							   (dccFastClearPitchAlignInMacroTile % 2) == 0)
						{
							heightInMacroTile >>= 1;
							dccFastClearPitchAlignInMacroTile >>= 1;
						}
						uint32_t dccFastClearPitchAlignInPixels = infoOut->m_pitchAlign * dccFastClearPitchAlignInMacroTile;
						if (isPowerOfTwo(dccFastClearPitchAlignInPixels))
						{
							outPitch = (outPitch + dccFastClearPitchAlignInPixels - 1) & ~(dccFastClearPitchAlignInPixels - 1);
						}
						else
						{
							outPitch += (dccFastClearPitchAlignInPixels - 1);
							outPitch /= dccFastClearPitchAlignInPixels;
							outPitch *= dccFastClearPitchAlignInPixels;
						}
					}
				}
			}
		}

		// hardware-specific pitch adjustment.
		// Pitch alignment is always 8, so if slice size is not padded to base alignment (pipe_interleave_size)
		// we need to increase pitch
		uint32_t tempPitch              = outPitch;
		uint64_t logicalSliceSizeBytes  = (uint64_t(tempPitch) * outHeight * bitsPerElement * tpTemp.m_numFragmentsPerPixel + 7) / 8;
		uint64_t physicalSliceSizeBytes = logicalSliceSizeBytes * thickness;
		while ((physicalSliceSizeBytes % infoOut->m_baseAlign) != 0)
		{
			tempPitch += infoOut->m_pitchAlign;
			logicalSliceSizeBytes  = (tempPitch * outHeight * bitsPerElement * tpTemp.m_numFragmentsPerPixel + 7) / 8;
			physicalSliceSizeBytes = logicalSliceSizeBytes * thickness;
		}
		// Workaround for depth/stencil: use 8bpp to align depth buffer again, since the stencil plane
		// may have a larger pitch if the slice size is smaller than the base alignment
		if (tpTemp.m_surfaceFlags.m_depthTarget /*&& !infoTemp.m_flags.m_noStencil*/)  // assume m_noStencil is set; a little extra padding doesn't hurt.
		{
			uint64_t logicalSliceSizeStencilBytes = tempPitch * outHeight;
			while ((logicalSliceSizeStencilBytes % infoOut->m_baseAlign) != 0)
			{
				tempPitch += infoOut->m_pitchAlign;
				logicalSliceSizeStencilBytes = tempPitch * outHeight;
			}
			if (tempPitch != outPitch)
			{
				logicalSliceSizeBytes = (logicalSliceSizeStencilBytes * bitsPerElement + 7) / 8;
			}
		}
		outPitch = tempPitch;

		// Output!
		infoOut->m_pitch       = outPitch;
		infoOut->m_height      = outHeight;
		infoOut->m_depth       = outDepth;
		infoOut->m_surfaceSize = logicalSliceSizeBytes * outDepth;
		infoOut->m_arrayMode   = arrayMode;
	}
	break;
	case kArrayModeLinearGeneral:
	case kArrayModeLinearAligned:
	case kArrayMode2dTiledThin:
	case kArrayMode2dTiledThick:
	case kArrayMode2dTiledXThick:
	case kArrayMode3dTiledThin:
	case kArrayMode3dTiledThick:
	case kArrayMode3dTiledXThick:
	case kArrayModeTiledThinPrt:
	case kArrayModeTiledThickPrt:
	case kArrayMode2dTiledThinPrt:
	case kArrayMode2dTiledThickPrt:
	case kArrayMode3dTiledThinPrt:
	case kArrayMode3dTiledThickPrt:
	default:
		LOG_ERR("Invalid arrayMode (0x%02X).", arrayMode);
		return kStatusInvalidArgument;
	}

	infoOut->m_pitch *= expandX;
	infoOut->m_height *= expandY;
	infoOut->m_pitchAlign *= expandX;
	infoOut->m_heightAlign *= expandY;
	if (tp->m_surfaceFlags.m_pow2Pad)
	{
		infoOut->m_pitch  = nextPowerOfTwo(infoOut->m_pitch);
		infoOut->m_height = nextPowerOfTwo(infoOut->m_height);
		infoOut->m_depth  = nextPowerOfTwo(infoOut->m_depth);
	}
	status = adjustTileMode(tpTemp.m_minGpuMode, &infoOut->m_tileMode, tp->m_tileMode, infoOut->m_arrayMode);
	if (status != kStatusSuccess)
		return status;

	if (tp->m_mipLevel == 0 && tp->m_baseTiledPitch > 0 && tp->m_baseTiledPitch != infoOut->m_pitch)
		return kStatusInvalidArgument;

	return kStatusSuccess;
}

int32_t adjustTileMode(GpuMode minGpuMode, TileMode* outTileMode, TileMode oldTileMode, ArrayMode newArrayMode)
{
	LOG_ASSERT_RETURN(outTileMode, kStatusInvalidArgument, "outTileMode must not be NULL.");
	
	ArrayMode oldArrayMode;
	int32_t status = GpuAddress::getArrayMode(&oldArrayMode, oldTileMode);
	if (status != kStatusSuccess)
		return status;
	if (newArrayMode == oldArrayMode)
	{
		*outTileMode = oldTileMode;
		return kStatusSuccess;
	}

	MicroTileMode microTileMode;
	status = GpuAddress::getMicroTileMode(&microTileMode, oldTileMode);
	if (status != kStatusSuccess)
		return status;
	switch (microTileMode)
	{
	case kMicroTileModeDepth:
		LOG_ASSERT(newArrayMode == kArrayMode1dTiledThin, "Expected degraded tile mode (%d) to be 1dThin.", newArrayMode);
		*outTileMode = kTileModeDepth_1dThin;
		return kStatusSuccess;
	case kMicroTileModeDisplay:
		if (newArrayMode == kArrayMode1dTiledThin)
			*outTileMode = kTileModeDisplay_1dThin;
		else
			break;
		return kStatusSuccess;
	case kMicroTileModeThick:
		if (newArrayMode == kArrayMode3dTiledThick)
			*outTileMode = kTileModeThick_3dThick;
		else if (newArrayMode == kArrayMode2dTiledThick)
			*outTileMode = kTileModeThick_2dThick;
		else if (newArrayMode == kArrayMode1dTiledThick)
			*outTileMode = kTileModeThick_1dThick;
		else if (newArrayMode == kArrayMode3dTiledThin)
			*outTileMode = kTileModeThin_3dThin;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode3dTiledThinPrt)
			*outTileMode = kTileModeThin_3dThinPrt;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode2dTiledThin)
			*outTileMode = kTileModeThin_2dThin;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode2dTiledThinPrt)
			*outTileMode = kTileModeThin_2dThinPrt;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayModeTiledThinPrt)
			*outTileMode = kTileModeThin_ThinPrt;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode1dTiledThin)
			*outTileMode = kTileModeThin_1dThin;  // thick modes can degrade to thin
		else
			break;
		return kStatusSuccess;
	case kMicroTileModeThin:
		if (newArrayMode == kArrayMode3dTiledThick)
			*outTileMode = kTileModeThick_3dThick;
		else if (newArrayMode == kArrayMode2dTiledThick)
			*outTileMode = kTileModeThick_2dThick;
		else if (newArrayMode == kArrayMode1dTiledThick)
			*outTileMode = kTileModeThick_1dThick;
		else if (newArrayMode == kArrayMode3dTiledThin)
			*outTileMode = kTileModeThin_3dThin;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode3dTiledThinPrt)
			*outTileMode = kTileModeThin_3dThinPrt;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode2dTiledThin)
			*outTileMode = kTileModeThin_2dThin;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode2dTiledThinPrt)
			*outTileMode = kTileModeThin_2dThinPrt;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayModeTiledThinPrt)
			*outTileMode = kTileModeThin_ThinPrt;  // thick modes can degrade to thin
		else if (newArrayMode == kArrayMode2dTiledThin)
			*outTileMode = kTileModeThin_2dThin;
		else if (newArrayMode == kArrayMode1dTiledThin)
			*outTileMode = kTileModeThin_1dThin;
		else
			break;
		return kStatusSuccess;
	case kMicroTileModeRotated:
	default:
		LOG_ERR("Rotated tile modes should not be used.");
		//if      (newArrayMode == kArrayMode1dTiledThin)
		//	return kTileModeRotated_1dThin;
		return kStatusInvalidArgument;
	}
	LOG_ERR("Unhandled case: oldTileMode=%d, newArrayMode=%d\n", oldTileMode, newArrayMode);
	return kStatusInvalidArgument;
}

int32_t getAltNumBanks(NumBanks* outAltNumBanks, TileMode tileMode, uint32_t bitsPerElement, uint32_t numFragmentsPerPixel)
{
	MacroTileMode macroTileMode;
	int32_t status = computeSurfaceMacroTileMode(&macroTileMode, tileMode, bitsPerElement, numFragmentsPerPixel);
	if (status == kStatusSuccess)
	{
		*outAltNumBanks = (NumBanks)SCE_GNM_GET_FIELD(g_macroTileModeDefaultsAsInts[macroTileMode], GB_MACROTILE_MODE0, ALT_NUM_BANKS);
	}
	return status;
}

int32_t getNumBanks(NumBanks* outNumBanks, TileMode tileMode, uint32_t bitsPerElement, uint32_t numFragmentsPerPixel)
{
	MacroTileMode macroTileMode;
	int32_t status = computeSurfaceMacroTileMode(&macroTileMode, tileMode, bitsPerElement, numFragmentsPerPixel);
	if (status == kStatusSuccess)
	{
		*outNumBanks = (NumBanks)SCE_GNM_GET_FIELD(g_macroTileModeDefaultsAsInts[macroTileMode], GB_MACROTILE_MODE0, NUM_BANKS);
	}

	return status;
}

}  // namespace GpuAddress