/********************************************************************************
* filename:     bootloader.c
* description:  bootloader for low density stm8l1xx series
* author:       ethan
* change history:
*   time        version
* 2016-11-30    v1.0
**********************************************************************************/

#include "bootloader.h"

int main(void)
{
  uint16_t tryCnt = 65535;
  uint8_t ch, page;
  uint8_t buf[BLOCK_BYTES];
  uint16_t i;
  uint8_t verify;
  uint16_t startAddress;
  uint8_t* addr;
  
  // Configure the system 
  // 1: configure the clock
  // set HSI as the sysclk source = 16MHz
  CLK->CKDIVR = 0x00;
  // enable the USART clock only
  CLK->PCKENR |= 0x20;
  // 2: configure the USART
  // set baudrate = 115200, enable the receive and send
  // Set the fraction of USARTDIV 
  USART->BRR2 = (uint8_t)((BaudRate_Mantissa >> (uint8_t)8)& (uint8_t)0xF0);
  // Set the MSB mantissa of USARTDIV 
  USART->BRR2 |= (uint8_t)(BaudRate_Mantissa & (uint8_t)0x0F);
  // USART->BRR2 = 0x0a;// baudrate 115200
  // Set the LSB mantissa of USARTDIV
  USART->BRR1 = (uint8_t)(BaudRate_Mantissa >> (uint8_t)4);
  // USART->BRR1 = 0x08; // baudrate 115200
  USART->CR2 |= (uint8_t)(USART_CR2_TEN | USART_CR2_REN);
  
  /* configure the gpio for the usart */
  /* configure the TX to push-pull */
  // set high PC3
  GPIOC->ODR |= (uint8_t)((0x08));
  // set output PC3
  GPIOC->DDR |= 0x08;
  // set push-pull PC3
  GPIOC->CR1 |= 0x08;
  /* configure the RX to input floating */
  // set input PC2
  GPIOC->DDR &= (uint8_t)(~(0x04));
  // set input-pull up
  GPIOC->CR1 |= (uint8_t)((0x04));
  
  i = 10;// about 0.5s
  while(i){
    //wait for head 
    if(USART->SR & USART_FLAG_RXNE){
      ch = (uint8_t)USART->DR;
      if(ch == BOOT_HEAD){
        USART_Send(BOOT_HEAD_OK);
        break;
      }
    }
    tryCnt--;
    if(tryCnt == 0) i--;
  }
  
  if(i == 0){
    //goto app
    goto goApp;
  }else{
    //unlock flash
    FLASH->PUKR = FLASH_RASS_KEY1;
    FLASH->PUKR = FLASH_RASS_KEY2;
    // wait until the flash memory unlock flag to be set 
    while(!(FLASH->IAPSR & 0x02));  
    while(1){
      ch = USART_Recv();
      switch(ch){
        case BOOT_GO:
          goApp:
          // unlock the main program area 
          FLASH->IAPSR &= FLASH_MEMTYPE_PROG; 
          //goto app
#ifdef INTERRUPT_ADDRESS_OVERLAPPED
          asm("JP $82fc");
#endif
#ifdef INTERRUPT_ADDRESS_REMAP
          asm("JP $8280");
#endif
          break;
        case BOOT_WRITE:
          page = USART_Recv();
          startAddress = FLASH_START + (page << BLOCK_SHIFT);
          addr = (uint8_t*)startAddress;
          // we need to check the validity of the Address           
          verify = 0;
          for(i = 0; i < BLOCK_BYTES; i++){
            buf[i] = USART_Recv();
            verify += buf[i];
          }
#ifdef USING_TRANSFER_VERIFICATION
          // send the verification byte 
          if(verify == USART_Recv()){
              USART_Send(BOOT_TRANSFER_VERIFICATION_OK);
              FLASH_ProgBlock(startAddress, buf);
#ifdef USING_PROGRAM_VERIFICATION
              // flash program verification error
              for(i = 0; i < BLOCK_BYTES; i++){
                  verify -= addr[i];
              }
              // verification the flash programming 
              if(verify == 0){
                USART_Send(BOOT_FLASH_VERIFICATION_OK);
              }else{
                // flash program verification error 
                USART_Send(BOOT_FLASH_VERIFICATION_ERR);
              }
#endif
          }else{
            // transfer verification err
            USART_Send(BOOT_TRANSFER_VERIFICATION_ERR);
          }
#else
          FLASH_ProgBlock(startAddress, buf);
          USART_Send(BOOT_OK);
#endif
          break;
      case BOOT_READ:
        // do we need to validate the offset
        addr = (uint8_t*)(FLASH_START + USART_Recv());
        USART_Send(*addr);
        break;
#ifdef INTERRUPT_ADDRESS_OVERLAPPED
      case BOOT_HEAD_MAIN:
        startAddress = ((uint16_t)FLASH_APP_MAIN/BLOCK_BYTES)<<BLOCK_SHIFT;
        addr = (uint8_t*)(startAddress);
        for(i = 0; i < BLOCK_BYTES; i++){
          buf[i] = addr[i];
        }
        for(i = 0; i < 4; i++){
          buf[i+FLASH_APP_MAIN-startAddress] = USART_Recv();       
        }
        FLASH_ProgBlock(startAddress, buf);
        USART_Send(BOOT_OK);
        break;
#endif
      default:
        // unkown type
        USART_Send(BOOT_UNKNOWN_ERR);
        break;
       }
    }
  }
}

void USART_Send(uint8_t ch){
  while (!(USART->SR & USART_FLAG_TC));
  USART->DR = ch;
}

uint8_t USART_Recv(void){
  while(!(USART->SR & USART_FLAG_RXNE));
  return ((uint8_t)USART->DR);
}

//addr must at begin of block
IN_RAM(void FLASH_ProgBlock(uint16_t StartAddress, uint8_t *Buffer))
{
  uint8_t i = 0;
  // Standard programming mode */ /*No need in standard mode
  FLASH->CR2 |= FLASH_CR2_PRG;
  // Copy data bytes from RAM to FLASH memory
  // within the block program, all bytes written in a programming sequence must be in the same block
  for (i = 0; i < BLOCK_BYTES; i++){
    *((PointerAttr uint8_t*)StartAddress + i) = ((uint8_t)(Buffer[i]));
  }
  while((uint8_t)(FLASH->IAPSR & (FLASH_IAPSR_EOP | FLASH_IAPSR_WR_PG_DIS)) == 0x00);
}
