/* embeddedC/simpleC/avr/simple12/uart.h */

#ifndef _UART_H_
#define _UART_H_ 1

/* baudrate */
#ifndef BAUDRATE
#define BAUDRATE 9600
#endif
#define BAUD2X 1

/* tamanho do buffer de recepção da UART */
#define UART_RX_BUFFER_SIZE 16
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1)
#if (UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK)
#error "UART_RX_BUFFER_SIZE NÃO É POTÊNCIA DE 2"
#endif

/*---------------------------------------------------------------------------*
 * Cálculo da constante UBRR (Baud Rate Register)
 * BAUD2X pode ser 0 ou 1
 *
 * A fórmula simples seria
 * UBRR = ( F_CPU/(16>>BAUD2X)/BAUDRATE - 1 )
 * mas para arredondar corretamente fica então:
 * UBRR = (((10*F_CPU)/(16>>BAUD2X)/BAUDRATE-5)/10)
 */
#if (BAUD2X != 0 && BAUD2X != 1)
#error "BAUD2X deve ser 0 ou 1"
#endif
#define BAUDK (((10*F_CPU)/(16>>BAUD2X)/BAUDRATE-5)/10)

/* erro de baudrate em partes por 1000 */
#define BAUDREAL (((10*F_CPU/(16>>BAUD2X)/(BAUDK+1))+5)/10)
#if BAUDREAL > BAUDRATE
#define BAUDERROR (((BAUDREAL-BAUDRATE)*1000+(BAUDRATE/2))/BAUDRATE)
#else
#define BAUDERROR (((BAUDRATE-BAUDREAL)*1000+(BAUDRATE/2))/BAUDRATE)
#endif
#if BAUDERROR > 25
#warning "DESVIO DE BAUDRATE MAIOR QUE 2.5%"
#endif

/* Sanity Check - válido apenas se F_CPU = 1000000 e baudrate = 9600 */
#if BAUDK != 12
#error "ERRO NO CÁLCULO DO BAUDRATE"
#endif
/*---------------------------------------------------------------------------*/

void uart_putchar(char c);
void uart_puts(char* s);
void uart_puts_P(const char* s);
unsigned char uart_kbhit(void);
char uart_getchar(void);

#endif
