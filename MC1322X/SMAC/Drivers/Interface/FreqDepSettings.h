#ifndef _FreqDepSettings_H
#define _FreqDepSettings_H

/* BEGIN Custom Clock Frequency Settings */
#define gCrystalFrequency_c    24000000
#define gSystemClock24MHz_c    TRUE
#define gDigitalClock_PN_c     24
#define gDigitalClock_RN_c     21
#define gMACA_Clock_DIV_c      95
#define gDigitalClock_RAFC_c   0

#define gaRFSynVCODivIVal_c      \
{\
     0x2f,\
     0x2f,\
     0x2f,\
     0x2f,\
     0x2f,\
     0x2f,\
     0x2f,\
     0x2f,\
     0x2f,\
     0x30,\
     0x30,\
     0x30,\
     0x30,\
     0x30,\
     0x30,\
     0x30\
}

#define gaRFSynVCODivFVal_c      \
{\
    0x00355555,\
    0x006aaaaa,\
    0x00a00000,\
    0x00d55555,\
    0x010aaaaa,\
    0x01400000,\
    0x01755555,\
    0x01aaaaaa,\
    0x01e00000,\
    0x00155555,\
    0x004aaaaa,\
    0x00800000,\
    0x00b55555,\
    0x00eaaaaa,\
    0x01200000,\
    0x01555555\
}
/* END Custom Clock Frequency Settings */



#endif /* _FreqDepCalc_H */
