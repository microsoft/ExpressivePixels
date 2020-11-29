// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * Class implementing animation capability
 **/
#pragma once
#include "CDisplayArray.h"
#include "CDisplayTopology.h"

#define ANIMATION_FADE_KERNEL	20

// File fragment token types
enum enumSequenceTokenTypes
{
	SEQUENCETOKEN_PALLETESIZE   = 1,
	SEQUENCETOKEN_PALLETEBYTES  = 2,
	SEQUENCETOKEN_FRAMESBYTELEN = 3,
	SEQUENCETOKEN_FRAMESBYTES   = 4,
	SEQUENCETOKEN_FRAMECOUNT    = 5,
	SEQUENCETOKEN_FRAMERATE     = 6,
	SEQUENCETOKEN_LOOPCOUNT     = 7,
	SEQUENCETOKEN_NAME			= 8,
	SEQUENCETOKEN_NAMELEN		= 9,
	SEQUENCETOKEN_GUID			= 10,
	SEQUENCETOKEN_GUIDLEN		= 11,
	SEQUENCETOKEN_UTCTIMESTAMP  = 12
};




#pragma pack(push)
#pragma pack(1)

// Structure encapsulating a palettecolor entry
typedef struct 
{
	uint8_t r;
	uint8_t g;
	uint8_t b;	
} PALETTE_ENTRY;



// Structure for animaion header
typedef struct
{
	uint16_t		frameCount;	// Count of frames stored in animation
	uint8_t			loopCount;	// Number of times animation loops before completion
	uint8_t			frameRate;	// Render rate of frames
	uint16_t		cbPallete;	// Byte size of palette colors used in animation
	uint32_t		cbFrames;	// Byte size of frames in animation
} EXPRESSIVEPIXEL_SEQUENCE_META;

#pragma pack(pop) 


// Live control instance state of animation
typedef struct 
{
	EXPRESSIVEPIXEL_SEQUENCE_META Meta;	// Animtion header	
	uint16_t		palleteHexLength;	// String HEX length of JSON palette parse
	PALETTE_ENTRY	*pPalette, *pROMPalette, *pRAMPalette;	// Live state of palette processing
	uint32_t		framesHexLength;	// String HEX length of JSON frames parse
	uint8_t			*pActiveFrames, *pROMFrames, *pRAMFrames; // Live state of frames processing
	void			*pFile;				// Reference to read animation from file
	uint32_t		frameBytesStartOffset; // Processing offset into file/memory 
	char			*pszName;			// Name of animation
	uint8_t			nameLen;			// Length of animation name
	char			szID[48];			// Unique GUID of animation
	uint32_t		utcTimeStamp;		// UTC creation time of animation
	
	// Volatile
	bool			runUntilComplete;
} EXPRESSIVEPIXEL_SEQUENCE, *PEXPRESSIVEPIXEL_SEQUENCE;



// Active firmware animation
struct ACTIVE_ANIMATIONSEQUENCE
{
	EXPRESSIVEPIXEL_SEQUENCE Sequence; // Core animation definition structure
	uint32_t				 firstFrameOffset;			// Byte offset to first frame in whole animation reference
	uint32_t				 currentFrameOffset;		// Byte offset to current frame in animation
	uint8_t                  currentRepeatIteration;	// Numer of times animation has repeated in loop sequence
	unsigned long	         frameInterval;				// Milliseconds between frames based on framerate
	unsigned long	         previousFrameRateMillis;	// Previous frame's milliseconds between frames based on framerate
};



// Animation manager class implementation
class CAnimationManager
{
public:
	CAnimationManager(CDisplayArray *pCDisplayArray);

	bool Activate(EXPRESSIVEPIXEL_SEQUENCE *pSequence);
	bool Activate(uint8_t *pAnimationBytes);
	bool CanPlay() { return m_pActiveSequence == NULL || (m_pActiveSequence != NULL && !m_pActiveSequence->Sequence.runUntilComplete); }
	bool IsActiveAnimationInfinite(char *pszID);
	bool Process();
	bool RenderFrameToBytes(ACTIVE_ANIMATIONSEQUENCE *pSequence, uint8_t *pFrameAsBytes);
	void Clear();
	void SetDisableInfiniteLooping(bool on) { m_disableInfiniteLooping = on; }
	void SetRotation(int rotation) { m_nRotation = rotation; }
	void SetTopology(CDisplayTopology *pCDisplayTopology) { m_pCDisplayTopology = pCDisplayTopology; }
	void ShowSingleFrame(uint8_t *pFrame, uint16_t pixelCount);
	void ShowSinglePixel(uint16_t logicalPixelPosition, uint32_t color);
	void Stop();

	static void ExpressivePixelSequenceFree(EXPRESSIVEPIXEL_SEQUENCE *pSequence);

private:
	bool ProcessFrame(ACTIVE_ANIMATIONSEQUENCE *pSequence, uint8_t *pFrameAsBytes = NULL);
	bool ReadAnimationBytes(ACTIVE_ANIMATIONSEQUENCE *pSequence, uint8_t *pBuf, uint16_t cb);
		
	ACTIVE_ANIMATIONSEQUENCE	*m_pActiveSequence;			// Currently running animation
	bool						m_disableInfiniteLooping;	// True to disable infinitely looping animation
	CDisplayArray				*m_pCDisplayArray;			// Reference to display array class
	CDisplayTopology			*m_pCDisplayTopology;		// Reference to display topology class
	int							m_nRotation;				// Rotation of display in 90 degree increments
	uint8_t						m_activeFadeKernel;			// Fade iteration in fade sequence
	unsigned long	            m_activeFadeMillis;			// Fade over X milliseconds when active
	unsigned long	            m_previousFadeMillis;		// Elapsed time tracking for current fade
	unsigned long	            m_activeDelayMillis;		// Sleep over X milliseconds when active
	unsigned long	            m_previousDelayMillis;		// Elapsed time tracking for current delay
};

