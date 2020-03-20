// ===============================================================================
// FILE NAME: dvCard.c
// DESCRIPTION:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ===============================================================================

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include "Driver/Bus/types.h"
#include "Driver/Bus/SPI.h"

#include "dvCard.h"

#define TIME_OUT        (10)
#define RETRY_DELAY     (10)
#define SLEEP_BTWN_RW   (2)
#define RETRY_COUNT     (1)

#define DVCARD_DBG_LEVEL 7  // 0: Always, 1,2: Error, 3,4: Warning, 5,6: Info, 7,8,9: Debugging
#define DVCARD_DBG_PREFIX "dvCard.c > "
#define DVCARD_DBG_MSG(level, cmd) if(level <= DVCARD_DBG_LEVEL){ cmd; }

static WORD m_wSeqId = 0;
//StaticSemaphore_t m_axCardSemaphoreBuffer;
//SemaphoreHandle_t m_xCardSemaphore;

static sMSG_STATE_DATA  m_sCardState;

// ==============================================================================
// FUNCTION NAME: dvCard_Device_Read
// DESCRIPTION:
//
//
// Params:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
static eRESULT dvCard_Device_Write(WORD wSize, BYTE *pcData)
{
#if 1
    UBYTE* readBuffer = malloc(wSize);
    BOOL ok = SPITransferBuffer(pcData, readBuffer, wSize);
    int i = 0;
    DVCARD_DBG_MSG(9, printf(DVCARD_DBG_PREFIX "Writing %d bytes to SPI %s:", wSize, ok ? "ok" : "failed"));
    for (i=0; i<wSize; i++)
    {
        DVCARD_DBG_MSG(9, printf("0x%x ", *(pcData + i)));
    }
    DVCARD_DBG_MSG(9, printf("\n\n\n\n\n"));
    free(readBuffer);
    return ok ? rcSUCCESS : rcERROR;
#else
    return rcSUCCESS;
#endif // 0
}

// ==============================================================================
// FUNCTION NAME: dvCard_Device_Read
// DESCRIPTION:
//
//
// Params:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
static eRESULT dvCard_Device_Read(WORD wSize, BYTE *pcData)
{
#if 1
    static int count = 0;
    UBYTE* writeBuffer = calloc(wSize, 1);
    BOOL ok = SPITransferBuffer(writeBuffer, pcData, wSize);
    int i = 0;
    //   DVCARD_DBG_MSG(9, printf(DVCARD_DBG_PREFIX "Reading %d bytes from SPI %s:", wSize, ok ? "ok" : "failed"));
    for (i=0; i<wSize; i++)
    {
        if ((*(pcData + i) == 0) || (*(pcData + i) == 0xff))
            count++;
        else
            count = 0;
        if (count < 5)
        {
            DVCARD_DBG_MSG(9, printf("0x%x ", *(pcData + i)));
        }
    }
    // DVCARD_DBG_MSG(9, printf("\n"));
    free(writeBuffer);
    return ok ? rcSUCCESS : rcERROR;
#else
    return rcSUCCESS;
#endif // 0
}

// ==============================================================================
// FUNCTION NAME: dvCard_State_Reset
// DESCRIPTION:
//
//
// Params:
// sMSG_STATE_DATA *psMsgData:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
static void dvCard_State_Reset(sMSG_STATE_DATA *psMsgData)
{
    memset(&psMsgData->sMsgPacket, 0, sizeof(sMSG_PACKET_FORMAT) / sizeof(BYTE));
    psMsgData->eMsgParsingState = eMSG_STATE_MAGIC_NUMBER;
    psMsgData->wRecivedByteCount = 0;
    psMsgData->wRecivedByteCRC = 0;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Initial
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
void dvCard_Initial(void)
{
    BOOL spiOpen = SPIOpen();

    DVCARD_DBG_MSG(0, printf(DVCARD_DBG_PREFIX "SPI device %s.\n", spiOpen ? "open" : "FAILED"))

    //m_xCardSemaphore = xSemaphoreCreateMutexStatic(&m_axCardSemaphoreBuffer);

    dvCard_State_Reset(&m_sCardState);

    m_sCardState.fpWriteFunc = dvCard_Device_Write;
    m_sCardState.fpReadFunc = dvCard_Device_Read;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Command_Write
// DESCRIPTION:
//
//
// Params:
// eCARD_CMD eCmd:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
eRESULT dvCard_Command_Write(eCMD_MODULE eModule, BYTE cSubCmd, WORD wSize, BYTE *pcData)
{
    eRESULT eResult = rcERROR;
    BYTE cRetry = RETRY_COUNT;
    sPAYLOAD sPayload;
    WORD wOutputSize = 0;

    do
    {
        wOutputSize = wSize;
        dvCard_State_Reset(&m_sCardState);

        sPayload.cModule = eModule;
        wOutputSize++;
        sPayload.cSubCmd = cSubCmd;
        wOutputSize++;

        if(0 != wSize)
        {
            memcpy(sPayload.acData, pcData, wSize);
        }

        utilHost_PacketBuild(&m_sCardState.sMsgPacket, eDEVICE_DOCKING_BOARD, eDEVICE_GIM, ePAYLOAD_TYPE_EXE_WRITE, m_wSeqId, wOutputSize, &sPayload);

        if(NULL != m_sCardState.fpWriteFunc)
        {
            m_sCardState.fpWriteFunc((HEAD_PACK_SIZE + m_sCardState.sMsgPacket.sPacketHeader.wPacketSize), (BYTE *)&m_sCardState.sMsgPacket);
        }

        // utilDelayMs(SLEEP_BTWN_RW);
        dvCard_State_Reset(&m_sCardState);

        // utilCounterSet(eCOUNTER_TYPE_CARD_WRITE, TIME_OUT);
        // TODO: Use real timer instead of counter?
        unsigned long count = 0xFFFF;
        while(eMSG_STATE_DATA_READY > m_sCardState.eMsgParsingState)
        {
            utilHost_StateProcess(&m_sCardState, 100);

            if(eMSG_STATE_DATA_READY == m_sCardState.eMsgParsingState)
            {
                if(m_wSeqId == m_sCardState.sMsgPacket.sPacketHeader.wSeqId)
                {
                    if((ePAYLOAD_TYPE_ACK == m_sCardState.sMsgPacket.sPacketHeader.cPacketType)
                       && (eACK_TYPE_ACK == m_sCardState.sMsgPacket.uFormat.acPacketPayload[0]))
                    {
                        eResult = rcSUCCESS;
                    }
                }
                else
                {
                    // utilCounterSet(eCOUNTER_TYPE_CARD_WRITE, TIME_OUT);
                    count = 0xFFFF;
                    dvCard_State_Reset(&m_sCardState);
                }
            }

            if(count == 0)
            {
                m_sCardState.eMsgParsingState = eMSG_STATE_TIMEOUT;
                break;
            }
            count--;
        }

        switch(m_sCardState.eMsgParsingState)
        {
        case eMSG_STATE_BAD_PACKET:
        case eMSG_STATE_TIMEOUT:
        case eMSG_STATE_RUN_OUT_OF_MEMORY:
        case eMSG_STATE_INITIAL_ERROR:
            //utilDelayMs(RETRY_DELAY);
            usleep(RETRY_DELAY * 1000);
            DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "DB_Write retry\n"))
            eResult = rcERROR;
            break;

        default:
            break;
        }

        m_wSeqId++;
    }
    while((cRetry--) && (eResult == rcERROR));

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Command_Read
// DESCRIPTION:
//
//
// Params:
// eCARD_CMD eCmd:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
eRESULT dvCard_Command_Read(eCMD_MODULE eModule, BYTE cSubCmd, WORD wSize, BYTE *pcData)
{
    eRESULT eResult = rcERROR;
    sPAYLOAD sPayload;
    BYTE cRetry = RETRY_COUNT;
    WORD wOutputSize = 0;
    WORD wIndex = 0;

    //if(NULL != m_xCardSemaphore)
    {
        //if(pdPASS == xSemaphoreTake(m_xCardSemaphore, portMAX_DELAY))
        {
            do
            {
                wOutputSize = 0;
                dvCard_State_Reset(&m_sCardState);

                sPayload.cModule = eModule;
                wOutputSize++;
                sPayload.cSubCmd = cSubCmd;
                wOutputSize++;

                if(0 != wSize)
                {
                    memcpy(sPayload.acData, pcData, wSize);
                }

                utilHost_PacketBuild(&m_sCardState.sMsgPacket, eDEVICE_DOCKING_BOARD, eDEVICE_GIM, ePAYLOAD_TYPE_EXE_READ, m_wSeqId, wOutputSize, &sPayload);

                if(NULL != m_sCardState.fpWriteFunc)
                {
                    m_sCardState.fpWriteFunc((HEAD_PACK_SIZE + m_sCardState.sMsgPacket.sPacketHeader.wPacketSize), (BYTE *)&m_sCardState.sMsgPacket);
                }

                // utilDelayMs(SLEEP_BTWN_RW);
                usleep(SLEEP_BTWN_RW * 1000);

                dvCard_State_Reset(&m_sCardState);

                // utilCounterSet(eCOUNTER_TYPE_CARD_READ, TIME_OUT);
                unsigned long count = 0xFFFF;

                while(eMSG_STATE_DATA_READY > m_sCardState.eMsgParsingState)
                {
                    utilHost_StateProcess(&m_sCardState, 100);

                    if(eMSG_STATE_DATA_READY == m_sCardState.eMsgParsingState)
                    {
                        if(m_wSeqId == m_sCardState.sMsgPacket.sPacketHeader.wSeqId)
                        {
                            if(ePAYLOAD_TYPE_REPLY == m_sCardState.sMsgPacket.sPacketHeader.cPacketType)
                            {
                                memcpy(pcData, m_sCardState.sMsgPacket.uFormat.acPacketPayload, m_sCardState.sMsgPacket.sPacketHeader.wPacketSize);
                                DVCARD_DBG_MSG(9, printf(DVCARD_DBG_PREFIX "Read Size = %d\n", m_sCardState.sMsgPacket.sPacketHeader.wPacketSize));
                                eResult = rcSUCCESS;
                            }
                        }
                        else
                        {
                            //utilCounterSet(eCOUNTER_TYPE_CARD_READ, TIME_OUT);
                            count = 0xFFFF;
                            dvCard_State_Reset(&m_sCardState);
                        }
                    }

                    if(count == 0)
                    {
                        m_sCardState.eMsgParsingState = eMSG_STATE_TIMEOUT;
                        break;
                    }
                    count--;
                }

                switch(m_sCardState.eMsgParsingState)
                {
                    case eMSG_STATE_TIMEOUT:
                        usleep(RETRY_DELAY * 1000);
                        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "DB_Read Timeout\n"))
                        eResult = rcERROR;
                        break;
                    case eMSG_STATE_BAD_PACKET:
                    case eMSG_STATE_RUN_OUT_OF_MEMORY:
                    case eMSG_STATE_INITIAL_ERROR:
                        // utilDelayMs(RETRY_DELAY);
                        usleep(RETRY_DELAY * 1000);
                        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "DB_Read retry\n"))
                        eResult = rcERROR;
                        break;

                    default:
                        break;
                }

                m_wSeqId++;
            }
            while((cRetry--) && (eResult == rcERROR));

            //xSemaphoreGive(m_xCardSemaphore);
        }
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dvCard_System_Ready_Get
// DESCRIPTION:
//
//
// Params:
// BYTE *pcReady:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/11/26, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_System_Ready_Get(BYTE *pcReady)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Read(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_READY, 0, pcReady);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Ready %d\n", *pcReady))
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "System Ready Error\n"))
    }

    return eResult == rcSUCCESS;
}

#if 0
// ==============================================================================
// FUNCTION NAME: dvCard_System_Card_Present_Get
// DESCRIPTION:
//
//
// Params:
// WORD *pwPresent:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/11/26, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_System_Card_Present_Get(WORD *pwPresent)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Read(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_CARD_PRESENT, 0, (BYTE *)pwPresent);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Card Present 0x%02X\n", *pwPresent))
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Read Card Present Error\n"))
    }

    return eResult == rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: dvCard_System_Card_Power_Set
// DESCRIPTION:
//
//
// Params:
// WORD *pwPresent:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/11/26, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_System_Card_Power_Set(WORD wEnable)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Write(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_CARD_POWER, sizeof(WORD), (BYTE *)&wEnable);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Card Power Pass\n"))
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Card Power Error\n"))
    }

    return eResult == rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: dvCard_System_Card_Power_Get
// DESCRIPTION:
//
//
// Params:
// WORD *pwPresent:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/11/26, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_System_Card_Power_Get(WORD *pwEnable)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Read(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_CARD_POWER, 0, (BYTE *)pwEnable);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Card Power 0x%02X\n", *pwEnable))
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Card Power Error\n"))
    }

    return eResult == rcSUCCESS;
}
#endif // 0

// ==============================================================================
// FUNCTION NAME: dvCard_System_Revision_Get
// DESCRIPTION:
//
//
// Params:
// eSOURCE_PORT ePort:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_System_Revision_Get(BYTE *pcVersion)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Read(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_VERSION, 0, pcVersion);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Version %d, %d, %d, %d\n", pcVersion[0], pcVersion[1], pcVersion[2], pcVersion[3]))
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Read Version Error\n"))
    }

    return eResult == rcSUCCESS;
}

#if 0
// ==============================================================================
// FUNCTION NAME: dvCard_Input_Port_Set
// DESCRIPTION:
//
//
// Params:
// eSOURCE_PORT ePort:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_Input_Port_Set(eSOURCE_LIST ePort)
{
    eRESULT eResult = rcERROR;

    if(eSOURCE_LIST_NUMBERS > ePort)
    {
        eResult = dvCard_Command_Write(eCMD_MODULE_SOURCE_SWITCH, eCMD_SOURCE_PORT_SWITCH, 1, (BYTE *)&ePort);

        if(rcSUCCESS == eResult)
        {
            DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Switch Pass\n"))
        }
        else
        {
            DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Switch Fail\n"))
        }
    }

    return eResult == rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Video_Timing_Get
// DESCRIPTION:
//
//
// Params:
// sVIDEO_TIMING *psVideoTiming:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/11/19, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_Get_Video_Mode(MODEINFO* modeInfo)
{
    sVIDEO_TIMING videoTiming;
    sVIDEO_FORMAT videoFormat;
    eRESULT eResult = rcERROR;

    memset(modeInfo, 0x00, sizeof(MODEINFO));
    eResult = dvCard_Command_Read(eCMD_MODULE_SOURCE_VIDEO, eCMD_SOURCE_VIDEO_TIMING, 0, (BYTE*)&videoTiming);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Timing Pass\n"))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "PCLK       = %d\n", videoTiming.dwPixClock))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HTotal     = %d\n", videoTiming.wHTol))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HDEW       = %d\n", videoTiming.wHAct))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HFPH       = %d\n", videoTiming.wHFront))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HSyncW     = %d\n", videoTiming.wHSync))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HBPH       = %d\n", videoTiming.wHBack))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HSyncPol   = %d\n", videoTiming.cHPol))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VTotal     = %d\n", videoTiming.wVTol))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VDEW       = %d\n", videoTiming.wVAct))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VFPH       = %d\n", videoTiming.wVFront))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VSyncW     = %d\n", videoTiming.wVSync))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VBPH       = %d\n", videoTiming.wVBack))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VSyncPol   = %d\n", videoTiming.cVPol))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "InterLaced = %d\n", videoTiming.cScanType))

        eResult = dvCard_Command_Read(eCMD_MODULE_SOURCE_VIDEO, eCMD_SOURCE_VIDEO_FORMAT, 0, (BYTE*)&videoFormat);

        if(rcSUCCESS == eResult)
        {
            DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Format Pass\n"))

            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Asp Ratio   = %d\n", videoFormat.eAspectRatio))
            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ColorDepth  = %d\n", videoFormat.eColorDepth))
            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ColorRange  = %d\n", videoFormat.eColorRange))
            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ColorSpace  = %d\n", videoFormat.eColorSpace))
            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HDCP Ver    = %d\n", videoFormat.cHdcpVersion))
            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HDMI Mode   = %d\n", videoFormat.cHdmiMode))
            DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Scramble    = %d\n", videoFormat.cScrambling))

            modeInfo->Valid = TRUE;
            modeInfo->PixClock = videoTiming.dwPixClock;    // Pixel clock in Hz

            if(0 != videoTiming.wHTol)
            {
                modeInfo->HFreq = videoTiming.dwPixClock / videoTiming.wHTol ;       // Horizontal frequency in Hz
            }

            if(0 != videoTiming.wVTol)
            {
                modeInfo->VFreq = (modeInfo->HFreq * 1000) / videoTiming.wVTol;       // Vertical frequency in Hz * 1000
            }

            modeInfo->HTot = videoTiming.wHTol;        // Total horizontal pixels
            modeInfo->VTot = videoTiming.wVTol;        // Total vertical pixels
            modeInfo->HAct = videoTiming.wHAct;        // Active horizontal pixels
            modeInfo->VAct = videoTiming.wVAct;        // Acive vertical pixels
            modeInfo->HBack = videoTiming.wHBack;       // Horizontal back porch
            modeInfo->VBack = videoTiming.wVBack;       // Vertical back porch
            modeInfo->HSync = videoTiming.wHSync;       // Horizontal sync width
            modeInfo->VSync = videoTiming.wVSync;       // Vertical sync width;
            modeInfo->HPol = videoTiming.cHPol;        // Horizontal sync polarity TODO: Correct?
            modeInfo->VPol = videoTiming.cVPol;        // Vertical sync polarity TODO: Correct?
            modeInfo->ScanType = videoTiming.cScanType;    // PROG_SCAN or INTER_SCAN TODO: Correct?
            modeInfo->AspectRatio = AR_IS_4_3; // Image aspect ratio: Either AR_SQUARE_PIX or (hor<<16 | ver), e.g. AR_IS_4_3
            modeInfo->CSampling = RGB444;   // Color sampling, e.g. RGB444, YUV420, etc. TODO: Conversion!
            modeInfo->CRange = videoFormat.eColorRange;      // Color range, i.e. CRNG_FULL, CRNG_LIMITED or CRNG_DEFAULT
            // modeInfo->Colorimetry; // Color standard
            // modeInfo->CEAMode;     // Detected CEA mode (0 = unknown or non-CEA)
            modeInfo->Bpp = videoFormat.eColorDepth;         // BPP24, BPP30 or BPP36
            modeInfo->HDCP = videoFormat.cHdcpVersion;        // TRUE if HDCP encrypted
            modeInfo->HDMI = videoFormat.cHdmiMode;        // TRUE if HDMI mode
            modeInfo->PixPerClk = 0;   // Pixels per clock + 1 (0 = single pixel, 1 = double pixel)
        }
        else
        {
            DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Format Fail\n"))
        }
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Timing Fail\n"))
    }

    return eResult == rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Audio_Info_Get
// DESCRIPTION:
//
//
// Params:
// sAUDIO_INFO *psAudioInfo:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/10/30, Leo Create
// 2019/12/18, Thomas modify
// --------------------
// ==============================================================================
BOOL dvCard_Audio_Info_Get(AUDIOMODEINFO* audioInfo)
{
    eRESULT eResult = rcERROR;

    sAUDIO_INFO sAudioInfo;
    eResult = dvCard_Command_Read(eCMD_MODULE_SOURCE_AUDIO, eCMD_SOURCE_AUDIO_INFO, 0, (BYTE*)&sAudioInfo);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Audio Pass\n"))

        audioInfo->AudioType = sAudioInfo.cAudioType;       // Audio type
        audioInfo->ChannelCount = sAudioInfo.cChannelCount; // Audio channel count
        audioInfo->SpeakMapping = SPEAKER_STEREO;           // Speaker mapping
        audioInfo->DownmixLevel = 0;                        // Downmix level
        audioInfo->DownmixPermit = FALSE;                   // Permission to downmix?
        audioInfo->ChannelMask = 0;                         // Channel mask
        memset(&(audioInfo->ChannelStatus), 0xFF, 5);       // Channel Status bits
        audioInfo->SampleSize = sAudioInfo.cSampleSize;     // Sample size
        audioInfo->SampleFreq = sAudioInfo.cSampleFreq;;    // Sample frequency
        audioInfo->MuteSPDIF = audioInfo->ChannelCount > SPEAKER_TWO; // Mute SPDIF output flag
        audioInfo->ForceSPDIF = FALSE;                      // Force output to use SPDIF as source

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Audio      = %d\n", sAudioInfo.cIsAudio))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Type       = %d\n", sAudioInfo.cAudioType))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ChCnt      = %d\n", sAudioInfo.cChannelCount))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Freq       = %d\n", sAudioInfo.cSampleFreq))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Size       = %d\n", sAudioInfo.cSampleSize))
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Audio Fail\n"))
    }

    return eResult == rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Slot_Info_Get
// DESCRIPTION:
//
//
// Params:
// sAUDIO_INFO *psAudioInfo:
//
// Returns:
//
//
// modification history
// --------------------
// 2020/02/04, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_Slot_Info_Get(sSLOT_INFO *psSlotInfo)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Read(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_SLOT_INFO, 0, (BYTE *)psSlotInfo);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Slot Info %d, %s\r\n", psSlotInfo->cInputs, psSlotInfo->acInputName))
    }
    else
    {
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Read Slot Info Error\r\n"))
    }

    return eResult == rcSUCCESS;
}
#endif // 0

// ==============================================================================
// FUNCTION NAME: dvCard_Slot_Ready_Get
// DESCRIPTION:
//
//
// Params:
// BYTE *pcReady:
//
// Returns:
//
//
// modification history
// --------------------
// 2020/02/04, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_Slot_Ready_Get(BYTE *pcReady)
{
    eRESULT eResult = rcERROR;

    eResult = dvCard_Command_Read(eCMD_MODULE_SYSTEM, eCMD_SYSTEM_SLOT_READY, 0, pcReady);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Slot Ready %d, %d\r\n", *pcReady))
    }
    else
    {
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Read Slot Ready Error\r\n"))
    }

    return eResult == rcSUCCESS? 1:0;
}

#if 0
BOOL dvCard_Input_Hdcp_Enable(BYTE input, BYTE enable)
{
    return TRUE;
}

BOOL dvCard_Input_Has_Signal(BYTE input, BOOL* hasSignal)
{
    *hasSignal = TRUE;
    return TRUE;
}

// ==============================================================================
// FUNCTION NAME: dvCard_Video_Info_Get
// DESCRIPTION:
//
//
// Params:
// BYTE input:
// MODEINFO* modeInfo:
//
// Returns:
//
//
// modification history
// --------------------
// 2019/02/06, Leo Create
// --------------------
// ==============================================================================
BOOL dvCard_Video_Info_Get(BYTE input, MODEINFO* modeInfo)
{
    sVIDEO_INFO videoInfo;
    eRESULT eResult = rcERROR;

    memset(modeInfo, 0x00, sizeof(MODEINFO));
    memset(&videoInfo, 0x00, sizeof(sVIDEO_INFO));
    eResult = dvCard_Command_Read(eCMD_MODULE_SOURCE_VIDEO, eCMD_SOURCE_VIDEO_INFO, 0, (BYTE*)&videoInfo);

    if(rcSUCCESS == eResult)
    {
        DVCARD_DBG_MSG(7, printf(DVCARD_DBG_PREFIX "Info Pass\n"))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Stable     = %d\n", videoInfo.cStable))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "PCLK       = %d\n", videoInfo.sTimingInfo.dwPixClock))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HTotal     = %d\n", videoInfo.sTimingInfo.wHTol))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HDEW       = %d\n", videoInfo.sTimingInfo.wHAct))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HFPH       = %d\n", videoInfo.sTimingInfo.wHFront))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HSyncW     = %d\n", videoInfo.sTimingInfo.wHSync))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HBPH       = %d\n", videoInfo.sTimingInfo.wHBack))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HSyncPol   = %d\n", videoInfo.sTimingInfo.cHPol))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VTotal     = %d\n", videoInfo.sTimingInfo.wVTol))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VDEW       = %d\n", videoInfo.sTimingInfo.wVAct))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VFPH       = %d\n", videoInfo.sTimingInfo.wVFront))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VSyncW     = %d\n", videoInfo.sTimingInfo.wVSync))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VBPH       = %d\n", videoInfo.sTimingInfo.wVBack))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "VSyncPol   = %d\n", videoInfo.sTimingInfo.cVPol))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "InterLaced = %d\n", videoInfo.sTimingInfo.cScanType))

        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Asp Ratio   = %d\n", videoInfo.sFormatInfo.eAspectRatio))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ColorDepth  = %d\n", videoInfo.sFormatInfo.eColorDepth))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ColorRange  = %d\n", videoInfo.sFormatInfo.eColorRange))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "ColorSpace  = %d\n", videoInfo.sFormatInfo.eColorSpace))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HDCP Ver    = %d\n", videoInfo.sFormatInfo.cHdcpVersion))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "HDMI Mode   = %d\n", videoInfo.sFormatInfo.cHdmiMode))
        DVCARD_DBG_MSG(8, printf(DVCARD_DBG_PREFIX "Scramble    = %d\n", videoInfo.sFormatInfo.cScrambling))

        modeInfo->Valid = videoInfo.cStable;
        modeInfo->PixClock = videoInfo.sTimingInfo.dwPixClock;    // Pixel clock in Hz

        if(0 != videoInfo.sTimingInfo.wHTol)
        {
            modeInfo->HFreq = videoInfo.sTimingInfo.dwPixClock / videoInfo.sTimingInfo.wHTol ;       // Horizontal frequency in Hz
        }

        if(0 != videoInfo.sTimingInfo.wVTol)
        {
            modeInfo->VFreq = (modeInfo->HFreq * 1000) / videoInfo.sTimingInfo.wVTol;       // Vertical frequency in Hz * 1000
        }

        modeInfo->HTot = videoInfo.sTimingInfo.wHTol;        // Total horizontal pixels
        modeInfo->VTot = videoInfo.sTimingInfo.wVTol;        // Total vertical pixels
        modeInfo->HAct = videoInfo.sTimingInfo.wHAct;        // Active horizontal pixels
        modeInfo->VAct = videoInfo.sTimingInfo.wVAct;        // Acive vertical pixels
        modeInfo->HBack = videoInfo.sTimingInfo.wHBack;       // Horizontal back porch
        modeInfo->VBack = videoInfo.sTimingInfo.wVBack;       // Vertical back porch
        modeInfo->HSync = videoInfo.sTimingInfo.wHSync;       // Horizontal sync width
        modeInfo->VSync = videoInfo.sTimingInfo.wVSync;       // Vertical sync width;
        modeInfo->HPol = videoInfo.sTimingInfo.cHPol;        // Horizontal sync polarity TODO: Correct?
        modeInfo->VPol = videoInfo.sTimingInfo.cVPol;        // Vertical sync polarity TODO: Correct?
        modeInfo->ScanType = videoInfo.sTimingInfo.cScanType;    // PROG_SCAN or INTER_SCAN TODO: Correct?

        modeInfo->AspectRatio = AR_IS_4_3; // Image aspect ratio: Either AR_SQUARE_PIX or (hor<<16 | ver), e.g. AR_IS_4_3
        modeInfo->CSampling = RGB444;   // Color sampling, e.g. RGB444, YUV420, etc. TODO: Conversion!
        modeInfo->CRange = videoInfo.sFormatInfo.eColorRange;      // Color range, i.e. CRNG_FULL, CRNG_LIMITED or CRNG_DEFAULT
        // modeInfo->Colorimetry; // Color standard
        // modeInfo->CEAMode;     // Detected CEA mode (0 = unknown or non-CEA)
        modeInfo->Bpp = videoInfo.sFormatInfo.eColorDepth;         // BPP24, BPP30 or BPP36
        modeInfo->HDCP = videoInfo.sFormatInfo.cHdcpVersion;        // TRUE if HDCP encrypted
        modeInfo->HDMI = videoInfo.sFormatInfo.cHdmiMode;        // TRUE if HDMI mode
        modeInfo->PixPerClk = 0;   // Pixels per clock + 1 (0 = single pixel, 1 = double pixel)
    }
    else
    {
        DVCARD_DBG_MSG(4, printf(DVCARD_DBG_PREFIX "Video_Info Fail\n"))
    }

    return eResult == rcSUCCESS;
}
#endif // 0
