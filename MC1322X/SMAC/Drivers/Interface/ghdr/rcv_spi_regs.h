
#ifndef _RCV_SPI_REGS_H_
#define _RCV_SPI_REGS_H_


#define lna_state_mask (0x00000001 << 11)
#define ifa_state_mask (0x0000000F << 12)

typedef union AGC_Output_Regs_SFD_Detect_Tag {
  struct
  {
    uint32_t  rx_pwr_est_sfd_reg:10;
    uint32_t  NOT_USED_1:1;
    uint32_t  lna_state_sfd_reg:1;
    uint32_t  ifa_state_sfd_reg:4;
    uint32_t  rx_dagc_gain_stat:9;
    uint32_t  NOT_USED_0:7;
  } Bits;
  uint32_t Reg;
}AGC_Output_Regs_SFD_Detect_T;


#define rcv_spi_base (0x80009400)


#define rx_corr_ctrl_adr    (*((volatile uint32_t *)(rcv_spi_base + 0x00)))

#define rx_agc_out_adr      (*((volatile uint32_t *)(rcv_spi_base + 0x90)))
#define rx_agc_out_sfd_adr  (*((volatile uint32_t *)(rcv_spi_base + 0x94)))





#endif

