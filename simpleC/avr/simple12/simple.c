/* embeddedC/simpleC/avr/simple12/simple.c */

/* Leitura de arquivos no formato Intel Hexadecimal.

   TODO: Transmissão serial pela UART0 modificada para funcionar por interrupção.

   Nesse exemplo existem 7 comandos:
   - f1 - mostra a mensagem "Essa é a função 1"
   - hello - mostra a mensagem "Welcome to the Monitor"
   - dump address nbytes - mostra o conteúdo de nbytes da memória a partir de address
   - fill value address nbytes - preenche com value nbytes da memória a partir de address
   - xsum address nbytes - calcula o checksum de nbytes da memória a partir de address
   - ihex - recebe um arquivo no formato Intel Hexadecimal e escreve na memória RAM o conteúdo desse arquivo
   - ihexv - recebe um arquivo no formato Intel Hexadecimal e verifica a memória RAM com o conteúdo desse arquivo

   A linha de comando aceita até 4 "palavras", descartando o excedente.
   "Palavra" é qualquer conjunto de caracteres limitado por um ou mais espaços.
   Qualquer texto depois de um caracter '#', inclusive o próprio, é ignorado.
   Isso permite "comentários" na linha de comando.
*/

/* tempo de interrupção do timer 0 em µs */
#define T0TICK 1000UL

/* defino onde está o led */
#define LED_PORT    PORTB
#define LED_DDR     DDRB
#define LED         PB0
#define LED_UART_RECEIVE_OVERFLOW PB7

/*---------------------------------------------------------------------------*
 *
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
#include <avr/pgmspace.h>

#include "uart.h"
#include "cli.h"

/*---------------------------------------------------------------------------*
 */

int main(void)
{
  /* configuro porta do led, sem afetar os outros bits */
  LED_PORT |= (1 << LED) | (1 << LED_UART_RECEIVE_OVERFLOW);
  LED_DDR  |= (1 << LED) | (1 << LED_UART_RECEIVE_OVERFLOW);

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

  putcrlf();
  hello();
  putprompt();

  while(1)
    {
      monitor();
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

      /* uart receive overflow led */
      if(uart_receive_overflow_timeout > 0)
        {
          uart_receive_overflow_timeout--;
          LED_PORT &= ~(1 << LED_UART_RECEIVE_OVERFLOW);
        }
      else
        LED_PORT |= (1 << LED_UART_RECEIVE_OVERFLOW);

    }
}
