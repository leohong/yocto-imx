#ifndef UTILHOSTAPI_H
#define UTILHOSTAPI_H
// ==============================================================================
// FILE NAME: UTILHOSTAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 21/11/2013, Leo Create
// --------------------
// ==============================================================================

#include "Driver/Bus/types.h"
//#include "Driver/Output/Audio.h"

#define AUDIO_CHANNEL_STATUS_NUMBERS    (5)
#define INPUT_PORT_NUMBERS              (4)

#pragma pack(push)
#pragma pack(1)

typedef struct
{
    UBYTE    cModule;
    UBYTE    cSubCmd;
    UBYTE    acData[1024 + 32];
} sPAYLOAD;

typedef enum
{
    eACK_TYPE_NONACK,
    eACK_TYPE_ACK,
    eACK_TYPE_BADPACKET,
    eACK_TYPE_TIMEOUT,
    eACK_TYPE_UNKNOWN,
    eACK_TYPE_FEEDBACK,
    eACK_TYPE_ERROR,

    eACK_TYPE_NUMBERS,
} eACK_TYPE;

/*
typedef enum
{
    rcERROR,
    rcSUCCESS,
    rcBUSY,
    rcINVALID,
} eRESULT;
*/

// Cmd table §Î¦¡
typedef struct
{
    WORD wCmdIndex;
    eRESULT(*pFunc)(sPAYLOAD *, sPAYLOAD *, WORD *);
} sCMDCFG;

#if 0
typedef enum
{
    eMODE_TYPE_UK,          // undefined: 0 = mode definition incomplete
    eMODE_TYPE_PALi,        // 50Hz SD   includes SECAM
    eMODE_TYPE_NTSCi,       // 60Hz SD
    eMODE_TYPE_PALp,        // 50Hz SD
    eMODE_TYPE_NTSCp,       // 60Hz SD
    eMODE_TYPE_720p,
    eMODE_TYPE_1080i,
    eMODE_TYPE_1080sfp,     // ie 1080i 48Hz 2:2 cadence Segmented Frame Progressive
    eMODE_TYPE_1080p,
    eMODE_TYPE_2160p,
    eMODE_TYPE_GRAPHIC,     // anything not TV oriented

    eMODE_TYPE_NUMBERS,
} eMODE_TYPE;

typedef enum
{
    ePIX_RPT_1X,
    ePIX_RPT_2X,
    ePIX_RPT_4X,

    ePIX_RPT_NUMBERS,
} ePIX_RPT;

typedef enum
{
    eSYNCPOL_NEG_POL,
    eSYNCPOL_POS_POL,
    eSYNCPOL_NUMBERS
} eSYNCPOL;

typedef enum
{
    eSCANTYPE_PROG_SCAN,
    eSCANTYPE_INTER_SCAN
} eSCANTYPE;

typedef enum
{
    eCOLOR_SAMPLING_RGB444,
    eCOLOR_SAMPLING_YUV422,
    eCOLOR_SAMPLING_YUV444,
    eCOLOR_SAMPLING_YUV420,
    eCOLOR_SAMPLING_NUMBERS
} eCOLOR_SAMPLING;


typedef enum
{
    eCOLOR_RANGE_FULL,
    eCOLOR_RANGE_LIMITED,
    eCOLOR_RANGE_DEFAULT
} eCOLOR_RANGE;

typedef enum
{
    eCOLOR_DEPTH_8,         //color depth 24
    eCOLOR_DEPTH_10,        //color depth 30
    eCOLOR_DEPTH_12,        //color depth 36
    eCOLOR_DEPTH_16,        //color depth 48

    eCOLOR_DEPTH_NUMBERS
} eCOLOR_DEPTH;

typedef enum
{
    eASPECT_RATIO_DEFAULT,
    eASPECT_RATIO_4_3,
    eASPECT_RATIO_16_9,
    eASPECT_RATIO_RESERVED
} eASPECT_RATIO;

typedef struct
{
    BYTE ch_val;
    BYTE cea_val;
} sAUDIOSAMPLESIZE;

//Audio Coding TYPE
enum
{
    AUDIO_TYPE_RESERVED,
    AUDIO_TYPE_NLPCM,
    AUDIO_TYPE_LPCM,
    AUDIO_TYPE_HBR,
    AUDIO_TYPE_DSD,
};

static sAUDIOSAMPLESIZE AudioSampleSizeFromHeader[] =
{
    {0x00, CEA_SS_RESERVED},
    {0x01, CEA_SS_RESERVED},
    {0x02, CEA_SS_16BIT},
    {0x03, CEA_SS_20BIT},
    {0x0A, CEA_SS_20BIT},
    {0x0B, CEA_SS_24BIT}
};

static BYTE CEAAudioFreqToChannelStatusBits[] =
{
    2,  //default CEA_SF_48000
    3,  //CEA_SF_32000
    0,  //CEA_SF_44100
    2,  //CEA_SF_48000
    8,  //CEA_SF_88200
    10, //CEA_SF_96000
    2,  //CEA_SF_176400 => CEA_SF_48000
    14, //CEA_SF_192000

    /*
    CEA_SF_44100,
    CEA_SF_RESERVED,
    CEA_SF_48000,
    CEA_SF_32000,
    CEA_SF_RESERVED,
    CEA_SF_RESERVED,
    CEA_SF_RESERVED,
    CEA_SF_RESERVED,
    CEA_SF_88200,
    CEA_SF_RESERVED,
    CEA_SF_96000,
    CEA_SF_RESERVED,
    CEA_SF_RESERVED,
    CEA_SF_RESERVED,
    CEA_SF_192000
    */
};

typedef struct
{

    DWORD dwPixClock;               // Pixel clock in Hz
    //DWORD ulHFreq;                // Horizontal frequency in Hz
    //DWORD ulVFreq;                // Vertical frequency in Hz * 1000
    WORD wHTol;                     // Total horizontal pixels
    WORD wVTol;                     // Total vertical pixels
    WORD wHAct;                     // Active horizontal pixels
    WORD wVAct;                     // Acive vertical pixels
    WORD wHBack;                    // Horizontal back porch
    WORD wVBack;                    // Vertical back porch
    WORD wHFront;                   // Horizontal Front porch
    WORD wVFront;                   // Vertical Front porch
    WORD wHSync;                    // Horizontal sync width
    WORD wVSync;                    // Vertical sync width;
    BYTE cHPol;                     // Horizontal sync polarity
    BYTE cVPol;                     // Vertical sync polarity
    BYTE cScanType;                 // PROG_SCAN or INTER_SCAN
    //BYTE     ucCEAMode;           // Detected CEA mode (0 = unknown or non-CEA)

} sVIDEO_TIMING;

typedef struct
{
    //---Main Source
    BYTE                cHdmiMode;      // TRUE if HDMI mode or FALSE for DVI mode
    BYTE                cHdcpVersion;   // TRUE if HDCP encrypted
    eCOLOR_SAMPLING     eColorSpace;
    eCOLOR_RANGE        eColorRange;
    eCOLOR_DEPTH        eColorDepth;
    BYTE                cScrambling;
    eASPECT_RATIO       eAspectRatio;
} sVIDEO_FORMAT;

typedef struct
{
    //---Main Source
    BYTE            cStable;        // Input is Stable
    sVIDEO_TIMING   sTimingInfo;
    sVIDEO_FORMAT   sFormatInfo;
} sVIDEO_INFO;

typedef struct
{
    BYTE            cIsAudio;
    BYTE            cAudioType;           // Audio type
    BYTE            cChannelCount;        // Audio channel count
    //BYTE          cSpeakMapping;        // Speaker mapping
    //BYTE          cDownmixLevel;        // Downmix level
    //BYTE          cDownmixPermit;       // Permission to downmix?
    //BYTE          cChannelMask;         // Channel mask
    //BYTE          caChannelStatus[AUDIO_CHANNEL_STATUS_NUMBERS];    // Channel Status bits
    BYTE            cSampleSize;          // Sample size
    BYTE            cSampleFreq;          // Sample frequency
    //BOOL           bMuteSPDIF;            // Mute SPDIF output flag
    //BOOL           bForceSPDIF;           // Force output to use SPDIF as source

} sAUDIO_INFO;
#endif //0

typedef struct
{
    BYTE cVideoInfoChange;
    BYTE cAudioInfoChange;
    BYTE cPortStatusChange;

} sSYSTEM_STATUS;

#define MAX_MSG_PKT     (8)
#define MSG_RETRY       (5)
// #define MAX_PACKET_SIZE (1024+16)
#define MAX_PACKET_SIZE (sizeof(sPAYLOAD)/sizeof(BYTE))
    
#define HEADERSIZE      (sizeof(sMSG_PACKET_HEADER)/sizeof(BYTE))
#define MAGICNUMBERSIZE (2)
#define MAGICNUMBER1    (0x55)
#define MAGICNUMBER2    (0x55)

#define HEAD_PACK_SIZE  (MAGICNUMBERSIZE + HEADERSIZE)

typedef enum
{
    eMSG_TYPE_UART,
	eMSG_TYPE_FOTA,

    eMSG_TYPE_NUMBERS,
} eMSG_TYPE;

typedef enum
{
    eDEVICE_APP = 0,
    eDEVICE_BOOTCODE,
    eDEVICE_FOTA,       // RK3399
    eDEVICE_HOST_NUMBERS,
    
    eDEVICE_DOCKING_BOARD = 0,
    eDEVICE_GIM,

    eDEVICE_SLOT_NUMBERS,
} eDEVICE;

typedef enum
{
    ePAYLOAD_TYPE_EXE_WRITE,
    ePAYLOAD_TYPE_EXE_READ,
    ePAYLOAD_TYPE_REPLY,
    ePAYLOAD_TYPE_ACK,

    ePAYLOAD_TYPE_NUMBERS,
} ePAYLOAD_TYPE;

typedef enum
{
    eMSG_STATE_MAGIC_NUMBER,
    eMSG_STATE_PACKET_HEADER,
    eMSG_STATE_PACKET_PAYLOAD,
    eMSG_STATE_DATA_READY,
    eMSG_STATE_BAD_PACKET,
    eMSG_STATE_TIMEOUT,
    eMSG_STATE_RUN_OUT_OF_MEMORY,
    eMSG_STATE_INITIAL_ERROR,

    eMSG_STATE_NUMBERS,
} eMSG_STATE;

typedef struct
{
    UBYTE    cSource;
    UBYTE    cDestination;
    UBYTE    cPacketType;
    WORD    wSeqId;
    WORD    wPacketSize;
    WORD    wChecksum;
} sMSG_PACKET_HEADER;

typedef struct
{
    UBYTE                cMagicNumber1;
    UBYTE                cMagicNumber2;
    sMSG_PACKET_HEADER  sPacketHeader;

    union
    {
        sPAYLOAD        sPayLoad;
        UBYTE            acPacketPayload[sizeof(sPAYLOAD)];
    }uFormat;

} sMSG_PACKET_FORMAT;

typedef eRESULT (*fpWRITE)(WORD wSize, UBYTE *pcData);
typedef eRESULT (*fpREAD)(WORD wSize, UBYTE *pcData);

typedef struct
{
    sMSG_PACKET_FORMAT  sMsgPacket;
    eMSG_STATE          eMsgParsingState;
    WORD                wRecivedByteCount;
    WORD                wRecivedByteCRC;
    fpWRITE             fpWriteFunc;
    fpREAD              fpReadFunc;

} sMSG_STATE_DATA;

#pragma pack(pop)   /* restore original alignment from stack */

void utilHost_Init(void);
void utilHost_Process(void);
void utilHost_PacketBuild(sMSG_PACKET_FORMAT *psPacket,
                          eDEVICE eSource,
                          eDEVICE eDestination,
                          ePAYLOAD_TYPE ePayloadType,
                          WORD wSeqId,
                          WORD wDataSize,
                          sPAYLOAD *psPayload);
eMSG_STATE utilHost_StateProcess(sMSG_STATE_DATA *psMsgData, DWORD dwMilliSecond);

#endif //UTILHOSTPROCAPI_H


