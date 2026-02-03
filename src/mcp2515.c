/*
 * mcp2515.c
 *
 *  Created on: Feb 3, 2026
 *      Author: saiikishen
 https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf - used for reference
 */


#include "mcp2515.h"

static inline void mcp2515_select(MCP2515_HandleTypeDef *h) {
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_RESET);
}

static inline void mcp2515_unselect(MCP2515_HandleTypeDef *h) {
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);
}

static HAL_StatusTypeDef mcp2515_spi_txrx(MCP2515_HandleTypeDef *h,
                                          uint8_t *tx, uint8_t *rx, uint16_t len)
{
    return HAL_SPI_TransmitReceive(h->hspi, tx, rx, len, HAL_MAX_DELAY);
}

/* --- Low-level primitives ------------------------------------------------ */

HAL_StatusTypeDef MCP2515_WriteReg(MCP2515_HandleTypeDef *h,
                                    uint8_t addr, uint8_t value)
{
    uint8_t buf[3] = { MCP2515_CMD_WRITE, addr, value };
    mcp2515_select(h);
    HAL_StatusTypeDef st = HAL_SPI_Transmit(h->hspi, buf, 3, HAL_MAX_DELAY);
    mcp2515_unselect(h);
    return st;
}

HAL_StatusTypeDef MCP2515_ReadReg(MCP2515_HandleTypeDef *h,
                                   uint8_t addr, uint8_t *value)
{
    uint8_t tx[3] = { MCP2515_CMD_READ, addr, 0x00 };
    uint8_t rx[3] = {0};

    mcp2515_select(h);
    HAL_StatusTypeDef st = mcp2515_spi_txrx(h, tx, rx, 3);
    mcp2515_unselect(h);

    if (st == HAL_OK && value) {
        *value = rx[2];
    }
    return st;
}

HAL_StatusTypeDef MCP2515_BitModify(MCP2515_HandleTypeDef *h,
                                     uint8_t addr, uint8_t mask, uint8_t data)
{
    uint8_t buf[4] = { MCP2515_CMD_BIT_MODIFY, addr, mask, data };
    mcp2515_select(h);
    HAL_StatusTypeDef st = HAL_SPI_Transmit(h->hspi, buf, 4, HAL_MAX_DELAY);
    mcp2515_unselect(h);
    return st;
}

/* RESET command  */
static HAL_StatusTypeDef MCP2515_Reset(MCP2515_HandleTypeDef *h)
{
    uint8_t cmd = MCP2515_CMD_RESET;
    mcp2515_select(h);
    HAL_StatusTypeDef st = HAL_SPI_Transmit(h->hspi, &cmd, 1, HAL_MAX_DELAY);
    mcp2515_unselect(h);
    HAL_Delay(1); /* allow OST to complete (128 cycles)  */
    return st;
}

/* --- Mode control -------------------------------------------------------- */

HAL_StatusTypeDef MCP2515_SetMode(MCP2515_HandleTypeDef *h, MCP2515_Mode mode)
{

    HAL_StatusTypeDef st = MCP2515_BitModify(h, MCP2515_REG_CANCTRL,
                                             0xE0, (uint8_t)mode);
    if (st != HAL_OK) return st;

    /* Poll OPMODE[2:0] in CANSTAT until it matches (Register 10-2) */
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 10) {
        uint8_t canstat = 0;
        st = MCP2515_ReadReg(h, MCP2515_REG_CANSTAT, &canstat);
        if (st != HAL_OK) return st;

        if ((canstat & 0xE0) == (uint8_t)mode) {
            return HAL_OK;
        }
    }
    return HAL_TIMEOUT;
}

/* --- Bit timing: 500 kbps @ 8 MHz --------------------------------------- */
/*
 * For Fosc = 8 MHz, target 500 kbit/s, one common config is:
 *   CNF1 = 0x00 (BRP=0, SJW=1TQ)
 *   CNF2 = 0x90 (BTLMODE=1, SAM=0, PHSEG1=3TQ, PRSEG=1TQ)
 *   CNF3 = 0x02 (PHSEG2=3TQ)
 * This gives 16 TQ per bit, sample at 75% .
 */
static HAL_StatusTypeDef MCP2515_Config_500k_8MHz(MCP2515_HandleTypeDef *h)
{
    HAL_StatusTypeDef st;

    st = MCP2515_WriteReg(h, MCP2515_REG_CNF1, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_CNF2, 0x90);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_CNF3, 0x02);
    if (st != HAL_OK) return st;

    return HAL_OK;
}

/* --- Filters/masks: accept all ------------------------------------------ */

static HAL_StatusTypeDef MCP2515_SetAcceptAll(MCP2515_HandleTypeDef *h)
{
    HAL_StatusTypeDef st;

    /* RXM0, RXM1 = 0x00000000 -> no bits masked off, but RXM bits in RXBxCTRL=11 receives all  */
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM0SIDH, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM0SIDL, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM0EID8, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM0EID0, 0x00);
    if (st != HAL_OK) return st;

    st = MCP2515_WriteReg(h, MCP2515_REG_RXM1SIDH, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM1SIDL, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM1EID8, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXM1EID0, 0x00);
    if (st != HAL_OK) return st;

    /* Filters = 0 (match any ID when mask=0)  */
    st = MCP2515_WriteReg(h, MCP2515_REG_RXF0SIDH, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_RXF0SIDL, 0x00);
    if (st != HAL_OK) return st;
    /* Similarly RXF1..RXF5 if needed */

    /* RXB0CTRL.RXM[1:0] = 11 (receive all messages)  */
    st = MCP2515_WriteReg(h, MCP2515_REG_RXB0CTRL, 0x60);
    if (st != HAL_OK) return st;
    /* RXB1CTRL.RXM[1:0] = 11 (receive all) */
    st = MCP2515_WriteReg(h, MCP2515_REG_RXB1CTRL, 0x60);
    if (st != HAL_OK) return st;

    return HAL_OK;
}

/* --- Public init: 500 kbps, 8 MHz, polling RX --------------------------- */

HAL_StatusTypeDef MCP2515_Init_500k_8MHz(MCP2515_HandleTypeDef *h)
{
    HAL_StatusTypeDef st;

    /* CS idle high */
    HAL_GPIO_WritePin(h->cs_port, h->cs_pin, GPIO_PIN_SET);

    /* Hardware RESET via command */
    st = MCP2515_Reset(h);
    if (st != HAL_OK) return st;

    /* Enter Configuration mode */
    st = MCP2515_SetMode(h, MCP2515_MODE_CONFIG);
    if (st != HAL_OK) return st;

    /* Configure bit timing */
    st = MCP2515_Config_500k_8MHz(h);
    if (st != HAL_OK) return st;

    /* Accept all frames into RXB0/RXB1 */
    st = MCP2515_SetAcceptAll(h);
    if (st != HAL_OK) return st;

    /* Disable all interrupts (we poll) */
    st = MCP2515_WriteReg(h, MCP2515_REG_CANINTE, 0x00);
    if (st != HAL_OK) return st;

    /* Clear interrupt flags and error flags  */
    st = MCP2515_WriteReg(h, MCP2515_REG_CANINTF, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_EFLG, 0x00);
    if (st != HAL_OK) return st;

    /* Finally go to Normal mode */
    st = MCP2515_SetMode(h, MCP2515_MODE_NORMAL);
    return st;
}

/* --- Helpers for ID packing/unpacking (standard frames) ----------------- */

/* Pack 11-bit ID into SIDH/SIDL for TXBnSIDH/SIDL (Section 3.0)  */
static void mcp2515_pack_std_id(uint16_t id, uint8_t *sidh, uint8_t *sidl)
{
    *sidh = (uint8_t)(id >> 3);             /* SID10..SID3 */
    *sidl = (uint8_t)((id & 0x07) << 5);    /* SID2..SID0, EXIDE=0 */
}

/* Unpack 11-bit ID from RXBnSIDH/SIDL [file:75] */
static uint16_t mcp2515_unpack_std_id(uint8_t sidh, uint8_t sidl)
{
    uint16_t id = 0;
    id  = ((uint16_t)sidh) << 3;
    id |= (uint16_t)(sidl >> 5);
    return (id & 0x7FF);
}

/* --- TX: use TXB0 only -------------------------------------------------- */

HAL_StatusTypeDef MCP2515_SendStd(MCP2515_HandleTypeDef *h,
                                   uint16_t id,
                                   uint8_t dlc,
                                   const uint8_t *data)
{
    if (dlc > 8) dlc = 8;

    /* Check TXB0CTRL.TXREQ; if pending, you may choose to wait or fail  */
    uint8_t txb0ctrl = 0;
    HAL_StatusTypeDef st = MCP2515_ReadReg(h, MCP2515_REG_TXB0CTRL, &txb0ctrl);
    if (st != HAL_OK) return st;
    if (txb0ctrl & 0x08) {
        /* TXREQ set -> buffer busy */
        return HAL_BUSY;
    }

    /* Write ID */
    uint8_t sidh, sidl;
    mcp2515_pack_std_id(id, &sidh, &sidl);
    st = MCP2515_WriteReg(h, MCP2515_REG_TXB0SIDH, sidh);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_TXB0SIDL, sidl);
    if (st != HAL_OK) return st;

    /* Extended ID registers = 0 for standard frame */
    st = MCP2515_WriteReg(h, MCP2515_REG_TXB0EID8, 0x00);
    if (st != HAL_OK) return st;
    st = MCP2515_WriteReg(h, MCP2515_REG_TXB0EID0, 0x00);
    if (st != HAL_OK) return st;

    /* DLC (RTR=0) */
    st = MCP2515_WriteReg(h, MCP2515_REG_TXB0DLC, (uint8_t)(dlc & 0x0F));
    if (st != HAL_OK) return st;

    /* Data bytes */
    for (uint8_t i = 0; i < dlc; i++) {
        st = MCP2515_WriteReg(h, (uint8_t)(MCP2515_REG_TXB0D0 + i), data[i]);
        if (st != HAL_OK) return st;
    }

    /* Request to send TXB0 (RTS instruction) [file:75] */
    uint8_t cmd = MCP2515_CMD_RTS_TX0;
    mcp2515_select(h);
    st = HAL_SPI_Transmit(h->hspi, &cmd, 1, HAL_MAX_DELAY);
    mcp2515_unselect(h);

    return st;
}

/* --- RX: simple polling of RXB0/RXB1 ----------------------------------- */

HAL_StatusTypeDef MCP2515_ReceivePoll(MCP2515_HandleTypeDef *h,
                                      MCP2515_Frame *frame)
{
    if (!frame) return HAL_ERROR;

    uint8_t canintf = 0;
    HAL_StatusTypeDef st = MCP2515_ReadReg(h, MCP2515_REG_CANINTF, &canintf);
    if (st != HAL_OK) return st;

    uint8_t buf[13];
    uint8_t sidh, sidl, dlc;
    uint8_t base;
    uint8_t flag_mask;

    if (canintf & 0x01) {
        /* RX0IF set -> RXB0 has a message  */
        base = MCP2515_REG_RXB0SIDH;
        flag_mask = 0x01;
    } else if (canintf & 0x02) {
        /* RX1IF set -> RXB1 has a message */
        base = MCP2515_REG_RXB1SIDH;
        flag_mask = 0x02;
    } else {
        return HAL_BUSY; /* no message */
    }

    /* Read SIDH..DLC + up to 8 data bytes in one burst */
    uint8_t tx[1] = { MCP2515_CMD_READ };
    mcp2515_select(h);
    /* Send READ + address */
    HAL_SPI_Transmit(h->hspi, tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(h->hspi, &base, 1, HAL_MAX_DELAY);
    /* Receive 13 bytes: SIDH, SIDL, EID8, EID0, DLC, D0..D7  */
    st = HAL_SPI_Receive(h->hspi, buf, 13, HAL_MAX_DELAY);
    mcp2515_unselect(h);
    if (st != HAL_OK) return st;

    sidh = buf[0];
    sidl = buf[1];
    dlc  = buf[4] & 0x0F;
    if (dlc > 8) dlc = 8;

    frame->id  = mcp2515_unpack_std_id(sidh, sidl);
    frame->dlc = dlc;
    for (uint8_t i = 0; i < dlc; i++) {
        frame->data[i] = buf[5 + i];
    }

    /* Clear RXnIF flag via BIT MODIFY  */
    st = MCP2515_BitModify(h, MCP2515_REG_CANINTF, flag_mask, 0x00);
    return st;
}
