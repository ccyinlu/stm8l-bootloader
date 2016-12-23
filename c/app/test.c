/********************************************************************************
* filename:     test.c
* description:  test for bootloader
* author:       ethan
* change history:
*   time        version
* 2016-11-30    v1.0
**********************************************************************************/

#include "stm8l10x.h"

#define   USART_FLAG_RXNE       ((uint8_t)0x20) /*!< Read Data Register Not Empty flag */
#define   USART_FLAG_TXE        ((uint8_t)0x80) /*!< Transmit Data Register Empty flag */
#define   USART_FLAG_TC         ((uint8_t)0x40) /*!< Transmission Complete flag */

//uart parameter
#define   SYSCLK                16000000
#define   BAUDRATE              57600
#define   BaudRate_Mantissa     ((uint32_t)SYSCLK / BAUDRATE)
#define   BaudRate_Mantissa100  ((uint32_t)SYSCLK*100 / BAUDRATE)
#define   USART_DIV             SYSCLK/BAUDRATE
void USART_Send(uint8_t ch){
  while (!(USART->SR & USART_FLAG_TC));
  USART->DR = ch;
}

void delay(uint16_t counter){
  uint16_t i,j,k;
  for(k=0;k<counter;k++)
    for(i=0;i<2000;i++)
      for(j=0;j<2000;j++)
        ;
}

int main(void){
  /* Configure the system */
  // 1: configure the clock
  /* set HSI as the sysclk source = 16MHz */
  CLK->CKDIVR = 0x00;
  /* enable the USART clock only */
  CLK->PCKENR |= 0x20; 
  // 2: configure the USART
  /* set baudrate = 115200, enable the receive and send */
  /* Set the fraction of USARTDIV */
  USART->BRR2 = (uint8_t)((BaudRate_Mantissa >> (uint8_t)8)& (uint8_t)0xF0);
  /* Set the MSB mantissa of USARTDIV */
  USART->BRR2 |= (uint8_t)(BaudRate_Mantissa & (uint8_t)0x0F);
  /* Set the LSB mantissa of USARTDIV */
  USART->BRR1 = (uint8_t)(BaudRate_Mantissa >> (uint8_t)4);
  
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
  
  USART_Send('h');
  USART_Send('e');
  USART_Send('l');
  USART_Send('l');
  USART_Send('o');
  USART_Send(' ');
  USART_Send('w');
  USART_Send('o');
  USART_Send('r');
  USART_Send('l');
  USART_Send('d');
  USART_Send('\n');
  while(1){
  }
}



