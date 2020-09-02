// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include "EPXPlatform_Runtime.h"
#include "EPXVariant.h"
#include "CAnimationManager.h"
#include "EPXPlatform_CStorage.h"
#include "stdlib.h"
EPX_OPTIMIZEFORDEBUGGING_ON

CAnimationManager::CAnimationManager(CDisplayArray *pCDisplayArray)
{
	m_pCDisplayTopology = NULL;
	m_pCDisplayArray = pCDisplayArray;
	m_pActiveSequence = NULL;
	m_activeDelayMillis = 0;
	m_previousDelayMillis = 0;
	m_activeFadeMillis = 0;
	m_previousFadeMillis = 0;
	m_activeFadeKernel = 0;
	m_nRotation = 0;
	m_disableInfiniteLooping = false;
}



bool CAnimationManager::Activate(uint8_t *pAnimationBytes)
{
	EXPRESSIVEPIXEL_SEQUENCE_META *pMeta = (EXPRESSIVEPIXEL_SEQUENCE_META *) pAnimationBytes;
	EXPRESSIVEPIXEL_SEQUENCE sequence;

	memset(&sequence, 0x00, sizeof(sequence));
	memcpy(&sequence.Meta, pAnimationBytes, sizeof(EXPRESSIVEPIXEL_SEQUENCE_META));	
	
	
	int siz  = sizeof(EXPRESSIVEPIXEL_SEQUENCE_META);
	uint8_t *BUF = (((uint8_t *) pAnimationBytes) + sizeof(EXPRESSIVEPIXEL_SEQUENCE_META));
	
	sequence.pROMPalette = (PALETTE_ENTRY *)(((uint8_t *) pAnimationBytes) + sizeof(EXPRESSIVEPIXEL_SEQUENCE_META)); 	
	sequence.pROMFrames = ((uint8_t *) pAnimationBytes) + sizeof(EXPRESSIVEPIXEL_SEQUENCE_META) + sequence.Meta.cbPallete;
	return Activate(&sequence);
}



bool CAnimationManager::Activate(EXPRESSIVEPIXEL_SEQUENCE *pSequence)
{
	ACTIVE_ANIMATIONSEQUENCE *pActiveSequence = NULL;

	Clear();	
	
	// Validate first
	if((pSequence->pRAMPalette != NULL || pSequence->pROMPalette != NULL) &&		
		(pSequence->pFile != NULL || (pSequence->pFile == NULL &&  (pSequence->pRAMFrames != NULL || pSequence->pROMFrames != NULL))))
	{
		pActiveSequence = (ACTIVE_ANIMATIONSEQUENCE *)malloc(sizeof(ACTIVE_ANIMATIONSEQUENCE));
		if (pActiveSequence != NULL)
		{
			memset(pActiveSequence, 0x00, sizeof(ACTIVE_ANIMATIONSEQUENCE));
			memcpy(&pActiveSequence->Sequence, pSequence, sizeof(EXPRESSIVEPIXEL_SEQUENCE));
			pActiveSequence->Sequence.pPalette = pActiveSequence->Sequence.pRAMPalette != NULL ? pActiveSequence->Sequence.pRAMPalette : pActiveSequence->Sequence.pROMPalette;
			pActiveSequence->Sequence.pActiveFrames = pActiveSequence->Sequence.pRAMFrames != NULL ? pActiveSequence->Sequence.pRAMFrames : pActiveSequence->Sequence.pROMFrames;			
			pActiveSequence->previousFrameRateMillis = millis();
			pActiveSequence->currentRepeatIteration = 0;
			pActiveSequence->frameInterval = 1000 / pActiveSequence->Sequence.Meta.frameRate;			
			ProcessFrame(pActiveSequence);
		}
		m_pActiveSequence = pActiveSequence;
	}
	return pActiveSequence != NULL;
}



void CAnimationManager::Clear()
{
	DEBUGLOGLN("**CAnimator::Clear**");

	Stop();
	m_pCDisplayArray->Clear();
	m_pCDisplayArray->Show(true);
}



void CAnimationManager::Stop()
{
	if (m_pActiveSequence != NULL)
	{
		ExpressivePixelSequenceFree(&m_pActiveSequence->Sequence);
		free(m_pActiveSequence);
		m_pActiveSequence = NULL;
	}
	m_activeDelayMillis = m_previousDelayMillis = 0;
	m_activeFadeMillis = m_previousFadeMillis = 0;
}



bool CAnimationManager::IsActiveAnimationInfinite(char *pszID)
{
	return (m_pActiveSequence != NULL && stricmp(m_pActiveSequence->Sequence.szID, pszID)==0 && m_pActiveSequence->Sequence.Meta.loopCount == 0);
}



bool CAnimationManager::Process()
{
	if (m_pActiveSequence != NULL)
	{
		unsigned long currentMillis = millis();
		
		// Test for an active delay
		if(m_activeDelayMillis != 0)
		{
			if (currentMillis - m_previousDelayMillis < m_activeDelayMillis)
				return false;
			m_activeDelayMillis = m_previousDelayMillis = 0;
		}
		// Test for an active fade
		else if(m_activeFadeMillis != 0)
		{						
			float fadeStepMillis = (float) m_activeFadeMillis  / (float) ANIMATION_FADE_KERNEL;
		
			if (currentMillis - m_previousFadeMillis < (int) fadeStepMillis)
				return false;
			
			m_activeFadeKernel++;
			if (m_activeFadeKernel < ANIMATION_FADE_KERNEL)
			{				
				// Update the display's brightness			
				m_previousFadeMillis = millis();
			
				uint8_t originalBrightness = m_pCDisplayArray->GetBrightness();
				float fKernel = (float)(ANIMATION_FADE_KERNEL - m_activeFadeKernel) / (float) ANIMATION_FADE_KERNEL;		
				uint8_t newBrightness = (uint8_t)((float) originalBrightness * fKernel);
			
				m_pCDisplayArray->SetBrightness(newBrightness, true);
				m_pCDisplayArray->Show();
				m_pCDisplayArray->SetBrightness(originalBrightness);	
				return false;
			}
			else
				m_activeFadeMillis = m_previousFadeMillis = 0;
		}
		else if(m_pActiveSequence->previousFrameRateMillis != 0 && currentMillis - m_pActiveSequence->previousFrameRateMillis < m_pActiveSequence->frameInterval)
			return false;
		m_pActiveSequence->previousFrameRateMillis = currentMillis;

		return ProcessFrame(m_pActiveSequence);
	}
	return false;
}



bool CAnimationManager::ReadAnimationBytes(ACTIVE_ANIMATIONSEQUENCE *pSequence, uint8_t *pBuf, uint16_t cb)
{	
#ifdef VARIANTCAPABILITY_STORAGE	
	if (pSequence->Sequence.pFile != NULL)
	{
		while (cb--)
		{
			if (CStorage::ReadFile(pSequence->Sequence.pFile, pBuf + cb, sizeof(uint8_t)) != sizeof(uint8_t))
				return false; // ERROR - mark animation as complete
			pSequence->currentFrameOffset++;
		}
	}
	else
#endif		
	{
		uint8_t *pCurrentFrame = pSequence->Sequence.pActiveFrames + pSequence->currentFrameOffset;
		while (cb--)
		{
			*(pBuf+cb) = *pCurrentFrame++;
			pSequence->currentFrameOffset++;
		}
	}
	return true;
}



bool CAnimationManager::RenderFrameToBytes(ACTIVE_ANIMATIONSEQUENCE *pSequence, uint8_t *pFrameAsBytes)
{
	return ProcessFrame(pSequence, pFrameAsBytes);	
}



bool CAnimationManager::ProcessFrame(ACTIVE_ANIMATIONSEQUENCE *pSequence, uint8_t *pFrameAsBytes)
{
	char        frameType;	
	uint16_t    pixelCount;
	uint16_t	totalPixels = m_pCDisplayTopology != NULL ? m_pCDisplayTopology->TotalPixels() : m_pCDisplayArray->TotalPixels();
	
	// Only read more if there are frames remaining
	if(pSequence->currentFrameOffset < pSequence->Sequence.Meta.cbFrames)
	{		
		// Read frame type
		if(!ReadAnimationBytes(pSequence, (uint8_t *) &frameType, sizeof(frameType)))
			return true;
	
		// Determine the animation frame type or action
		switch(frameType)
		{
		case 'I':
			// Extract the pixel count
			if(!ReadAnimationBytes(pSequence, (uint8_t *) &pixelCount, sizeof(pixelCount)))
				return true;
			//DEBUGLOGLN("Animator KEY frame %d pixels", pixelCount);
			
			// Process pixels
			for(int i = 0 ; i < pixelCount ; i++)
			{
				uint8_t  paletteIndex;
				uint16_t pixelPosition;
			
				// Extract palette index
				if(!ReadAnimationBytes(pSequence, (uint8_t *) &paletteIndex, sizeof(paletteIndex)))
					return true;

				if (paletteIndex <= pSequence->Sequence.Meta.cbPallete - 1)
				{
					PALETTE_ENTRY *paletteEntry = pSequence->Sequence.pPalette + paletteIndex;
					uint32_t color = CDisplayArray::ColorFromBytes(paletteEntry->r, paletteEntry->g, paletteEntry->b);

					// Figure topographical position
					if(m_pCDisplayTopology != NULL)
						pixelPosition = m_nRotation == 0 ? m_pCDisplayTopology->Map(i) : m_pCDisplayTopology->MapRotated(i, m_nRotation);
					else
						pixelPosition = i;							
					if (pixelPosition != TOPOLOGYPIXEL_UNASSIGNED)
					{
						if (pFrameAsBytes != NULL)
							m_pCDisplayArray->GetPixelBytes(color, pFrameAsBytes + (i * 3), pFrameAsBytes + (i * 3) + 1, pFrameAsBytes + (i * 3) + 2);
						else
							// Extract color from palette and set LED color
							m_pCDisplayArray->SetPixelColor(pixelPosition, color);
					}
				}			
			}
			if (pFrameAsBytes == NULL)
				m_pCDisplayArray->Show();
			break;

		case 'P':
			// Extract the pixel count
			if(!ReadAnimationBytes(pSequence, (uint8_t *) &pixelCount, sizeof(pixelCount)))
				return true;
			//DEBUGLOGLN("Animator Predictive frame %d pixels", pixelCount);
			
			// Process pixels
			for(int i = 0 ; i < pixelCount ; i++)
			{
				uint8_t  paletteIndex;
				uint16_t logicalPixelPosition = 0;
				uint16_t physicalPixelPosition = TOPOLOGYPIXEL_UNASSIGNED;

				// Determine if 8 or 16 bit pixel position size
				uint8_t pixelPositionByteSize = totalPixels > 256 ? sizeof(uint16_t) : sizeof(uint8_t);
			
				// Extract pixel position
				if(!ReadAnimationBytes(pSequence, (uint8_t *) &logicalPixelPosition, pixelPositionByteSize))
					return true;

				// Extract palette index
				if(!ReadAnimationBytes(pSequence, (uint8_t *) &paletteIndex, sizeof(paletteIndex)))
					return true;

				if (paletteIndex < pSequence->Sequence.Meta.cbPallete)
				{				
					PALETTE_ENTRY *paletteEntry = pSequence->Sequence.pPalette + paletteIndex;
					uint32_t color = CDisplayArray::ColorFromBytes(paletteEntry->r, paletteEntry->g, paletteEntry->b);
				
					if (m_pCDisplayTopology != NULL)
						physicalPixelPosition = m_nRotation == 0 ? m_pCDisplayTopology->Map(logicalPixelPosition) : m_pCDisplayTopology->MapRotated(logicalPixelPosition, m_nRotation);
				
					// If a positional pixel
					if(physicalPixelPosition != TOPOLOGYPIXEL_UNASSIGNED)
					{
						if (pFrameAsBytes != NULL)
							m_pCDisplayArray->GetPixelBytes(color, pFrameAsBytes + (logicalPixelPosition * 3), pFrameAsBytes + (logicalPixelPosition * 3) + 1, pFrameAsBytes + (logicalPixelPosition * 3) + 2);
						else
							// Extract color from palette and set LED color
							m_pCDisplayArray->SetPixelColor(physicalPixelPosition, color);
					}
				}
			}
			if (pFrameAsBytes == NULL)
				m_pCDisplayArray->Show();
			break;

		case 'D':		
			uint16_t delayMillis;
		
			if (!ReadAnimationBytes(pSequence, (uint8_t *) &delayMillis, sizeof(delayMillis)))
				return true;
			m_activeDelayMillis = delayMillis;
			m_previousDelayMillis = millis();
			DEBUGLOGLN("WAIT %d", m_activeDelayMillis);
			break;
		
		case 'F':
			uint16_t fadeMillis;
		
			if (!ReadAnimationBytes(pSequence, (uint8_t *) &fadeMillis, sizeof(fadeMillis)))
				return true;
			m_activeFadeKernel = 0;
			m_activeFadeMillis = fadeMillis;
			m_previousFadeMillis = millis();
			DEBUGLOGLN("FADE over %d ms", m_activeFadeMillis);
			return false;
		}
	}

	// See if all of the animation bytes have been read
	if(pFrameAsBytes == NULL && pSequence->currentFrameOffset >= pSequence->Sequence.Meta.cbFrames)
	{
		//DEBUGLOGLN("Animator ALL animation bytes read %d >= %d", pSequence->currentFrameOffset ,pSequence->Sequence.framesByteLen);
		
		/*** Last frame in animation now complete ***/
		if(pSequence->Sequence.Meta.loopCount == 1)
			return true;
		else
		{
			if (pSequence->Sequence.Meta.loopCount > 1)
			{
				if (pSequence->Sequence.Meta.loopCount < 255)
				{
					// This animation repeats
					pSequence->currentRepeatIteration++;

					// If all repeats complete
					if(pSequence->currentRepeatIteration >= pSequence->Sequence.Meta.loopCount)
					{
						//DEBUGLOGLN("Animator ALL repeats complete %d >= %d", pSequence->currentRepeatIteration, pSequence->Sequence.loopCount);
						// Return full complete
						return true;
					}
				}
			}
			else if (m_disableInfiniteLooping)
				return true;

			// Start the animation over again
			pSequence->currentFrameOffset = 0;
			
#ifdef VARIANTCAPABILITY_STORAGE			
			if (pSequence->Sequence.pFile != NULL)
				CStorage::Seek(pSequence->Sequence.pFile, pSequence->Sequence.frameBytesStartOffset);
#endif			
			m_pCDisplayArray->Clear();
		}
	}

	// Animation still in progress
	//DEBUGLOGLN("Animation still in progress");
	return false;
}



void CAnimationManager::ExpressivePixelSequenceFree(EXPRESSIVEPIXEL_SEQUENCE *pSequence)
{
	if (pSequence != NULL)
	{
#ifdef VARIANTCAPABILITY_STORAGE		
		if (pSequence->pFile != NULL)
		{
			CStorage::Close(pSequence->pFile);
			pSequence->pFile = NULL;
		}
#endif		
		if (pSequence->pRAMFrames != NULL)
		{
			free(pSequence->pRAMFrames);
			pSequence->pRAMFrames = NULL;
		}
		if (pSequence->pRAMPalette != NULL)
		{
			free(pSequence->pRAMPalette);
			pSequence->pRAMPalette = NULL;
		}		
		if (pSequence->pszName != NULL)
		{
			free(pSequence->pszName);
			pSequence->pszName = NULL;
		}		
		memset(pSequence, 0x00, sizeof(EXPRESSIVEPIXEL_SEQUENCE));
	}
}



void CAnimationManager::ShowSingleFrame(uint8_t *pFrame, uint16_t pixelCount)
{
	uint8_t		byteRed, byteGreen, byteBlue;
	uint16_t    pixelPosition;
	
	//DEBUGLOGLN("CAnimationManager::ShowSingleFrame %d pixels %s", pixelCount, m_pCDisplayTopology != NULL ? "ACTIVE" : "INACTIVE");	
	for (uint16_t i = 0; i < pixelCount; i++)
	{		
		pixelPosition = TOPOLOGYPIXEL_UNASSIGNED;
		byteRed = *pFrame++;
		byteGreen = *pFrame++;
		byteBlue = *pFrame++;

		// Find topographical position
		if(m_pCDisplayTopology != NULL)
			pixelPosition = m_nRotation == 0 ? m_pCDisplayTopology->Map(i) : m_pCDisplayTopology->MapRotated(i, m_nRotation);
					
		// Light up LED
		if(pixelPosition != TOPOLOGYPIXEL_UNASSIGNED)
			m_pCDisplayArray->SetPixelRGB(pixelPosition, byteRed, byteGreen, byteBlue);
	}
	m_pCDisplayArray->Show();
}



void CAnimationManager::ShowSinglePixel(uint16_t logicalPixelPosition, uint32_t color)
{
	uint16_t pixelPosition = TOPOLOGYPIXEL_UNASSIGNED;

	// Find topographical position
	if(m_pCDisplayTopology != NULL)
		pixelPosition = m_nRotation == 0 ? m_pCDisplayTopology->Map(logicalPixelPosition) : m_pCDisplayTopology->MapRotated(logicalPixelPosition, m_nRotation);
	
	if (pixelPosition != TOPOLOGYPIXEL_UNASSIGNED)	
		m_pCDisplayArray->ShowSinglePixel(pixelPosition, color);	
}

