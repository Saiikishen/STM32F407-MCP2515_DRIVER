STM32 HAL MCP2515 Driver is a lightweight, dependency-free library for interfacing STM32F4xx MCUs (tested on F407) with MCP2515 CAN controllers via SPI. It supports configuration, standard frame TX/RX polling, and diagnostics optimized for CubeIDE Live Expressions. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

## Features

- Pure HAL_SPI integration (no RTOS or external libs).
- Pre-configured for 500 kbps @ 8 MHz crystal (CNF1=0x03, CNF2=0xB1, CNF3=0x05). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- Mode switching: Normal, Loopback, Config, etc. (polls CANSTAT.OPMODE). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- TX: Auto-finds free buffer, loads std ID/DLC/data, polls TXnIF.
- RX: Polls RXB0/RXB1 priority, extracts frame.
- Diagnostics: Raw reg read/write/bit-modify; CANINTF/EFLG support. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

## Requirements

- STM32CubeIDE + HAL (F4xx family).
- SPI peripheral configured (e.g., SPI1: mode 0, NSS soft, ~2-10 MHz).
- CS GPIO (push-pull output, idle HIGH).
- MCP2515 module (e.g., HW-184) with 8 MHz XTAL, 5V supply. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

## Hardware Wiring (STM32F407 Example)

| MCP2515 Pin | STM32 Pin    | Notes |
|-------------|--------------|-------|
| VCC         | 5V           |       |
| GND         | GND          | Common GND |
| CS          | PC4 (GPIO)   | Idle HIGH |
| SCK         | PA5 (SPI1)   | AF5   |
| MOSI        | PA7 (SPI1)   | AF5   |
| MISO        | PA6 (SPI1)   | AF5   |
| INT         | NC           | Polling only |
| CANH/CANL   | Bus          | 120Ω term. at ends [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf) |

## Installation

1. Add `mcp2515.h` to `Core/Inc/`, `mcp2515.c` to `Core/Src/`.
2. CubeMX: Enable SPI1 (master, mode 0, 8-bit), GPIO for CS (output HIGH).
3. `#include "mcp2515.h"` in `main.c`.
4. Init: `MCP2515_HandleTypeDef hcan = {.hspi = &hspi1, .cs_port = GPIOC, .cs_pin = GPIO_PIN_4}; MCP2515_Init_500k_8MHz(&hcan);` [perplexity](https://www.perplexity.ai/search/54ed83d1-fa9e-4cff-afe3-c4b4a40f9048)

## API Reference

### Core Types
```c
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} MCP2515_HandleTypeDef;

typedef struct {
    uint16_t id;    // 11-bit std ID
    uint8_t dlc;
    uint8_t data[8];
} MCP2515_Frame;
```

### Key Functions
- `HAL_StatusTypeDef MCP2515_Init_500k_8MHz(MCP2515_HandleTypeDef *h)` – Reset + config + Normal mode. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- `HAL_StatusTypeDef MCP2515_SetMode(MCP2515_HandleTypeDef *h, uint8_t mode)` – e.g., `MCP2515_MODE_LOOPBACK = 0x02`. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- `HAL_StatusTypeDef MCP2515_SendStd(MCP2515_HandleTypeDef *h, uint16_t id, uint8_t dlc, uint8_t *data)` – Returns `HAL_OK`/`HAL_BUSY`/`HAL_ERROR`.
- `HAL_StatusTypeDef MCP2515_ReceivePoll(MCP2515_HandleTypeDef *h, MCP2515_Frame *frame)` – `HAL_OK` (frame), `HAL_BUSY` (none), `HAL_ERROR` (fail). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- `HAL_StatusTypeDef MCP2515_ReadReg(MCP2515_HandleTypeDef *h, uint8_t addr, uint8_t *val)` – e.g., CANSTAT=0x0E, CANINTF=0x2C. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

Constants: `MCP2515_REG_CANSTAT = 0x0E`, etc. (full in header). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

## Example Usage (main.c)

```c
MCP2515_HandleTypeDef hcan = {.hspi = &hspi1, .cs_port = GPIOC, .cs_pin = GPIO_PIN_4};
MCP2515_Frame frame = {0};

if (MCP2515_Init_500k_8MHz(&hcan) == HAL_OK) {
    while (1) {
        // TX
        uint8_t txdata[8] = {0xAA, 0x55, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
        MCP2515_SendStd(&hcan, 0x123, 8, txdata);

        // RX poll
        if (MCP2515_ReceivePoll(&hcan, &frame) == HAL_OK) {
            // Use frame.id/data
        }

        HAL_Delay(100);
    }
}
```
Globals for Live Expr: `g_last_canintf`, `g_last_eflg` via `MCP2515_ReadReg(&hcan, MCP2515_REG_CANINTF, &val);` [perplexity](https://www.perplexity.ai/search/54ed83d1-fa9e-4cff-afe3-c4b4a40f9048)

## Bit Timing Details

For 8 MHz XTAL, 500 kbps:
- tq = 20 (BRP=3 → 8 MHz / (1+3+1) / 20 = 500 kHz). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- CNF1: SJW=1 (0b001), BRP=3 (0b011).
- CNF2: BTLMODE=1 (sync+prop+ph1), PRSEG=1, PHSEG1=6 (0b101), sample=75%.
- CNF3: PHSEG2=3 (0b101). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

## Troubleshooting

- `HAL_TIMEOUT` init: CS wiring/MISO float (CANSTAT=0xFF). [perplexity](https://www.perplexity.ai/search/54ed83d1-fa9e-4cff-afe3-c4b4a40f9048)
- TX `HAL_BUSY`: No ACK (single node, no term). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
- EFLG !=0: Errors (bus-off=0x80). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)

## License

MIT – Free to use/modify. Based on MCP2515 datasheet (Microchip DS20001801J). [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/80763925/ac616e8f-7bfb-46ab-a6d2-ce74a2b783f2/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf)
