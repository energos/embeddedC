/* embeddedC/simpleC/avr/simple07/simple.c */

/* A recepção pela UART é feita por interrupção, com um "buffer" de 16 caracteres.
   A transmissão pela UART não é feita por interrupção, mas por um "busy loop".

   A biblioteca avr-libc oferece recursos bem mais sofisticados que os usados aqui.
   Veja a documentação em
   http://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html
   e um exemplo em
   http://www.nongnu.org/avr-libc/user-manual/group__stdiodemo.html. */

/* baudrate */
#define BAUDRATE 9600
#define BAUD2X 1

/* tempo de interrupção do timer 0 em µs */
#define T0TICK 1000UL

/* defino onde está o led */
#define LED_PORT    PORTB
#define LED_DDR     DDRB
#define LED         PB0

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
unsigned char uart_kbhit(void);
char uart_getchar(void);


char UART_RxBuffer[UART_RX_BUFFER_SIZE];
volatile unsigned char UART_RxHead = 0;
unsigned char UART_RxTail = 0;

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


int main(void)
{
  char c;

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
  /* habilito interrupção na recepção */
  UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

  /* habilito interrupções */
  sei();

  while(1)
    {
      c = uart_getchar();
      if(c == '!')
        uart_puts("Hello World!\n");
      else
        uart_putchar(c);
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
    }
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
