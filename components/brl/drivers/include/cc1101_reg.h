#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

// Status registers
#define CC1101_PARTNUM        0x30        // Part number
#define CC1101_VERSION        0x31        // Current version number
#define CC1101_FREQEST       0x32        // Frequency offset estimate
#define CC1101_RSSI          0x34        // Received signal strength indication
#define CC1101_MARCSTATE     0x35        // Control state machine state
#define CC1101_WORTIME1      0x36        // High byte of WOR timer
#define CC1101_WORTIME0      0x37        // Low byte of WOR timer
#define CC1101_PKTSTATUS     0x38        // Current GDOx status and packet status
#define CC1101_VCO_VC_DAC    0x39        // Current setting from PLL calibration module
#define CC1101_TXBYTES       0x3A        // Underflow and number of bytes in TX FIFO
#define CC1101_RXBYTES       0x3B        // Overflow and number of bytes in RX FIFO
#define CC1101_RCCTRL1_STATUS 0x3C        // Last RC oscillator calibration result
#define CC1101_RCCTRL0_STATUS 0x3D        // Last RC oscillator calibration result

// Command strobes
#define CC1101_SRES          0x30        // Reset chip
#define CC1101_SFSTXON       0x31        // Enable and calibrate frequency synthesizer
#define CC1101_SXOFF         0x32        // Turn off crystal oscillator
#define CC1101_SCAL          0x33        // Calibrate frequency synthesizer
#define CC1101_SRX           0x34        // Enable RX
#define CC1101_STX           0x35        // Enable TX
#define CC1101_SIDLE         0x36        // Exit RX/TX
#define CC1101_SWOR          0x38        // Start automatic RX polling
#define CC1101_SPWD          0x39        // Enter power down mode
#define CC1101_SFRX          0x3A        // Flush RX FIFO buffer
#define CC1101_SFTX          0x3B        // Flush TX FIFO buffer
#define CC1101_SWORRST       0x3C        // Reset real time clock
#define CC1101_SNOP          0x3D        // No operation

// Configuration registers
#define CC1101_IOCFG2        0x00        // GDO2 Output Pin Configuration
#define CC1101_IOCFG1        0x01        // GDO1 Output Pin Configuration
#define CC1101_IOCFG0        0x02        // GDO0 Output Pin Configuration
#define CC1101_FIFOTHR       0x03        // RX FIFO and TX FIFO Thresholds
#define CC1101_SYNC1         0x04        // Sync Word, High Byte
#define CC1101_SYNC0         0x05        // Sync Word, Low Byte
#define CC1101_PKTLEN        0x06        // Packet Length
#define CC1101_PKTCTRL1      0x07        // Packet Automation Control
#define CC1101_PKTCTRL0      0x08        // Packet Automation Control
#define CC1101_ADDR          0x09        // Device Address
#define CC1101_CHANNR        0x0A        // Channel Number
#define CC1101_FSCTRL1       0x0B        // Frequency Synthesizer Control
#define CC1101_FSCTRL0       0x0C        // Frequency Synthesizer Control
#define CC1101_FREQ2         0x0D        // Frequency Control Word, High Byte
#define CC1101_FREQ1         0x0E        // Frequency Control Word, Middle Byte
#define CC1101_FREQ0         0x0F        // Frequency Control Word, Low Byte

// CC1101 state definitions
typedef enum {
    CC1101_STATE_IDLE = 0,
    CC1101_STATE_RX,
    CC1101_STATE_TX,
    CC1101_STATE_FSTXON,
    CC1101_STATE_CALIBRATE,
    CC1101_STATE_SETTLING,
    CC1101_STATE_RXFIFO_OVERFLOW,
    CC1101_STATE_TXFIFO_UNDERFLOW
} cc1101_state_t;

// CC1101 configuration structure
typedef struct {
    spi_host_device_t spi_host;
    int miso_pin;
    int mosi_pin;
    int sck_pin;
    int cs_pin;
    int gdo0_pin;
    int gdo2_pin;
} cc1101_config_t;
