/* embeddedC/simpleC/avr/simple05/simple.c */

/* http://www.nongnu.org/avr-libc/user-manual/modules.html
   http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html */

/* tempo de interrupção do timer 1 em µs */
#define T1TICK 1000UL
/* tempo de interrupção do timer 0 em µs */
#define T0TICK 1000UL

/* defino onde estão os leds */
#define LED_PORT    PORTB
#define LED_DDR     DDRB
#define LED0ovf     PB2         /* controlado por timer 0 overflow interrupt */
#define LED0cmp     PB1         /* controlado por timer 0 compare interrupt  */
#define LED1        PB0         /* controlado pot timer 1 compare interrupt  */

/*---------------------------------------------------------------------------*
 * Constante de comparação do timer 1, calculada para gerar interrupção de 1 ms
 *      timer prescaler = clock/1
 *      clock = 1 MHz
 *      T1COMPARE = clock/1 * t1tick
 *      precisa caber em 1 word (16 bits)
 */
#if ((((F_CPU/10) * (T1TICK/10))/10000) - 1) > 65536
#error "T1COMPARE não cabe em 1 word"
#else
#define T1COMPARE ((((F_CPU/10) * (T1TICK/10))/10000) - 1)
#endif

/* Sanity Check - válido apenas se F_CPU = 1000000 */
#if T1COMPARE != 1000 - 1
#error "Erro no cálculo de T1COMPARE"
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
/*---------------------------------------------------------------------------*
 * Constante de recarga do timer 0, calculada para gerar interrupção de 1 ms
 *      timer prescaler = clock/8
 *      clock = 1 MHz
 *      T0RELOAD = clock/8 * t0tick
 *      precisa caber em 1 byte
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
/*---------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
  /* configuro porta do led, sem afetar os outros bits */
  LED_PORT |= (1 << LED1) | (1 << LED0cmp) | (1 << LED0ovf);
  LED_DDR  |= (1 << LED1) | (1 << LED0cmp) | (1 << LED0ovf);

  /* TIMER 1 */
  /* timer1 free-running, gerando interrupção a cada 1 ms */
  /* prescaler = clock/1 */
  /* clear timer on compare match (CTC) */
  /*
    TCCR1A:
    | COM1A1 | COM1A0 | COM1B1 | COM1B0 | FOC1A | FOC1B | WGM11 | WGM10 |
    | 0      | 0      | 0      | 0      | 0     | 0     | 0     | 0     |

    COM1A1:COM1A0/COM1B1:COM1B0 - Compare Output Mode for Channel A/B
    ~                             0:0 = Normal operation, OC1A/OC1B disconnected
    FOC1A/FOC1B                 - Force Output Compare for Channel A/B
    ~                             0 = Normal operation
    WGM13:WGM12:WGM11:WGM10     - Waweform Generation Mode
    ~                             0:1:0:0 = Mode 4, clear timer on compare match (CTC)

    TCCR1B:
    | ICNC1  | ICES1  | -      | WGM13  | WGM12 | CS12  | CS11  | CS10  |
    | 0      | 0      | 0      | 0      | 1     | 0     | 0     | 1     |

    ICNC1                       - Input Capture Noise Canceler
    ICES1                       - Input Capture Edge Select
    CS12:CS11:CS10              - Clock Select
    ~                             0:0:1 prescaler = clock/1
  */
  TCCR1A = 0;
  TCCR1B = (1 << CS10) + (1 << WGM12);
  OCR1A = T1COMPARE;

  /* TIMER 0 */
  /* timer0 free-running, gerando interrupção a cada 1 ms */
  /* prescaler = clock/8 */
  /* clear timer on compare match (CTC) */
  TCCR0 = (1 << CS01) + (1 << WGM01);
  OCR0 = T0COMPARE;

  /* habilito interrupções */
#if 0
  /* "compare match" do timer 1 canal A
     "compare match" do timer 0 */
  TIMSK = (1 << OCIE1A) | (1 << OCIE0);
#else
  /* "compare match" do timer 1 canal A
     "overflow"      do timer 0 */
  TCNT0 = T0RELOAD;
  TIMSK = (1 << OCIE1A) | (1 << TOIE0);
#endif

  /* habilito interrupções */
  sei();

  while(1)
    {
      ;   /* tudo é feito pela interrupção */
    }

  return 0;
}

ISR(TIMER1_COMPA_vect)
{
  static unsigned int prescaler = 0;

  if(++prescaler >= 500)
    {
      prescaler = 0;
      LED_PORT ^= (1 << LED1);
    }
}

ISR(TIMER0_COMP_vect)
{
  static unsigned int prescaler = 0;

  if(++prescaler >= 500)
    {
      prescaler = 0;
      LED_PORT ^= (1 << LED0cmp);
    }
}

ISR(TIMER0_OVF_vect)
{
  static unsigned int prescaler = 0;

  TCNT0 = TCNT0 + T0RELOAD;

  if(++prescaler >= 500)
    {
      prescaler = 0;
      LED_PORT ^= (1 << LED0ovf);
    }
}
