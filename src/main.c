
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Device header file.
#ifdef VVC_F1
  #include "stm32f1xx.h"
#elif VVC_L4
  #include "stm32l4xx.h"
#endif

uint32_t SystemCoreClock = 0;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;
volatile char rxb;


__attribute__( ( naked ) ) void reset_handler( void ) {
  
  __asm__( "LDR r0, =_estack\n\t"
            "MOV sp, r0" );

  __asm__( "B main" );
}


int _write( int handle, char* data, int size ) {
  int count = size;
  while ( count-- ) {
    #ifdef VVC_F1
      while ( !( USART2->SR & USART_SR_TXE ) ) {};
      USART2->DR = *data++;
    #elif VVC_L4
      while ( !( USART2->ISR & USART_ISR_TXE ) ) {};
      USART2->TDR = *data++;
    #endif
  }
  return size;
}



int main( void ) {
 
  memcpy( &_sdata, &_sidata, ( ( void* )&_edata - ( void* )&_sdata ) );
  
  memset( &_sbss, 0x00, ( ( void* )&_ebss - ( void* )&_sbss ) );


  SCB->CPACR    |=  ( 0xF << 20 );


  #ifdef VVC_F1

    SystemCoreClock = 8000000;
  #elif VVC_L4


    RCC->CR |=  ( RCC_CR_HSION );
    while ( !( RCC->CR & RCC_CR_HSIRDY ) ) {};
    RCC->CFGR &= ~( RCC_CFGR_SW );
    RCC->CFGR |=  ( RCC_CFGR_SW_HSI );
    while ( ( RCC->CFGR & RCC_CFGR_SWS ) != RCC_CFGR_SWS_HSI ) {};
    SystemCoreClock = 16000000;
  #endif

  #ifdef VVC_F1

    RCC->APB1ENR  |=  ( RCC_APB1ENR_USART2EN );
    RCC->APB2ENR  |=  ( RCC_APB2ENR_IOPAEN );

    GPIOA->CRL    &= ~( GPIO_CRL_MODE2 |
                        GPIO_CRL_CNF2 |
                        GPIO_CRL_MODE3 |
                        GPIO_CRL_CNF3 );
    GPIOA->CRL    |= ( ( 0x1 << GPIO_CRL_MODE2_Pos ) |
                       ( 0x2 << GPIO_CRL_CNF2_Pos ) |
                       ( 0x0 << GPIO_CRL_MODE3_Pos ) |
                       ( 0x1 << GPIO_CRL_CNF3_Pos ) );
  #elif VVC_L4

    RCC->APB1ENR1 |= ( RCC_APB1ENR1_USART2EN );
    RCC->AHB2ENR  |= ( RCC_AHB2ENR_GPIOAEN );

    GPIOA->MODER    &= ~( ( 0x3 << ( 2 * 2 ) ) |
                          ( 0x3 << ( 15 * 2 ) ) );
    GPIOA->MODER    |=  ( ( 0x2 << ( 2 * 2 ) ) |
                          ( 0x2 << ( 15 * 2 ) ) );
    GPIOA->OTYPER   &= ~( ( 0x1 << 2 ) |
                          ( 0x1 << 15 ) );
    GPIOA->OSPEEDR  &= ~( ( 0x3 << ( 2 * 2 ) ) |
                          ( 0x3 << ( 15 * 2 ) ) );
    GPIOA->OSPEEDR  |=  ( ( 0x2 << ( 2 * 2 ) ) |
                          ( 0x2 << ( 15 * 2 ) ) );
    GPIOA->AFR[ 0 ] &= ~( ( 0xF << ( 2 * 4 ) ) );
    GPIOA->AFR[ 0 ] |=  ( ( 0x7 << ( 2 * 4 ) ) );
    GPIOA->AFR[ 1 ] &= ~( ( 0xF << ( ( 15 - 8 ) * 4 ) ) );
    GPIOA->AFR[ 1 ] |=  ( ( 0x3 << ( ( 15 - 8 ) * 4 ) ) );
  #endif



  NVIC_SetPriorityGrouping( 0 );

  uint32_t uart_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
  NVIC_SetPriority( USART2_IRQn, uart_pri_encoding );
  NVIC_EnableIRQ( USART2_IRQn );

//baud rate set to 9600.
  uint16_t uartdiv = SystemCoreClock / 9600;
  #ifdef VVC_F1
    USART2->BRR = ( ( ( uartdiv / 16 ) << USART_BRR_DIV_Mantissa_Pos ) |
                    ( ( uartdiv % 16 ) << USART_BRR_DIV_Fraction_Pos ) );
  #elif VVC_L4
    USART2->BRR = uartdiv;
  #endif

  USART2->CR1 |= ( USART_CR1_RE |
                   USART_CR1_TE |
                   USART_CR1_UE |
                   USART_CR1_RXNEIE );

  // Main loop
  while ( 1 ) {
    __WFI();
    putchar( rxb );
    fflush( stdout );
  }
}

// USART2 interrupt handler
void USART2_IRQn_handler( void ) {
  #ifdef VVC_F1

    if ( USART2->SR & USART_SR_RXNE ) {

      rxb = USART2->DR;
    }
  #elif VVC_L4

    if ( USART2->ISR & USART_ISR_RXNE ) {

      rxb = USART2->RDR;
    }
  #endif
}
