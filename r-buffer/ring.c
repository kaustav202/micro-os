#include "ringbuf.h"

#define RINGBUF_SIZE ( 128 )
volatile char rb_buf[ RINGBUF_SIZE + 1 ];
ringbuf rb = {
  len: RINGBUF_SIZE,
  buf: rb_buf,
  pos: 0,
  ext: 0
};
volatile int newline = 0;

int main(void){

 while ( 1 ) {
    while ( newline == 0 ) {
      __WFI();
    }
    while ( rb.pos != rb.ext ) {
      putchar( ringbuf_read( &rb ) );
    }
    printf( "\n" );
    newline = 0;
  	}
}

// USART2 interrupt handler
void USART2_IRQn_handler( void ) {
  #ifdef VVC_F1

    if ( USART2->SR & USART_SR_RXNE ) {

      char c = USART2->DR;
      ringbuf_write( rb, c );
      if ( c == '\r' ) { newline = 1; }
    }
  #elif VVC_L4

    if ( USART2->ISR & USART_ISR_RXNE ) {

      char c = USART2->RDR;
      ringbuf_write( rb, c );
      if ( c == '\r' ) { newline = 1; }
    }
  #endif
}
