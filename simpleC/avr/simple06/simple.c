/* embeddedC/simpleC/avr/simple06/simple.c */

/* O cálculo da constante de baudrate (UBRR) é feito pelo pré-processador,
   bastando definir F_CPU (no Makefile) e BAUDRATE e BAUD2X.
   Se o desvio de baudrate for maior que 2.5% será emitido um aviso na compilação.

   Opcionalmente o cálculo de UBRR pode ser feito por macros da avr-libc
   http://www.nongnu.org/avr-libc/user-manual/group__util__setbaud.html */

/* baudrate */
#define BAUDRATE 9600
#define BAUD2X 1

/* tempo de interrupção do timer 0 em µs */
#define T0TICK 1000UL

/* defino onde está o led */
#define LED_PORT    PORTB
#define LED_DDR     DDRB
#define LED         PB0

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
/*---------------------------------------------------------------------------*
 * Constante de comparação do timer 0, calculada para gerar interrupção de 1 ms
 *      timer prescaler = clock/8
 *      clock = 1 MHz
 *      T0COMPARE = clock/8 * t0tick
 *      precisa caber em 1 byte
 */
#if ((((F_CPU/10) * (T0TICK/10))/80000) - 1) > 256
#error "T0COMPARE não cabe em 1 byte"
#else
#define T0COMPARE ((((F_CPU/10) * (T0TICK/10))/80000) - 1)
#endif

/* Sanity Check - válido apenas se F_CPU = 1000000 */
#if T0COMPARE != 125 - 1
#error "Erro no cálculo de T0COMPARE"
#endif
/*---------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>

/* Protótipos */
void uart_putchar(char c);
void uart_puts(char* s);

volatile unsigned char PrintTime = 0;

void uart_putchar(char c)
{
  if(c == '\n')
    uart_putchar('\r');
  while(!(UCSRA & (1 << UDRE)));    // loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c;
}

void uart_puts(char* s)
{
  char c;
  while((c = *s++))
    {
      uart_putchar(c);
    }
}

int main(void)
{
  /* configuro porta do led, sem afetar os outros bits */
  LED_PORT |= (1 << LED);
  LED_DDR  |= (1 << LED);

  /* timer0 free-running, gerando interrupção a cada 1 ms */
  /* prescaler = clock/8 */
  /* clear timer on compare match (CTC) */
  TCCR0 = (1 << CS01) + (1 << WGM01);
  OCR0 = T0COMPARE;

  /* habilito interrupção de "compare match" do timer 0 */
  TIMSK = (1 << OCIE0);

  /* configuro UART */
  /* carrego constante de baudrate */
#if BAUD2X == 1
  UCSRA = (1 << U2X);
#endif
#if BAUDK >= 256
  UBRRH = (BAUDK >> 8) & 0xFF;
#endif
  UBRRL = BAUDK & 0xFF;
  /* habilito recepção e transmissão */
  UCSRB = (1 << RXEN) | (1 << TXEN);

  /* habilito interrupções */
  sei();

  while(1)
    {
      if(PrintTime)
        {
          PrintTime = 0;
          uart_puts("Hello, World! ");
        }
    }

  return 0;
}

ISR(TIMER0_COMP_vect)
{
  static unsigned int prescaler;

  if(++prescaler >= 1000)
    {
      prescaler = 0;
      LED_PORT ^= (1 << LED);

      /* Print it! */
      PrintTime = 1;
    }
}
