#include "xbeeinit.h"
#include "smac_MC9S08GT60.h"

void xbeeInit(void) {

	// On the GB60, the following do not have pins but are still active on the die and need to be set as outputs.
	PTGDD |= 0xF0;
	PTGD &= ~0xF0;
	PTDDD |= 0xE0;
	PTDD &= ~0xE0;
	PTEDD |= 0xC0;
	PTED &= ~0xC0;
	PTAPE |= 0x2; //Module ID Pin (Low – XBee-PRO, High – XBee)
	PTADD |= 0x10; //VDDA_VREF – A/D Reads referenced from VREF (external) voltage. PTAD &= ~0x10;
	PTADD &= ~0x20; //SLEEP_RQ Pin - input
	PTADD &= ~0x40; //RTS input – input PTAD &= ~0x40;
	PTADD |= 0x80; //unused pin PTAD &= ~0x80;
	PTBDD |= 0x20; //ASSOC pin – output PTBD |= 0x20;
	PTCDD |= 0x01; //unused pins PTCD &= ~0x01; PTCDD |= 0x02; PTCD &= ~0x02;
	PTCDD |= 0x20; //CD – output PTCD &= ~0x20;
	PTCDD |= 0x80; //ON/SLEEP – output PTCD |= 0x80;
	PTDDD |= 0x01; //PWM0/RSSI PTDDD |= 0x02; //PWM1 PTDD &= ~0x01; PTDD &= ~0x02;
	PTADD |= 0x01; //CTS Setup PTAD &= ~0x01; //CTS ready.
	PTEDD &= ~0x02; //Config Setup
	PTEPE |= 0x02;

}
