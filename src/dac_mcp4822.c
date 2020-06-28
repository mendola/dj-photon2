#include <ch.h>
#include <hal.h>

#include "dac_mcp4822.h"
#include "board.h"

#define DAC_PIN_CS      GPIO_PB12_DAC_CS
#define DAC_PIN_SDI     GPIO_PB15_DAC_MOSI
#define DAC_PIN_SCK     GPIO_PB13_DAC_SCK
#define DAC_PIN_LDAC    GPIO_PB14_DAC_MISO

#define SET_DAC_CS()      palSetPad(GPIOB, GPIO_PB12_DAC_CS)
#define CLEAR_DAC_CS()    palClearPad(GPIOB, GPIO_PB12_DAC_CS)
#define SET_DAC_SDI()     palSetPad(GPIOB, GPIO_PB15_DAC_MOSI)
#define CLEAR_DAC_SDI()   palClearPad(GPIOB, GPIO_PB15_DAC_MOSI)
#define SET_DAC_SCK()     palSetPad(GPIOB, GPIO_PB13_DAC_SCK)
#define CLEAR_DAC_SCK()   palClearPad(GPIOB, GPIO_PB13_DAC_SCK)
#define SET_DAC_LDAC()    palSetPad(GPIOB, GPIO_PB14_DAC_MISO)
#define CLEAR_DAC_LDAC()  palClearPad(GPIOB, GPIO_PB14_DAC_MISO)

#define NORMALIZED_TO_12BIT_FACTOR ((float)((1<<11) - 1)) // Convert [0,2.0] -> [0,0xFFF]

// SPI_CR1 Settings
#define SPI_CR1_CLOCK_PHASE_BIT  (0 << 0) // The first clock transition is the first data capture edge
#define SPI_CR1_CLOCK_POLARITY   (0 << 1) // CK to 0 when idle
#define SPI_CR1_MASTER_SELECTION (1 << 2) // Master configuration
#define SPI_CR1_BAUD_RATE_CONFIG (0b000 << 3) // fpclk/2
#define SPI_CR1_SPI_ENABLE       (0 << 6) // SPI Disabled (turned on by driver later)
#define SPI_CR1_FRAME_FORMAT     (0 << 7) // MSB-first
#define SPI_CR1_INT_SS           (0 << 8) // Internal slave select
#define SPI_CR1_SOFT_SLAVE_MGMT  (0 << 9) // Not enabled
#define SPI_CR1_RX_ONLY          (0 << 10) // Tx and Rx
#define SPI_CR1_DAT_FRAME_FMT    (0 << 11) // 16-bit frame
#define SPI_CR1_CRC_TX_NEXT      (0 << 12) // No CRC phase
#define SPI_CR1_HARDWARE_CRC     (0 << 13) // Disabled
#define SPI_CR1_OUTPUT_ENBL      (0 << 14) // Output enabled
#define SPI_CR1_BIDIR_MODE       (0 << 15) // 1-Line mode
#define SPI_CR1_CONFIG  (SPI_CR1_CLOCK_PHASE_BIT \
                        | SPI_CR1_CLOCK_POLARITY \
                        | SPI_CR1_MASTER_SELECTION \
                        | SPI_CR1_BAUD_RATE_CONFIG \
                        | SPI_CR1_SPI_ENABLE \
                        | SPI_CR1_FRAME_FORMAT \
                        | SPI_CR1_INT_SS \
                        | SPI_CR1_SOFT_SLAVE_MGMT \
                        | SPI_CR1_RX_ONLY \
                        | SPI_CR1_DAT_FRAME_FMT \
                        | SPI_CR1_CRC_TX_NEXT \
                        | SPI_CR1_HARDWARE_CRC \
                        | SPI_CR1_OUTPUT_ENBL \
                        | SPI_CR1_BIDIR_MODE)

// SPI_CR2 Settings
#define SPI_CR2_RX_DMA_EN           (0 << 0) // Disabled
#define SPI_CR2_TX_DMA_EN           (0 << 1) // Disabled
#define SPI_CR2_SS_OUT_EN           (0 << 2) // Disabled
#define SPI_CR2_RESERVED            (0b00 << 3) // Must be 0
#define SPI_CR2_ERR_INT_EN          (0 << 5) // Disabled
#define SPI_CR2_RX_N_EMPTY_INT_EN   (0 << 6) // Disabled
#define SPI_CR2_TX_BUF_EMPTY_INT_EN (0 << 7) // Disabled
#define SPI_CR2_CONFIG    (SPI_CR2_RX_DMA_EN \
                          | SPI_CR2_TX_DMA_EN \
                          | SPI_CR2_SS_OUT_EN \
                          | SPI_CR2_RESERVED \
                          | SPI_CR2_ERR_INT_EN \
                          | SPI_CR2_RX_N_EMPTY_INT_EN \
                          | SPI_CR2_TX_BUF_EMPTY_INT_EN)


/*
 * Maximum speed SPI configuration (18MHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig hs_spicfg = {
  false,
  NULL,
  GPIOB,
  DAC_PIN_CS,
  SPI_CR1_CONFIG,
  SPI_CR2_CONFIG
};
// static const SPIConfig hs_spicfg = {
//   false,
//   NULL,
//   GPIOB,
//   DAC_PIN_CS,
//   0,
//   0
// };

/*
 * Red LEDs blinker thread, times are in milliseconds.
 */
// static THD_WORKING_AREA(waDacThread, 128);
// static THD_FUNCTION(DacThreadFn, arg) {

//   (void)arg;

//   chRegSetThreadName("dac_thread");
//   while (true) {
//     palClearPad(IOPORT3, GPIOC_LED);
//     chThdSleepMilliseconds(500);
//     palSetPad(IOPORT3, GPIOC_LED);
//     chThdSleepMilliseconds(500);
//   }
// }

void InitDac(void) {
  /*
   * SPI1 I/O pins setup.
   */
  palSetPadMode(GPIOB, 13, PAL_MODE_OUTPUT_PUSHPULL);     /* SCK. */
  palSetPadMode(GPIOB, 14, PAL_MODE_OUTPUT_PUSHPULL);     /* MISO.*/
  palSetPadMode(GPIOB, 15, PAL_MODE_OUTPUT_PUSHPULL);     /* MOSI.*/
  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOB, 12);

  // Make sure pins are set up right in board.h
  //spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */

  //spiStart(&SPID2, &hs_spicfg);       /* Setup transfer parameters.       */
  //spiReleaseBus(&SPID2);              /* Ownership release.  */
}

static uint16_t MakeCommandPacket(uint16_t value, const bool is_left) {
  // if (value < -1.0) {
  //   value = -1.0;
  // } else if (value > 1.0) {
  //   value = 1.0;
  // }
  //value += 1.0;
  const uint16_t twelve_bit_cmd = (uint16_t)(value);// * NORMALIZED_TO_12BIT_FACTOR);
  const uint16_t gain_selection = 1 << 13;
  const uint16_t channel_bit = is_left ? (1 << 15) : 0;
  const uint16_t power_on_bit = 1 << 12;
  return (twelve_bit_cmd | gain_selection | channel_bit | power_on_bit);
}

void SendFrameManually(uint16_t frame) {
  volatile int i = 0;
  volatile int j = 0;
  CLEAR_DAC_CS();
  for (j =0; j < 10; ++j);
  for (i=0; i < 16; ++i) {
    uint16_t mask = 1 << (15 - i);
    if (frame & mask) {
      SET_DAC_SDI();
    } else {
      CLEAR_DAC_SDI();
    }
    for (j =0; j < 10; ++j);
    SET_DAC_SCK();
    for (j =0; j < 10; ++j);
    CLEAR_DAC_SCK();
    for (j =0; j < 10; ++j);
  }
  SET_DAC_CS();
  for (j =0; j < 10; ++j);
}

void TransmitSamples(const uint16_t ch1_out, const uint16_t ch2_out) {
  volatile uint16_t both_samples[4] = {0};
  both_samples[0] = MakeCommandPacket(ch1_out, true);
  both_samples[1] = MakeCommandPacket(ch2_out, false);
  //spiSelect(&SPID2);                  /* Slave Select assertion.          */
  SET_DAC_LDAC(); // Don't set DAC outputs until LDAC goes low (after both inputs set)
 // spiSend(&SPID2, 4, (void*)&both_samples); /* Atomic transfer operations.      */
 SendFrameManually(both_samples[0]);
 SendFrameManually(both_samples[1]);
  CLEAR_DAC_LDAC(); // Copy DAC input registers to output 
  //spiUnselect(&SPID2);                /* Slave Select de-assertion.       */
  static uint16_t tx_counter = 0;
  if ((++tx_counter) > 1000) {
    tx_counter = 0;
    palTogglePad(GPIOC, GPIOC_LED);
  }

}