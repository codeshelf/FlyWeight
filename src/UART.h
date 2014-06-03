#ifndef __UART
#define __UART

//#include "Cpu.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"

#if defined(MC13192EVB)
	#define	SCID			SCI2D
	#define	SCIBDH			SCI2BDH
	#define	SCIBDL			SCI2BDL
	#define	SCIS1			SCI2S1
	#define	SCIS1_RDRF		SCI2S1_RDRF
	#define	SCIS1_OR_MASK	SCI2S1_OR_MASK
	#define	SCIS1_NF_MASK	SCI2S1_NF_MASK
	#define	SCIS1_FE_MASK	SCI2S1_FE_MASK
	#define	SCIS1_PF_MASK	SCI2S1_PF_MASK
	#define	SCIC1			SCI2C1
	#define	SCIC2			SCI2C2
	#define	SCIC2_TIE		SCI2C2_TIE
	#define	SCIC2_TE_MASK	SCI2C2_TE_MASK
	#define	SCIC2_RE_MASK	SCI2C2_RE_MASK
	#define	SCIC3			SCI2C3
	#define RTS				PTAD_PTAD7
	#define RTS_SETUP		PTAPE_PTAPE7 = 0; PTADD_PTADD7 = 0;
	#define CTS_ON  		PTAD_PTAD6 = 0;
	#define CTS_OFF  		PTAD_PTAD6 = 1;
	#define CTS_SETUP      	PTAPE_PTAPE6 = 0; PTADD_PTADD6 = 1;
#else
	#define	SCID			SCI1D
	#define	SCIBDH			SCI1BDH
	#define	SCIBDL			SCI1BDL
	#define	SCIS1			SCI1S1
	#define	SCIS1_RDRF		SCI1S1_RDRF
	#define	SCIS1_OR_MASK	SCI1S1_OR_MASK
	#define	SCIS1_NF_MASK	SCI1S1_NF_MASK
	#define	SCIS1_FE_MASK	SCI1S1_FE_MASK
	#define	SCIS1_PF_MASK	SCI1S1_PF_MASK
	#define	SCIC1			SCI1C1
	#define	SCIC2			SCI1C2
	#define	SCIC2_TIE		SCI1C2_TIE
	#define	SCIC2_TE_MASK	SCI1C2_TE_MASK
	#define	SCIC2_RE_MASK	SCI1C2_RE_MASK
	#define	SCIC3			SCI1C3
	#define RTS				PTAD_PTAD7
	#define RTS_SETUP		PTAPE_PTAPE7 = 0; PTADD_PTADD7 = 0;
	#define CTS_ON  		PTAD_PTAD6 = 0;
	#define CTS_OFF  		PTAD_PTAD6 = 1;
	#define CTS_SETUP      	PTAPE_PTAPE6 = 0; PTADD_PTADD6 = 1;
#endif


#ifndef __BWUserType_tItem
#define __BWUserType_tItem
  typedef struct {                     /* Item of the index table for possible baudrates */
	gwUINT16 div;                          /* divisor */
	gwUINT8 val;                          /* values of the prescalers */
  } tItem;
#endif
#ifndef __BWUserType_UART_TError
#define __BWUserType_UART_TError
  typedef union {
    gwUINT8 err;
    struct {
      gwBoolean OverRun  : 1;               /* Overrun error flag */
      gwBoolean Framing  : 1;               /* Framing error flag */
      gwBoolean Parity   : 1;               /* Parity error flag */
      gwBoolean RxBufOvf : 1;               /* Rx buffer full error flag */
      gwBoolean Noise    : 1;               /* Noise error flag */
      gwBoolean Break    : 1;               /* Break detect */
      gwBoolean LINSync  : 1;               /* LIN synchronization error */
    } errName;
  } UART_TError;                        /* Error flags. For languages which don't support bit access is byte access only to error flags possible. */
#endif

#ifndef __BWUserType_U_TComData
#define __BWUserType_UART_TComData
  typedef gwUINT8 UART_TComData ;          /* User type for communication. Size of this type depends on the communication data witdh. */
#endif

#define UART_INP_BUF_SIZE 150           /* Input buffer size */
#define UART_OUT_BUF_SIZE 5             /* Output buffer size */
#define UART_RTS_BUF_SIZE 130           /* Number of characters in rcv. buffer when RTS signal gets activated */

#define UART_GetCharsInRxBuf() (UART_InpLen)                           /* Return number of chars in receive buffer */

extern gwUINT8 UART_OutLen;                /* Length of the output buffer content */
extern gwUINT8 UART_InpLen;                /* Length of the input buffer content */

void getFromRXBuffer(void);

void UART_Init(gwUINT8 uartNum, gwUINT32 baudrate, gwBoolean enableFlowControl);
void UART_Close(gwUINT8 uartNum);
void UART_ReadOneChar(gwUINT8 portNum, UART_TComData *outDataPtr);
gwUINT8 UART_SendChar(gwUINT8 portNum, UART_TComData Chr);

//void UART_SetHigh(void);
//void UART_SetSlow(void);
//void UART_CheckInterface();
//gwUINT8 UART_RecvChar(gwUINT8 portNum, UART_TComData *Chr);
//gwUINT8 UART_RecvBlock(UART_TComData *Ptr,gwUINT16 Size,gwUINT16 *Rcv);
//gwUINT8 UART_SendBlock(UART_TComData * Ptr,gwUINT16 Size,gwUINT16 *Snd);
//gwUINT8 UART_GetError(UART_TError *Err);
//gwISR UART_InterruptRx(void);
//gwISR UART_InterruptTx(void);
//gwISR UART_InterruptError(void);

#endif // __UART
