// JBW/GW - 15DEC09: Add regs formats for timers.
#ifndef _TIMER_REGS_H_
#define _TIMER_REGS_H_

#include "../Interface/EmbeddedTypes.h"
#include "../LibInterface/Timer.h"

#define TMR0_CTRL_BIT      ((volatile TmrConfig_t*)&TMR0_REGS_P->Ctrl)->bitFields
#define TMR0_CTRL_WORD     ((volatile TmrConfig_t*)&TMR0_REGS_P->Ctrl)->uintValue
#define TMR0_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR0_REGS_P->StatCtrl)->bitFields
#define TMR0_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR0_REGS_P->StatCtrl)->uintValue

#define TMR1_CTRL_BIT      ((volatile TmrConfig_t*)&TMR1_REGS_P->Ctrl)->bitFields
#define TMR1_CTRL_WORD     ((volatile TmrConfig_t*)&TMR1_REGS_P->Ctrl)->uintValue
#define TMR1_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR1_REGS_P->StatCtrl)->bitFields
#define TMR1_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR1_REGS_P->StatCtrl)->uintValue

#define TMR2_CTRL_BIT      ((volatile TmrConfig_t*)&TMR2_REGS_P->Ctrl)->bitFields
#define TMR2_CTRL_WORD     ((volatile TmrConfig_t*)&TMR2_REGS_P->Ctrl)->uintValue
#define TMR2_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR2_REGS_P->StatCtrl)->bitFields
#define TMR2_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR2_REGS_P->StatCtrl)->uintValue

#define TMR3_CTRL_BIT      ((volatile TmrConfig_t*)&TMR3_REGS_P->Ctrl)->bitFields
#define TMR3_CTRL_WORD     ((volatile TmrConfig_t*)&TMR3_REGS_P->Ctrl)->uintValue
#define TMR3_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR3_REGS_P->StatCtrl)->bitFields
#define TMR3_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR3_REGS_P->StatCtrl)->uintValue

#endif /*_TIMER_REGS_H_ */
