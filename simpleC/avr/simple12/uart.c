/* embeddedC/simpleC/avr/simple12/uart.c */

/* adaptado do Application Note "AVR306: Using the AVR UART in C" da Atmel */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "uart.h"

char UART_RxBuffer[UART_RX_BUFFER_SIZE];
volatile unsigned char UART_RxHead = 0;
unsigned char UART_RxTail = 0;

char UART_TxBuffer[UART_TX_BUFFER_SIZE];
unsigned char UART_TxHead = 0;
volatile unsigned char UART_TxTail = 0;


void uart_putchar(char c)
{
  unsigned char tmphead;

  if(c == '\n')
    uart_putchar('\r');

  tmphead = (UART_TxHead + 1) & UART_TX_BUFFER_MASK;
  while(tmphead == UART_TxTail);

  UART_TxBuffer[tmphead] = c;
  UART_TxHead = tmphead;

  /* habilito interrupção */
  UCSRB |= (1 << UDRIE);
}

void uart_puts(char* s)
{
  char c;
  while((c = *s++))
    {
      uart_putchar(c);
    }
}

void uart_puts_P(const char* s)
{
  char c;
  while((c = pgm_read_byte(s++)))
    {
      uart_putchar(c);
    }
}

unsigned char uart_kbhit(void)
{
  return UART_RxHead - UART_RxTail;
}

char uart_getchar(void)
{
  char c;
  unsigned char tmptail;

  while(!uart_kbhit());

  tmptail = (UART_RxTail + 1) & UART_RX_BUFFER_MASK;

  UART_RxTail = tmptail;

  c = UART_RxBuffer[tmptail];
  if(c == '\r') c = '\n';
  return c;
}

ISR(USART_RX_vect)
{
  unsigned char tmphead;

  /* calculo e salvo novo apontador */
  tmphead = (UART_RxHead + 1) & UART_RX_BUFFER_MASK;
  UART_RxHead = tmphead;

  /*
    if(tmphead == UART_RxTail)
    {
    // Overflow! Incluir tratamento se necessário.
    }
  */

  /* salvo caracter no buffer */
  UART_RxBuffer[tmphead] = UDR;
}

ISR(USART_UDRE_vect)
{
  unsigned char tmptail;

  if(UART_TxHead != UART_TxTail)
    {
      /* se ainda há dados para transmitir */
      /* calculo e salvo novo apontador */
      tmptail = (UART_TxTail + 1) & UART_TX_BUFFER_MASK;
      UART_TxTail = tmptail;
      /* transmito o dado */
      UDR = UART_TxBuffer[tmptail];
    }
  else
    {
      /* se não há dados, desabilito interrupção */
      UCSRB &= ~(1 << UDRIE);
    }
}
