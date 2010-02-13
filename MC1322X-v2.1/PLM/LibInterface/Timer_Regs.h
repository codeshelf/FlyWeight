// JBW/GW - 15DEC09: Add regs formats for timers.
#ifndef _TIMER_REGS_H_
#define _TIMER_REGS_H_

#include "../Interface/EmbeddedTypes.h"
#include "../LibInterface/Timer.h"

#define TMR0_CTRL_BIT      ((volatile TmrConfigReg_t*)&TMR0_REGS_P->Ctrl)->bitFields
#define TMR0_CTRL_WORD     ((volatile TmrConfigReg_t*)&TMR0_REGS_P->Ctrl)->uintValue
#define TMR0_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR0_REGS_P->StatCtrl)->bitFields
#define TMR0_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR0_REGS_P->StatCtrl)->uintValue
#define TMR0_CSCTRL_BIT    ((volatile TmrComparatorStatusCtrl_t*)&TMR0_REGS_P->CompStatCtrl)->bitFields
#define TMR0_CSCTRL_WORD   ((volatile TmrComparatorStatusCtrl_t*)&TMR0_REGS_P->CompStatCtrl)->uintValue

#define TMR1_CTRL_BIT      ((volatile TmrConfigReg_t*)&TMR1_REGS_P->Ctrl)->bitFields
#define TMR1_CTRL_WORD     ((volatile TmrConfigReg_t*)&TMR1_REGS_P->Ctrl)->uintValue
#define TMR1_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR1_REGS_P->StatCtrl)->bitFields
#define TMR1_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR1_REGS_P->StatCtrl)->uintValue
#define TMR1_CSCTRL_BIT    ((volatile TmrComparatorStatusCtrl_t*)&TMR1_REGS_P->CompStatCtrl)->bitFields
#define TMR1_CSCTRL_WORD   ((volatile TmrComparatorStatusCtrl_t*)&TMR1_REGS_P->CompStatCtrl)->uintValue

#define TMR2_CTRL_BIT      ((volatile TmrConfigReg_t*)&TMR2_REGS_P->Ctrl)->bitFields
#define TMR2_CTRL_WORD     ((volatile TmrConfigReg_t*)&TMR2_REGS_P->Ctrl)->uintValue
#define TMR2_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR2_REGS_P->StatCtrl)->bitFields
#define TMR2_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR2_REGS_P->StatCtrl)->uintValue
#define TMR2_CSCTRL_BIT    ((volatile TmrComparatorStatusCtrl_t*)&TMR2_REGS_P->CompStatCtrl)->bitFields
#define TMR2_CSCTRL_WORD   ((volatile TmrComparatorStatusCtrl_t*)&TMR2_REGS_P->CompStatCtrl)->uintValue

#define TMR3_CTRL_BIT      ((volatile TmrConfigReg_t*)&TMR3_REGS_P->Ctrl)->bitFields
#define TMR3_CTRL_WORD     ((volatile TmrConfigReg_t*)&TMR3_REGS_P->Ctrl)->uintValue
#define TMR3_SCTRL_BIT     ((volatile TmrStatusCtrl_t*)&TMR3_REGS_P->StatCtrl)->bitFields
#define TMR3_SCTRL_WORD    ((volatile TmrStatusCtrl_t*)&TMR3_REGS_P->StatCtrl)->uintValue
#define TMR3_CSCTRL_BIT    ((volatile TmrComparatorStatusCtrl_t*)&TMR3_REGS_P->CompStatCtrl)->bitFields
#define TMR3_CSCTRL_WORD   ((volatile TmrComparatorStatusCtrl_t*)&TMR3_REGS_P->CompStatCtrl)->uintValue

#endif /*_TIMER_REGS_H_ */
