/* embeddedC/simpleC/avr/simple03/simple.c */

/* http://www.nongnu.org/avr-libc/user-manual/modules.html
   http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html */

/* tempo de interrupção do timer 0 em µs */
#define T0TICK 1000UL

/* defino onde está o led */
#define LED_PORT    PORTB
#define LED_DDR     DDRB
#define LED         PB1

/*---------------------------------------------------------------------------*
 *
 * Constante de recarga do timer 0, calculada para gerar interrupção de 1 ms
 *      timer prescaler = clock/8
 *      clock = 1 MHz
 *      T0RELOAD = clock/8 * t0tick
 *      precisa caber em 1 byte
 *
 */
#if (((F_CPU/10) * (T0TICK/10))/80000) > 256
#error "T0RELOAD não cabe em 1 byte"
#else
#define T0RELOAD (256 - (((F_CPU/10) * (T0TICK/10))/80000))
#endif

/* Sanity Check - válido apenas se F_CPU = 1000000 */
#if T0RELOAD != (256 - 125)
#error "Erro no cálculo de T0RELOAD"
#endif
/*
 *---------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
  /* configuro porta do led, sem afetar os outros bits */
  LED_PORT |= (1 << LED);
  LED_DDR  |= (1 << LED);

  /* timer0 free-running, gerando interrupção a cada 1 ms */
  /* prescaler = clock/8 */
  TCCR0 = (1 << CS01);
  TCNT0 = T0RELOAD;

  /* habilito interrupção de "overflow" do timer 0 */
  TIMSK = (1 << TOIE0);

  /* habilito interrupções */
  sei();

  while(1)
    {
      ;   /* tudo é feito pela interrupção */
    }

  return 0;
}


ISR(TIMER0_OVF_vect)
{
  static unsigned int prescaler = 0;

  TCNT0 = TCNT0 + T0RELOAD;

  if(++prescaler >= 500)
    {
      prescaler = 0;
      LED_PORT ^= (1 << LED);
    }
}
