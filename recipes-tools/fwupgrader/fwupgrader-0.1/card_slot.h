#pragma once
#ifndef GIM_SLOT_H
#define GIM_SLOT_H

//#include "InOut/Modes.h"
//#include "Inputs.h"
//#include "Driver/Output/AudioInfoFrame.h"
//#include "Misc/EDID/EDID_Creator.h"

#define CARDSLOT_NUMINPUTS 5
enum {
    CARDSLOT_1,
    CARDSLOT_2,
    CARDSLOT_3,
    CARDSLOT_4,
    CARDSLOT_OPS,
};

void cardSlotReset();
int cardSlotInit();
// SWITCHRESULT cardSlotSetInput(UBYTE);
// UBYTE cardSlotCurrentInput();
// void cardSlotSetParams(UBYTE);

// MODEINFO* cardSlotModeInfoSlot1();
// MODEINFO* cardSlotModeInfoSlot2();
// MODEINFO* cardSlotModeInfoSlot3();
// MODEINFO* cardSlotModeInfoSlot4();
// MODEINFO* cardSlotModeInfoSlotOps();

// void cardSlotInitModeInfo();
// void cardSlotIFM();

// BOOL cardSlotGimAvailable();
// BOOL cardSlotOpsAvailable();
// BOOL cardSlotGetNumberOfInputs();

#endif  // GIM_SLOT_H
