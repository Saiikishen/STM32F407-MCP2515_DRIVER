#ifndef MCP2515_H
#define MCP2515_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;
} MCP2515_HandleTypeDef;


#define MCP2515_CMD_RESET          0xC0
#define MCP2515_CMD_READ           0x03
#define MCP2515_CMD_WRITE          0x02
#define MCP2515_CMD_BIT_MODIFY     0x05
#define MCP2515_CMD_READ_STATUS    0xA0
#define MCP2515_CMD_RX_STATUS      0xB0
#define MCP2515_CMD_READ_RX0       0x90
#define MCP2515_CMD_READ_RX1       0x94
#define MCP2515_CMD_LOAD_TX0       0x40
#define MCP2515_CMD_RTS_TX0        0x81


#define MCP2515_REG_CANCTRL        0x0F
#define MCP2515_REG_CANSTAT        0x0E
#define MCP2515_REG_CNF1           0x2A
#define MCP2515_REG_CNF2           0x29
#define MCP2515_REG_CNF3           0x28
#define MCP2515_REG_CANINTE        0x2B
#define MCP2515_REG_CANINTF        0x2C
#define MCP2515_REG_EFLG           0x2D


#define MCP2515_REG_TXB0CTRL       0x30
#define MCP2515_REG_TXB0SIDH       0x31
#define MCP2515_REG_TXB0SIDL       0x32
#define MCP2515_REG_TXB0EID8       0x33
#define MCP2515_REG_TXB0EID0       0x34
#define MCP2515_REG_TXB0DLC        0x35
#define MCP2515_REG_TXB0D0         0x36  /* up to TXB0D7 (0x3D) */


#define MCP2515_REG_RXB0CTRL       0x60
#define MCP2515_REG_RXB0SIDH       0x61
#define MCP2515_REG_RXB0SIDL       0x62
#define MCP2515_REG_RXB0EID8       0x63
#define MCP2515_REG_RXB0EID0       0x64
#define MCP2515_REG_RXB0DLC        0x65
#define MCP2515_REG_RXB0D0         0x66

#define MCP2515_REG_RXB1CTRL       0x70
#define MCP2515_REG_RXB1SIDH       0x71
#define MCP2515_REG_RXB1SIDL       0x72
#define MCP2515_REG_RXB1EID8       0x73
#define MCP2515_REG_RXB1EID0       0x74
#define MCP2515_REG_RXB1DLC        0x75
#define MCP2515_REG_RXB1D0         0x76

#define MCP2515_REG_RXM0SIDH       0x20
#define MCP2515_REG_RXM0SIDL       0x21
#define MCP2515_REG_RXM0EID8       0x22
#define MCP2515_REG_RXM0EID0       0x23
#define MCP2515_REG_RXM1SIDH       0x24
#define MCP2515_REG_RXM1SIDL       0x25
#define MCP2515_REG_RXM1EID8       0x26
#define MCP2515_REG_RXM1EID0       0x27

#define MCP2515_REG_RXF0SIDH       0x00
#define MCP2515_REG_RXF0SIDL       0x01


typedef enum {
    MCP2515_MODE_NORMAL      = 0x00,
    MCP2515_MODE_SLEEP       = 0x20,
    MCP2515_MODE_LOOPBACK    = 0x40,
    MCP2515_MODE_LISTEN_ONLY = 0x60,
    MCP2515_MODE_CONFIG      = 0x80
} MCP2515_Mode;

typedef struct {
    uint16_t id;       /* 11-bit identifier */
    uint8_t  dlc;      /* 0..8 */
    uint8_t  data[8];
} MCP2515_Frame;

/* Public API */
HAL_StatusTypeDef MCP2515_Init_500k_8MHz(MCP2515_HandleTypeDef *h);
HAL_StatusTypeDef MCP2515_SetMode(MCP2515_HandleTypeDef *h, MCP2515_Mode mode);

/* TX/RX (polling, no INT) */
HAL_StatusTypeDef MCP2515_SendStd(MCP2515_HandleTypeDef *h,
                                   uint16_t id,
                                   uint8_t dlc,
                                   const uint8_t *data);

HAL_StatusTypeDef MCP2515_ReceivePoll(MCP2515_HandleTypeDef *h,
                                      MCP2515_Frame *frame);

/* Low-level helpers (optional use) */
HAL_StatusTypeDef MCP2515_WriteReg(MCP2515_HandleTypeDef *h,
                                    uint8_t addr, uint8_t value);
HAL_StatusTypeDef MCP2515_ReadReg(MCP2515_HandleTypeDef *h,
                                   uint8_t addr, uint8_t *value);
HAL_StatusTypeDef MCP2515_BitModify(MCP2515_HandleTypeDef *h,
                                     uint8_t addr, uint8_t mask, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* MCP2515_H */
