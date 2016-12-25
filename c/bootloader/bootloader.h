/********************************************************************************
* filename:     bootloader.h
* description:  bootloader for low density stm8l1xx series
* author:       ethan
* change history:
*   time        version
* 2016-11-30    v1.0
**********************************************************************************/
#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "stm8l10x.h"
#include <stdio.h>

/*********************************************************************************/
/***************************** function defination *******************************/
//#define   USING_TRANSFER_VERIFICATION
//#define   USING_PROGRAM_VERIFICATION
/*********************************************************************************/
// choose one of the option below
#define   INTERRUPT_ADDRESS_REMAP
//#define   INTERRUPT_ADDRESS_OVERLAPPED

//device information
#define   BLOCK_BYTES           64
#define	  BLOCK_SHIFT           6
#define   FLASH_START           0x008000
#define   FLASH_END             0x009F80
#define   FLASH_APP_MAIN        0x827c

//cmd code
#ifdef USING_TRANSFER_VERIFICATION
#define   BOOT_TRANSFER_VERIFICATION_OK	  0xa0
#define   BOOT_TRANSFER_VERIFICATION_ERR  0xa1
#endif
#ifdef USING_PROGRAM_VERIFICATION
#define   BOOT_FLASH_VERIFICATION_OK      0xa2
#define   BOOT_FLASH_VERIFICATION_ERR     0xa3
#endif

#define   BOOT_UNKNOWN_ERR                0xa4
#define   BOOT_HEAD                       0xa5
#define   BOOT_READ                       0xa6
#define   BOOT_WRITE                      0xa7
#define   BOOT_GO                         0xa8
#define   BOOT_OK                         0xa9
#define   BOOT_HEAD_OK                    0xaa
#define   BOOT_HEAD_MAIN                  0xab


//uart parameter
#define   SYSCLK                16000000
#define   BAUDRATE              57600
#define   BaudRate_Mantissa     ((uint32_t)SYSCLK / BAUDRATE)
#define   BaudRate_Mantissa100  ((uint32_t)SYSCLK*100 / BAUDRATE)
#define   USART_DIV             SYSCLK/BAUDRATE


//reg definition
#define   FLASH_RASS_KEY1       ((uint8_t)0x56) /*!< First RASS key */
#define   FLASH_RASS_KEY2       ((uint8_t)0xAE) /*!< Second RASS key */

// status defination
#define   USART_FLAG_RXNE       ((uint8_t)0x20) /*!< Read Data Register Not Empty flag */
#define   USART_FLAG_TXE        ((uint8_t)0x80) /*!< Transmit Data Register Empty flag */
#define   USART_FLAG_TC         ((uint8_t)0x40) /*!< Transmission Complete flag */
#define   FLASH_MEMTYPE_PROG    ((uint8_t)0xFD) /*!< Program memory */
#define   FLASH_MEMTYPE_DATA    ((uint8_t)0xF7) /*!< Data EEPROM memory */


void USART_Send(uint8_t ch);
uint8_t USART_Recv(void);

IN_RAM(void FLASH_ProgBlock(uint16_t StartAddress, uint8_t *Buffer));

#endif
