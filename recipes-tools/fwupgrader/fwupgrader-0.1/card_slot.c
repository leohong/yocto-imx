
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "Driver/Bus/types.h"
//#include "errors.h"
//#include "Services/SPD.h"
//#include "Driver/GPIO/GPIO.h"
//#include "Driver/Bus/I2C1.h"
//#include "Driver/FPGA/HDMI.h"
//#include "Driver/Input/SiI9687.h"
//#include "I2CADR.h"
//#include "InOut/Modes.h"
//#include "Driver/Output/AviInfoFrame.h"
//#include "Inputs.h"
//#include "Driver/FPGA/PCI.h"
//#include "Config/pv7/PCIADR.h"
//#include "Main/MainMessages.h"
//#include "main.h"
//#include "Driver/Output/Audio.h"
//#include "Driver/FPGA/Transceiver.h"
//#include "Driver/FPGA/CVI_HDMI.h"
//#include "Driver/FPGA/CVI.h"
//#include "globals.h"
#include "dvCard.h"
#include "card_slot.h"
//#include "Services/SPD.h"

#define CARDSLOT_DBG_LEVEL 0  // 0: Always, 1,2: Error, 3,4: Warning, 5,6: Info, 7,8,9: Debugging
#define CARDSLOT_DBG_PREFIX "card_slot.c > "
#define CARDSLOT_DBG_MSG(level, cmd) if(level <= CARDSLOT_DBG_LEVEL){ cmd; }

#define CARD_IFM_VALID_DELAY   2
#define CARD_IFM_INVALID_DELAY 2

#define RETRY_COUNT (10)

static UBYTE currentInput = 0xFF;
//static AUDIOMODEINFO audioModeInfo;
static sSLOT_INFO m_sSlotInfo;
static BOOL gimPresent = FALSE;
static BOOL opsPresent = FALSE;
static BYTE powerMode = 0x00;
//static MODEINFO modeInfo[CARDSLOT_NUMINPUTS];

static UBYTE ifmValidDelayCnt[CARDSLOT_NUMINPUTS];
static UBYTE ifmInvalidDelayCnt[CARDSLOT_NUMINPUTS];

static char slot1_default_name[32] = {0x00};
static char slot2_default_name[32] = {0x00};
static char slot3_default_name[32] = {0x00};
static char slot4_default_name[32] = {0x00};

void cardSlotReset()
{
    memset(m_sSlotInfo.acInputName, 0x00, sizeof(sSLOT_INFO));

    gimPresent = FALSE;
    opsPresent = FALSE;
}

/*
 * void cardSlotInitModeInfo()
{
    UBYTE i;
    for(i = 0; i < CARDSLOT_NUMINPUTS; i++)
    {
        memset(&modeInfo[i], 0x00, sizeof(MODEINFO));
    }
}
*/

/* ============================================================
a. Read the OSD Input Card Variable.
b. Wait for the card system ready (need time out protection).
c. Read card present.
d. Decide which slot has to Power On by Input Card Variable:
    1.OPS Enable:
        Enable OPS Power.

    2. Slot Enable:
        1. Enable Slot Power.
        2. Wait for Slot Card-Ready (need time out protection).
        3. Read Slot Card Info, it includes the input numbers and input name string.
        4. Read the Slot input and set to Slot Card.
=============================================================*/
int cardSlotInit()
{
    BOOL    ok;
    WORD    cards_present;
    BYTE    ready = 0;
    //SWORD16 selectedCard = INPUT_CARD_SLOT;
    BYTE    cCount = 0;

//    SPDGetCacheParS16(INPUT_CARD_VAR, &selectedCard);

    cardSlotReset();
    //cardSlotInitModeInfo();
    memset(&m_sSlotInfo, 0x00, sizeof(m_sSlotInfo));

    dvCard_Initial();

#if 0
    do
    {
        dvCard_System_Ready_Get(&ready);

        if(0 == ready)
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Card(s) System not Ready\n"));
        }
        else
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Card(s) System Ready\n"));
        }

        usleep(500 * 1000);
        cCount++;
    }
    while((0 == ready) && (cCount < RETRY_COUNT));
#endif // 0

#if 0
    if(TRUE == ready)
    {
        ok = dvCard_System_Card_Present_Get(&cards_present);

        if (ok)
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Card(s) present mask: 0x%x:\n", cards_present));
        }
        else
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Could not read card present mask.\n"));
        }

        if (ok && ((cards_present & OPS_MASK) == OPS_MASK))
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "OPS card is present.\n"));
            opsPresent = TRUE;

            if(selectedCard == INPUT_CARD_OPS)
            {
                ok = dvCard_System_Card_Power_Set(OPS_MASK);

                m_sSlotInfo.cInputs = 1;
                m_sSlotInfo.acInputName[0] = 'O';
                m_sSlotInfo.acInputName[1] = 'P';
                m_sSlotInfo.acInputName[2] = 'S';
                m_sSlotInfo.acInputName[3] = '\0';

                powerMode = OPS_MASK;
            }
        }
        else
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "OPS card not is present!\n"));
            opsPresent = FALSE;
        }

        if (ok && ((cards_present & CARD_MASK) == CARD_MASK))
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Slot card is present.\n"));
            gimPresent = TRUE;

            if(selectedCard == INPUT_CARD_SLOT)
            {
                BYTE cSlotReady = FALSE;
                ok = dvCard_System_Card_Power_Set(CARD_MASK);
                powerMode = CARD_MASK;
                cCount = 0;

                do
                {
                    ok = ok && dvCard_Slot_Ready_Get(&cSlotReady);
                    CARDSLOT_DBG_MSG(5, printf(CARDSLOT_DBG_PREFIX "cSlotReady = %d, cCount = %d\n", cSlotReady, cCount));
                    usleep(500 * 1000);
                }
                while((!cSlotReady) && (cCount++ < RETRY_COUNT));

                if(cSlotReady)
                {
                    ok = ok && dvCard_Slot_Info_Get(&m_sSlotInfo);
                    CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Slot Ready\n"));
                    CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Slot card has %d inputs.\n", m_sSlotInfo.cInputs));
                    CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Slot card Inputs Name is %s\n", m_sSlotInfo.acInputName));
                    ok = ok && dvCard_Input_Port_Set(eSOURCE_LIST_SLOT_0);

                    BOARD_INPUT inputs[4] = {INPUT_SLOT_1, INPUT_SLOT_2, INPUT_SLOT_3, INPUT_SLOT_4};
                    int index = 0;
                    char names[32] = {0x00};
                    char* defaultNames[4] = {slot1_default_name, slot2_default_name, slot3_default_name, slot4_default_name};
                    strcpy(names, m_sSlotInfo.acInputName);
                    char* name = strtok(names, ",");
                    while (name != NULL)
                    {
                        sprintf(defaultNames[index], "SLOT %s", name);
                        INPUT_INDEX i = GetInputIndex(inputs[index]);
                        input_name_var_table[i]->default_value = (void*)defaultNames[index];
                        SPDSetCachePar(input_name_var_table[i]->index, defaultNames[index]);
                        SPDSaveNVPar(input_name_var_table[i]->index);
                        name = strtok(NULL, ",");
                        CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Input Name %d: %s\n", index, defaultNames[index]));
                        index++;
                    }
                }
                else
                {
                    CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Slot Not Ready\n"));
                }
            }
        }
        else
        {
            CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "Slot card not is present!\n"));
            gimPresent = FALSE;
        }
    }

    currentInput = 3;
#endif // 0
    return 1;//OK;
}

#if 0
BOOL cardSlotGimAvailable()
{
    return gimPresent;
}

BOOL cardSlotOpsAvailable()
{
    return opsPresent;
}

BOOL cardSlotGetNumberOfInputs()
{
    return m_sSlotInfo.cInputs;
}

SWITCHRESULT cardSlotSetInput(UBYTE input)
{
    CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "cardSlotSetInput(%d)\n", input))

    if (input >= CARDSLOT_NUMINPUTS)
    {
        CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "cardSlotSetInput: invalid input %d.\n", input))
        return INPUTSWITCH_ERROR;
    }
    if (currentInput == input)
    {
        CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "cardSlotSetInput: input %d already selected.\n", input))
        return INPUTSWITCH_SEAMLESS;
    }

    if (SiI9687SetInput(SII9687_INP_HDMI4) == INPUTSWITCH_ERROR)
    {
        CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "cardSlotSetInput: SiL9687 switch failed.\n"))
        return INPUTSWITCH_ERROR;
    }

    eSOURCE_LIST ePort = eSOURCE_LIST_NUMBERS;

    switch (input)
    {
    case CARDSLOT_OPS:
        ePort = eSOURCE_LIST_OPS_HDMI;
        break;
    case CARDSLOT_1:
        ePort = eSOURCE_LIST_SLOT_0;
        break;
    case CARDSLOT_2:
        ePort = eSOURCE_LIST_SLOT_1;
        break;
    case CARDSLOT_3:
        ePort = eSOURCE_LIST_SLOT_2;
        break;
    case CARDSLOT_4:
        ePort = eSOURCE_LIST_SLOT_3;
        break;
    default:
        break;
        };

    CARDSLOT_DBG_MSG(0, printf(CARDSLOT_DBG_PREFIX "cardSlotSetInput: Select port %d.\n", ePort))

    if (!dvCard_Input_Port_Set(ePort))
        return INPUTSWITCH_ERROR;

    currentInput = input;
    return INPUTSWITCH_MODECHANGE;
}

void cardSlotSetParams(UBYTE input)
{
    UBYTE HDCPEnable = 0;
    SWORD16 HPCPEnable;

    // TODO Do we need this?
    /*
    if (SPDCacheParChanged(SLOT1_IN_HDCP_VAR))
    {
        SPDGetCacheParS16(SLOT1_IN_HDCP_VAR, &HPCPEnable);
        dvCard_Input_Hdcp_Enable(0, HDCPEnable);
    }
    if (SPDCacheParChanged(SLOT2_IN_HDCP_VAR))
    {
        SPDGetCacheParS16(SLOT2_IN_HDCP_VAR, &HPCPEnable);
        dvCard_Input_Hdcp_Enable(1, HDCPEnable);
    }
    if (SPDCacheParChanged(SLOT3_IN_HDCP_VAR))
    {
        SPDGetCacheParS16(SLOT3_IN_HDCP_VAR, &HPCPEnable);
        dvCard_Input_Hdcp_Enable(2, HDCPEnable);
    }
    if (SPDCacheParChanged(SLOT4_IN_HDCP_VAR))
    {
        SPDGetCacheParS16(SLOT4_IN_HDCP_VAR, &HPCPEnable);
        dvCard_Input_Hdcp_Enable(3, HDCPEnable);
    }
    */
}

UBYTE cardSlotCurrentInput()
{
    return currentInput;
}

MODEINFO* cardSlotModeInfoSlot1()
{
    return &modeInfo[0];
}

MODEINFO* cardSlotModeInfoSlot2()
{
    return &modeInfo[1];
}

MODEINFO* cardSlotModeInfoSlot3()
{
    return &modeInfo[2];
}

MODEINFO* cardSlotModeInfoSlot4()
{
    return &modeInfo[3];
}

MODEINFO* cardSlotModeInfoSlotOps()
{
    return &modeInfo[4];
}

void cardSlotIFM()
{
    CARDSLOT_DBG_MSG(9, printf(CARDSLOT_DBG_PREFIX "CardSlotIFM:\n"));
    UBYTE input;
    MODEINFO modeCurrent;
    for(input = 0; input < CARDSLOT_NUMINPUTS; input++)
    {
        memset(&modeCurrent, 0x00, sizeof(modeCurrent));
        /*
        if (!gimPresent && (input <= CARDSLOT_4))
            continue;
        if (input >= gimNumberOfInputs)
            continue;
        if (!opsPresent && (input == CARDSLOT_OPS))
            continue;
        */
        if (input == currentInput)
        {
            // Query slot card or SiL9687?
            MODEINFO* info = SiI9687GetModeInfo3();
            if (info)
                memcpy(&modeCurrent, info, sizeof(modeCurrent));
            //            MODEINFO tmp;
            //            dvCard_Get_Video_Mode(&tmp);
            CARDSLOT_DBG_MSG(9, printf(CARDSLOT_DBG_PREFIX "CardSlotIFM: Input %d is %s.\n", input, modeCurrent.Valid ? "valid" : "invalid"));
            memcpy(&(modeInfo[input]), &modeCurrent, sizeof(modeCurrent));
            /*
            if (modeCurrent.Valid)
            {
                switch (modeCurrent.CSampling)
                {
                case YUV422:
                    printf("card_slot.c: SIL: YUV422, %d BPP\n", modeCurrent.Bpp * 6 + 24);
                    //                    modeCurrent.CSampling = YUV420;
                    break;
                case RGB444:
                    printf("card_slot.c: SIL: RGB444\n");
                    break;
                case YUV444:
                    printf("card_slot.c: SIL: YUV444\n");
                    break;
                case YUV420:
                    printf("card_slot.c: SIL: YUV420\n");
                    break;
                };
            }
            if (tmp.Valid)
            {
                switch (tmp.CSampling)
                {
                case YUV422:
                    printf("card_slot.c: Card: YUV422, %d BPP\n", tmp.Bpp * 6 + 24);
                    break;
                case RGB444:
                    printf("card_slot.c: Card: RGB444, %d BPP\n", tmp.Bpp * 6 + 24);
                    break;
                case YUV444:
                    printf("card_slot.c: Card: YUV444, %d BPP\n", tmp.Bpp * 6 + 24);
                    break;
                case YUV420:
                    printf("card_slot.c: Card: YUV420, %d BPP\n", tmp.Bpp * 6 + 24);
                    break;
                };
            }
            */
        }
        else
        {
            if ((input == CARDSLOT_OPS) && (powerMode == OPS_MASK))
            {
                BOOL hasSignal = FALSE;
                if (dvCard_Input_Has_Signal(input, &hasSignal) && hasSignal)
                    modeCurrent.Valid = TRUE;
            }
            else if (powerMode == CARD_MASK)
            {
                BOOL hasSignal = FALSE;
                if (dvCard_Input_Has_Signal(input, &hasSignal) && hasSignal)
                    modeCurrent.Valid = TRUE;
            }
        }

        if((modeCurrent.Valid          != modeInfo[input].Valid)
           || (modeCurrent.HAct        != modeInfo[input].HAct)
           || (modeCurrent.VAct        != modeInfo[input].VAct)
           || (modeCurrent.HTot        != modeInfo[input].HTot)
           || (modeCurrent.VTot        != modeInfo[input].VTot)
           || (modeCurrent.CSampling   != modeInfo[input].CSampling)
           || (modeCurrent.CRange      != modeInfo[input].CRange)
           || (modeCurrent.Colorimetry != modeInfo[input].Colorimetry)
           || (modeCurrent.CEAMode     != modeInfo[input].CEAMode)
           || (modeCurrent.Bpp         != modeInfo[input].Bpp)
           || (modeCurrent.HDCP        != modeInfo[input].HDCP)
           || (modeCurrent.HDMI        != modeInfo[input].HDMI)
           || (modeCurrent.ScanType    != modeInfo[input].ScanType))
        {
            if (modeCurrent.Valid)
            {
                ifmValidDelayCnt[input]++;
                ifmInvalidDelayCnt[input] = 0;
                if (ifmValidDelayCnt[input] >= CARD_IFM_VALID_DELAY)
                {
                    CARDSLOT_DBG_MSG(9, printf(CARDSLOT_DBG_PREFIX "CardSlotIFM: Input %d has signal.\n", input))
                    memcpy(&(modeInfo[input]), &modeCurrent, sizeof(modeCurrent));
                }
            }
            else
            {
                ifmValidDelayCnt[input] = 0;
                ifmInvalidDelayCnt[input]++;
                if (ifmInvalidDelayCnt[input] >= CARD_IFM_INVALID_DELAY)
                {
                    CARDSLOT_DBG_MSG(6, printf(CARDSLOT_DBG_PREFIX "CardSlotIFM: Input %d has no signal.\n", input))
                    memcpy(&(modeInfo[input]), &modeCurrent, sizeof(modeCurrent));
                }
            }
        }
        else
        {
            ifmValidDelayCnt[input] = 0;
            ifmInvalidDelayCnt[input] = 0;
        }

        if ((input == currentInput) && modeInfo[input].Valid)
        {
            SiI9687GetAudioSources();
            return;
        }
    }
}
#endif // 0
