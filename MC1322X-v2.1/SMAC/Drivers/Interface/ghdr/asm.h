#ifndef _ASM_H_
#define _ASM_H_

#include "../../../../PLM/Interface/EmbeddedTypes.h"


#define ASM_BASE_ADD (0x80008000)

#define asm_key0          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x00)))
#define asm_key1          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x04)))
#define asm_key2          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x08)))
#define asm_key3          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x0C)))
#define asm_data0         (*((volatile uint32_t *)(ASM_BASE_ADD + 0x10)))
#define asm_data1         (*((volatile uint32_t *)(ASM_BASE_ADD + 0x14)))
#define asm_data2         (*((volatile uint32_t *)(ASM_BASE_ADD + 0x18)))
#define asm_data3         (*((volatile uint32_t *)(ASM_BASE_ADD + 0x1C)))
#define asm_ctr0          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x20)))
#define asm_ctr1          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x24)))
#define asm_ctr2          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x28)))
#define asm_ctr3          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x2C)))
#define asm_ctr_result0   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x30)))
#define asm_ctr_result1   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x34)))
#define asm_ctr_result2   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x38)))
#define asm_ctr_result3   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x3C)))
#define asm_cbc_result0   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x40)))
#define asm_cbc_result1   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x44)))
#define asm_cbc_result2   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x48)))
#define asm_cbc_result3   (*((volatile uint32_t *)(ASM_BASE_ADD + 0x4C)))
#define asm_control0      (*((volatile uint32_t *)(ASM_BASE_ADD + 0x50)))
#define asm_control1      (*((volatile uint32_t *)(ASM_BASE_ADD + 0x54)))
#define asm_status        (*((volatile uint32_t *)(ASM_BASE_ADD + 0x58)))
#define asm_mac0          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x60)))
#define asm_mac1          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x64)))
#define asm_mac2          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x68)))
#define asm_mac3          (*((volatile uint32_t *)(ASM_BASE_ADD + 0x6C)))


#endif
