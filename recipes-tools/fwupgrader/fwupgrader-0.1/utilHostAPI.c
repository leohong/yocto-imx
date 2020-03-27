// ==============================================================================
// FILE NAME: UTILHOST.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================


// #include "utilCounterAPI.h"
// #include "utilLogAPI.h"
// #include "utilCLICmdAPI.h"
#include "stdio.h"
#include "string.h"

#include "utilHostAPI.h"

// 解封包狀態機資訊
static sMSG_STATE_DATA m_sMsgState;  // SM of Packet Type Data

// task stack - start -
// static StaticTask_t m_xHostTaskTCB;
// static StackType_t m_axHostStack[EVENT_TASK_STACK_SZ];
// static TaskHandle_t xHost_Handle = NULL;
// task stack - end -

// ==============================================================================
// FUNCTION NAME: UTILHOST_STATERESET
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
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_StateReset(sMSG_STATE_DATA *psMsgData)
{
    memset(&psMsgData->sMsgPacket, 0, sizeof(sMSG_PACKET_FORMAT) / sizeof(BYTE));
    psMsgData->eMsgParsingState = eMSG_STATE_MAGIC_NUMBER;
    psMsgData->wRecivedByteCount = 0;
    psMsgData->wRecivedByteCRC = 0;
}

// ==============================================================================
// FUNCTION NAME: utilHost_Package_Print
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
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_Package_Print(sMSG_STATE_DATA *psMsgData)
{
    WORD wCount = 0;
    BYTE *pcBuffer = (BYTE *) (&psMsgData->sMsgPacket);

    printf("MSK_LIST_HOST, \n");
    for (wCount = 0; wCount <= (HEAD_PACK_SIZE + psMsgData->sMsgPacket.sPacketHeader.wPacketSize); wCount++) {
        printf("0x%02X ", *(pcBuffer + wCount));
    }

    (void) pcBuffer;

    printf("MSK_LIST_HOST, \n");
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_STATEPROCESS
// DESCRIPTION:
//
// |---------------------------------------------------------------------------|
// |Magic Number|                       Packet Header           |Packet Payload|
// |---------------------------------------------------------------------------|
// |            |Packet Type|Packet Seq |Packet Size|Packet CRC |   Data       |
// |---------------------------------------------------------------------------|
// |  2 bytes   |   1 byte  |   1 byte  |   2 bytes |   2 bytes |   N bytes    |
// |---------------------------------------------------------------------------|
//
// Params:
// sMSG_PACKET_FORMAT *psMsgPacket:

//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
eMSG_STATE utilHost_StateProcess(sMSG_STATE_DATA *psMsgData, D_WORD dwMilliSecond)
{
    BYTE cReadBuffer = 0;

    if (NULL == psMsgData->fpReadFunc) {
        // vTaskSuspend(NULL);
        return eMSG_STATE_INITIAL_ERROR;
    }

    if (rcSUCCESS == psMsgData->fpReadFunc(1, &cReadBuffer)) {
        // printf("MSK_LIST_HOST, 0x%02X ", cReadBuffer);

        switch (psMsgData->eMsgParsingState) {
        case eMSG_STATE_MAGIC_NUMBER: {
            psMsgData->sMsgPacket.cMagicNumber2 = cReadBuffer;

            if ((MAGICNUMBER1 == psMsgData->sMsgPacket.cMagicNumber1) && (MAGICNUMBER2 == psMsgData->sMsgPacket.cMagicNumber2)) {
                psMsgData->eMsgParsingState = eMSG_STATE_PACKET_HEADER;
                psMsgData->wRecivedByteCount = 0;
                psMsgData->wRecivedByteCRC = 0;
                // printf("\n\n");
            } else {
                psMsgData->sMsgPacket.cMagicNumber1 = psMsgData->sMsgPacket.cMagicNumber2;
            }
        } break;

        case eMSG_STATE_PACKET_HEADER: {
            BYTE *pcBuffer = (BYTE *) &psMsgData->sMsgPacket.sPacketHeader;
            *(pcBuffer + psMsgData->wRecivedByteCount++) = cReadBuffer;

            // printf("0x%02X ", cReadBuffer);

            if (HEADERSIZE == psMsgData->wRecivedByteCount) {
                if (MAX_PACKET_SIZE >= psMsgData->sMsgPacket.sPacketHeader.wPacketSize) {
                    psMsgData->wRecivedByteCount = 0;
                    psMsgData->eMsgParsingState = eMSG_STATE_PACKET_PAYLOAD;

                    // 計算CRC
                    psMsgData->wRecivedByteCRC += psMsgData->sMsgPacket.sPacketHeader.cSource;
                    psMsgData->wRecivedByteCRC += psMsgData->sMsgPacket.sPacketHeader.cDestination;
                    psMsgData->wRecivedByteCRC += psMsgData->sMsgPacket.sPacketHeader.cPacketType;
                    psMsgData->wRecivedByteCRC += ((psMsgData->sMsgPacket.sPacketHeader.wSeqId >> 8) & 0x00FF);
                    psMsgData->wRecivedByteCRC += (psMsgData->sMsgPacket.sPacketHeader.wSeqId & 0x00FF);
                    psMsgData->wRecivedByteCRC += ((psMsgData->sMsgPacket.sPacketHeader.wPacketSize >> 8) & 0x00FF);
                    psMsgData->wRecivedByteCRC += (psMsgData->sMsgPacket.sPacketHeader.wPacketSize & 0x00FF);
                    // 不帶Payload資料,檢查CRC
                    if (0 == psMsgData->sMsgPacket.sPacketHeader.wPacketSize) {
                        if (0 == (0xFFFF & (psMsgData->sMsgPacket.sPacketHeader.wChecksum + psMsgData->wRecivedByteCRC))) {
                            psMsgData->eMsgParsingState = eMSG_STATE_DATA_READY;
                        } else {
                            printf("CRC ERROR\n");
                            // utilHost_Package_Print(psMsgData);
                            psMsgData->eMsgParsingState = eMSG_STATE_BAD_PACKET;
                        }

                        return psMsgData->eMsgParsingState;
                    }
                } else {
                    printf("OVER SIZE\n");
                    // utilHost_Package_Print(psMsgData);
                    psMsgData->eMsgParsingState = eMSG_STATE_BAD_PACKET;
                    return eMSG_STATE_BAD_PACKET;
                }
            }
        } break;

        case eMSG_STATE_PACKET_PAYLOAD: {
            psMsgData->wRecivedByteCRC += cReadBuffer;
            psMsgData->sMsgPacket.uFormat.acPacketPayload[psMsgData->wRecivedByteCount++] = cReadBuffer;

            if (psMsgData->sMsgPacket.sPacketHeader.wPacketSize == psMsgData->wRecivedByteCount) {
                if (0 == (0xFFFF & (psMsgData->sMsgPacket.sPacketHeader.wChecksum + psMsgData->wRecivedByteCRC))) {
                    psMsgData->eMsgParsingState = eMSG_STATE_DATA_READY;
                    // printf("\n");
                } else {
                    printf("CRC ERROR\n");
                    // utilHost_Package_Print(psMsgData);
                    psMsgData->eMsgParsingState = eMSG_STATE_BAD_PACKET;
                }

                return psMsgData->eMsgParsingState;
            }
        } break;

        default:
            return psMsgData->eMsgParsingState;
            // break;
        }
    } else {
        printf("MSK_LIST_HOST, Read failed!\n");
        psMsgData->eMsgParsingState = eMSG_STATE_BAD_PACKET;
    }

    return psMsgData->eMsgParsingState;
}

// ==============================================================================
// FUNCTION NAME: UTILHOST_CHECKSUMCALC
// DESCRIPTION:
//
//
// Params:
// sMSG_PACKET_FORMAT *psPacket:
// WORD wSize:
// BYTE *pcBuffer:
//
// Returns:
//
//
// modification history
// --------------------
// 15/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_CheckSumCalc(sMSG_PACKET_FORMAT *psPacket)
{
    WORD wCount = 0;
    WORD wChecksum = 0;

    wChecksum = 0;
    wChecksum += psPacket->sPacketHeader.cSource;
    wChecksum += psPacket->sPacketHeader.cDestination;
    wChecksum += psPacket->sPacketHeader.cPacketType;
    wChecksum += ((psPacket->sPacketHeader.wSeqId >> 8) & 0x00FF);
    wChecksum += (psPacket->sPacketHeader.wSeqId & 0x00FF);
    wChecksum += ((psPacket->sPacketHeader.wPacketSize >> 8) & 0x00FF);
    wChecksum += (psPacket->sPacketHeader.wPacketSize & 0x00FF);

    for (wCount = 0; wCount < psPacket->sPacketHeader.wPacketSize; wCount++) {
        wChecksum += psPacket->uFormat.acPacketPayload[wCount];
    }

    psPacket->sPacketHeader.wChecksum = ((~wChecksum + 1) & 0xFFFF);
}

// ==============================================================================
// FUNCTION NAME: utilHost_Ack_Build
// DESCRIPTION:
//
// |---------------------------------------------------------------------------|
// |Magic Number|                       Packet Header           |   ACK        |
// |---------------------------------------------------------------------------|
// |            |Packet Type|Packet Seq |Packet Size|Packet CRC |   Data       |
// |---------------------------------------------------------------------------|
// |  2 bytes   |   1 byte  |   1 byte  |   2 bytes |   2 bytes |   1 bytes    |
// |---------------------------------------------------------------------------|
//
// Ack 封包大小 = Header Size + 1 Byte Ack
//
// Params:
// eMSG_TYPE eMsgType:
// eACK_TYPE eAckType:
//
// Returns:
//
//
// modification history
// --------------------
// 15/07/2013, Leo written
// --------------------
// ==============================================================================
static void utilHost_Ack_Build(sMSG_PACKET_FORMAT *psPacket,
                               eDEVICE eSource,
                               eDEVICE eDestination,
                               eACK_TYPE eAckType)
{
    psPacket->cMagicNumber1 = MAGICNUMBER1;
    psPacket->cMagicNumber2 = MAGICNUMBER2;
    psPacket->sPacketHeader.cSource = eSource;
    psPacket->sPacketHeader.cDestination = eDestination;
    psPacket->sPacketHeader.cPacketType = ePAYLOAD_TYPE_ACK;
    psPacket->sPacketHeader.wPacketSize = 1;
    psPacket->uFormat.acPacketPayload[0] = eAckType;
    utilHost_CheckSumCalc(psPacket);
}

// ==============================================================================
// FUNCTION NAME: utilHost_PacketBuild
// DESCRIPTION:
//
//
// Params:
// sMSG_PACKET_FORMAT *psPacket:
// eDEVICE eSource:
// eDEVICE eDestination:
// ePAYLOAD_TYPE ePayloadType:
// WORD wSeqId:
// WORD wDataSize:
// sPAYLOAD *psPayload:
//
// Returns:
//
//
// modification history
// --------------------
// 12/07/2013, Leo written
// --------------------
// ==============================================================================
void utilHost_PacketBuild(sMSG_PACKET_FORMAT *psPacket,
                          eDEVICE eSource,
                          eDEVICE eDestination,
                          ePAYLOAD_TYPE ePayloadType,
                          WORD wSeqId,
                          WORD wDataSize,
                          sPAYLOAD *psPayload)
{
    psPacket->cMagicNumber1 = MAGICNUMBER1;
    psPacket->cMagicNumber2 = MAGICNUMBER2;
    psPacket->sPacketHeader.cSource = eSource;
    psPacket->sPacketHeader.cDestination = eDestination;
    psPacket->sPacketHeader.cPacketType = ePayloadType;
    psPacket->sPacketHeader.wSeqId = wSeqId;
    psPacket->sPacketHeader.wPacketSize = wDataSize;
    psPacket->uFormat.sPayLoad = *psPayload;

    utilHost_CheckSumCalc(psPacket);
}
